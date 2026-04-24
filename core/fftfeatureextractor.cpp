#include "fftfeatureextractor.h"

#include "data_filter.h"
#include <algorithm>
#include <complex>
#include <cmath>
#include <vector>

static int nearestBin(double f, double df, int maxBin)
{
    if (df <= 0.0) return 0;
    int k = (int)std::lround(f / df);
    if (k < 0) k = 0;
    if (k > maxBin) k = maxBin;
    return k;
}

// DataFilter::perform_fft 返回的频谱通常未归一化（未除以 N），直接取 |X[k]| 会随 N 线性放大。
// 这里把它换算为“单边幅值谱”（更接近输入信号幅值尺度）：
// - k=0(DC) 与 k=Nyquist(若存在) 使用 1/N
// - 其它频点使用 2/N
static double scaledMagAtBin(const std::complex<double> *X, int fftLen, int N, int k)
{
    if (!X || fftLen <= 0 || N <= 0 || k < 0 || k >= fftLen)
        return 0.0;
    const double mag = std::abs(X[k]);
    const bool isEdge = (k == 0) || (k == fftLen - 1);
    const double scale = isEdge ? (1.0 / (double)N) : (2.0 / (double)N);
    return mag * scale;
}

// 频带幅值：在 [f0,f1] 内取“幅值谱”的最大值（band 内 peak）
static double bandPeakMag(const std::complex<double> *X, int fftLen, int N, double df, double f0, double f1)
{
    if (!X || fftLen <= 1 || df <= 0.0)
        return 0.0;
    const int k0 = nearestBin(std::min(f0, f1), df, fftLen - 1);
    const int k1 = nearestBin(std::max(f0, f1), df, fftLen - 1);
    double peak = 0.0;
    for (int k = k0; k <= k1; ++k)
        peak = std::max(peak, scaledMagAtBin(X, fftLen, N, k));
    return peak;
}

//取中间值
static double bandCenterMag(const std::complex<double> *X, int fftLen, int N, double df, double f0, double f1)
{
    if (!X || fftLen <= 1 || df <= 0.0)
        return 0.0;
    const double fc = 0.5 * (std::min(f0, f1) + std::max(f0, f1));
    const int k = nearestBin(fc, df, fftLen - 1);
    return scaledMagAtBin(X, fftLen, N, k);
}

FftFeatureExtractor::FftFeatureExtractor()
    : FftFeatureExtractor(Options())
{
}

FftFeatureExtractor::FftFeatureExtractor(const Options &opt)
    : m_opt(opt)
{
}

void FftFeatureExtractor::setOptions(const Options &opt) { m_opt = opt; }
const FftFeatureExtractor::Options &FftFeatureExtractor::options() const { return m_opt; }

FftFeatureResult FftFeatureExtractor::compute(const QVector<double> &y, int fs) const
{
    FftFeatureResult out;
    if (fs <= 0 || y.size() < m_opt.minSamples)
        return out;

    // 选 N（偶数、2 的幂、<= y.size()）
    int N = DataFilter::get_nearest_power_of_two((int)y.size());
    if (N > y.size()) N /= 2;
    N = std::min(N, m_opt.maxNfft);
    if (N < m_opt.minNfft) return out;
    if (N % 2 == 1) --N;
    if (N < m_opt.minNfft) return out;

    // 去均值（减少 DC/漂移泄漏到低频段，避免 delta/theta 被“顶爆”）
    double mean = 0.0;
    for (int i = 0; i < N; ++i)
        mean += y[i];
    mean /= (double)N;

    std::vector<double> sig;
    sig.reserve(N);
    for (int i = 0; i < N; ++i)
        sig.push_back(y[i] - mean);

    int fftLen = 0;
    std::complex<double> *X = DataFilter::perform_fft(sig.data(), N, m_opt.window, &fftLen);
    if (!X || fftLen <= 1)
        return out;

    const double df = (double)fs / (double)N;
    out.nfft = N;
    out.fftLen = fftLen;
    out.df = df;

    // 1) 五频段幅值（频带内取最大 |X|）
    out.delta = bandPeakMag(X, fftLen, N, df, m_opt.fDeltaLo, m_opt.fDeltaHi);
    out.theta = bandPeakMag(X, fftLen, N, df, m_opt.fThetaLo, m_opt.fThetaHi);
    out.alpha = bandPeakMag(X, fftLen, N, df, m_opt.fAlphaLo, m_opt.fAlphaHi);
    out.beta  = bandPeakMag(X, fftLen, N, df, m_opt.fBetaLo,  m_opt.fBetaHi);
    out.gamma = bandPeakMag(X, fftLen, N, df, m_opt.fGammaLo, m_opt.fGammaHi);



    delete[] X;
    out.ok = true;
    return out;
}
