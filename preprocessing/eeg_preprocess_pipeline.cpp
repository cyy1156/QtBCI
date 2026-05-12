#include "eeg_preprocess_pipeline.h"

#include <QtMath>
#include <limits>

EegPreprocessPipeline::EegPreprocessPipeline()
    : EegPreprocessPipeline(Options())
{
}

EegPreprocessPipeline::EegPreprocessPipeline(const Options &opt)
    : m_opt(opt),
      m_fs_smooth(opt.fs_nominal),
      m_has_last(false)
{
}

void EegPreprocessPipeline::setOptions(const Options &opt)
{
    m_opt = opt;
}

const EegPreprocessPipeline::Options &EegPreprocessPipeline::options() const
{
    return m_opt;
}

double EegPreprocessPipeline::fsEstimate() const
{
    return m_fs_smooth;
}

void EegPreprocessPipeline::reset()
{
    m_fs_smooth = m_opt.fs_nominal;
    m_has_last = false;
    m_last = FsHint{};
    m_last_fs_inst = std::numeric_limits<double>::quiet_NaN();
}

double EegPreprocessPipeline::clamp(double v, double lo, double hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

void EegPreprocessPipeline::calcPtpAndAbsMax(const QVector<double> &x, double &ptp, double &absmax)
{
    if (x.isEmpty())
    {
        ptp = 0.0;
        absmax = 0.0;
        return;
    }

    double mn = x[0];
    double mx = x[0];
    absmax = qAbs(x[0]);
    for (double v : x)
    {
        mn = qMin(mn, v);
        mx = qMax(mx, v);
        absmax = qMax(absmax, qAbs(v));
    }
    ptp = mx - mn;
}

void EegPreprocessPipeline::updateFsEstimate(const FsHint &hint, QString &dbg)
{
    m_last_fs_inst = std::numeric_limits<double>::quiet_NaN();

    if (hint.seq < 0 || hint.ts_ms <= 0)
        return;

    if (!m_has_last)
    {
        m_last = hint;
        m_has_last = true;
        return;
    }

    const qint64 dseq = hint.seq - m_last.seq;
    const qint64 dtms = hint.ts_ms - m_last.ts_ms;
    m_last = hint;

    if (dseq <= 0 || dtms <= 0)
        return;

    const double fs_inst = (double)dseq * 1000.0 / (double)dtms;
    m_last_fs_inst = fs_inst;
    const double fs_limited = clamp(fs_inst, m_opt.fs_min, m_opt.fs_max);
    m_fs_smooth = m_opt.fs_alpha * m_fs_smooth + (1.0 - m_opt.fs_alpha) * fs_limited;
    dbg += QStringLiteral("fs_inst=%1 fs_use=%2; ")
               .arg(fs_inst, 0, 'f', 2)
               .arg(m_fs_smooth, 0, 'f', 2);
}

EegPreprocessPipeline::Result EegPreprocessPipeline::process(const QVector<double> &x_uv,
                                                             const FsHint &hint)
{
    Result out;
    out.y = x_uv;

    QString dbg;
    if (m_opt.use_fixed_nominal_fs)
    {
        m_fs_smooth = m_opt.fs_nominal;
        out.fs_inst_hz = std::numeric_limits<double>::quiet_NaN();
    }
    else
    {
        updateFsEstimate(hint, dbg);
        out.fs_inst_hz = m_last_fs_inst;
    }

    const int fs = (int)qRound(m_fs_smooth);
    out.fs_used = (double)fs;

    if (out.y.isEmpty())
    {
        out.debug = dbg + QStringLiteral("empty epoch; ");
        return out;
    }

    // 1) 陷波（工频）
    if (m_opt.enable_notch)
    {
        std::vector<double> tmp;
        tmp.reserve(out.y.size());
        for (double v : out.y) tmp.push_back(v);

        DataFilter::remove_environmental_noise(tmp.data(), (int)tmp.size(), fs, m_opt.env_noise_type);
        for (int i = 0; i < out.y.size(); ++i)
            out.y[i] = tmp[(size_t)i];
        dbg += QStringLiteral("notch(ok); ");
    }

    // 2) 带通
    if (m_opt.enable_bandpass)
    {
        std::vector<double> tmp;
        tmp.reserve(out.y.size());
        for (double v : out.y) tmp.push_back(v);

        DataFilter::perform_bandpass(tmp.data(), (int)tmp.size(), fs,
                                     m_opt.bp_low_hz, m_opt.bp_high_hz,
                                     m_opt.bp_order, m_opt.bp_filter_type, m_opt.bp_ripple);
        for (int i = 0; i < out.y.size(); ++i)
            out.y[i] = tmp[(size_t)i];
        dbg += QStringLiteral("bandpass(%1-%2Hz ok); ").arg(m_opt.bp_low_hz).arg(m_opt.bp_high_hz);
    }

    // 2.5) 单通道可选：小波去噪（默认关闭）
    if (m_opt.enable_wavelet_denoise)
    {
        std::vector<double> tmp;
        tmp.reserve(out.y.size());
        for (double v : out.y) tmp.push_back(v);

        DataFilter::perform_wavelet_denoising(tmp.data(), (int)tmp.size(),
                                              m_opt.wavelet,
                                              m_opt.wavelet_level,
                                              m_opt.wavelet_denoise,
                                              m_opt.wavelet_threshold,
                                              m_opt.wavelet_extension,
                                              m_opt.wavelet_noise_level);
        for (int i = 0; i < out.y.size(); ++i)
            out.y[i] = tmp[(size_t)i];
        dbg += QStringLiteral("wavelet_denoise(ok); ");
    }

    // 3) 降采样（会改变 fs_used）
    if (m_opt.enable_downsample && m_opt.downsample_factor > 1)
    {
        int filtered_size = 0;
        std::vector<double> tmp;
        tmp.reserve(out.y.size());
        for (double v : out.y) tmp.push_back(v);

        filtered_size = 0;
        double *filtered = DataFilter::perform_downsampling(tmp.data(), (int)tmp.size(),
                                                            m_opt.downsample_factor,
                                                            m_opt.downsample_agg,
                                                            &filtered_size);
        QVector<double> ds;
        ds.resize(filtered_size);
        for (int i = 0; i < filtered_size; ++i)
            ds[i] = filtered[i];
        delete[] filtered;

        out.y = ds;
        out.fs_used = out.fs_used / (double)m_opt.downsample_factor;
        dbg += QStringLiteral("downsample(x%1 ok); ").arg(m_opt.downsample_factor);
    }

    // 4) 单通道伪影标记（不删除数据）
    if (m_opt.enable_artifact_mark)
    {
        calcPtpAndAbsMax(out.y, out.ptp_uv, out.absmax_uv);

        std::vector<double> tmp;
        tmp.reserve(out.y.size());
        for (double v : out.y) tmp.push_back(v);
        out.railed_ratio = DataFilter::get_railed_percentage(
            tmp.data(), (int)tmp.size(), m_opt.artifact_railed_gain);

        bool mark = false;
        if (out.ptp_uv > m_opt.artifact_ptp_uv) mark = true;
        if (out.absmax_uv > m_opt.artifact_absmax_uv) mark = true;
        if (out.railed_ratio > m_opt.artifact_railed_ratio) mark = true;
        out.artifact_marked = mark;

        dbg += QStringLiteral("artifact(ptp=%1 abs=%2 rail=%3 mark=%4); ")
                   .arg(out.ptp_uv, 0, 'f', 1)
                   .arg(out.absmax_uv, 0, 'f', 1)
                   .arg(out.railed_ratio, 0, 'f', 2)
                   .arg(out.artifact_marked ? 1 : 0);
    }

    out.debug = dbg;
    return out;
}

