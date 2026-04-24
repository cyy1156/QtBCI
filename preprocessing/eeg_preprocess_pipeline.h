#pragma once

#include <QtGlobal>
#include <QVector>
#include <QString>

#include <vector>

// 你的工程里已有（来自 QtBCI/preprocessing/data_filter.h）
#include "data_filter.h"

#include "brainflow_constants.h"

class EegPreprocessPipeline
{
public:
    struct Options
    {
        // -------- 采样率估计 --------
        double fs_nominal = 250.0;
        double fs_min = 200.0;
        double fs_max = 300.0;
        double fs_alpha = 0.90;   // 越大越平滑

        // -------- 滤波参数（单通道/多通道通用）--------
        bool enable_notch = true;
        int env_noise_type = (int)NoiseTypes::FIFTY; // 50Hz

        bool enable_bandpass = true;
        double bp_low_hz = 0.5;
        double bp_high_hz = 40.0;
        int bp_order = 4;
        int bp_filter_type = (int)FilterTypes::BUTTERWORTH_ZERO_PHASE;
        double bp_ripple = 0.0;

        // -------- 降采样（可选）--------
        bool enable_downsample = false;
        int downsample_factor = 2;
        int downsample_agg = 0; // AggOperations: mean

        // -------- 单通道伪影处理（替代 ICA）--------
        // 仅标记坏段，不在此处删除数据
        bool enable_artifact_mark = true;
        double artifact_ptp_uv = 300.0;      // 峰峰值阈值
        double artifact_absmax_uv = 180.0;   // 绝对振幅阈值
        double artifact_railed_ratio = 0.30; // 饱和比例阈值
        int artifact_railed_gain = 1;

        // 小波去噪（可选，默认关闭）
        bool enable_wavelet_denoise = false;
        int wavelet = (int)WaveletTypes::DB4;
        int wavelet_level = 3;
        int wavelet_denoise = (int)WaveletDenoisingTypes::SURESHRINK;
        int wavelet_threshold = (int)ThresholdTypes::HARD;
        int wavelet_extension = (int)WaveletExtensionTypes::SYMMETRIC;
        int wavelet_noise_level = (int)NoiseEstimationLevelTypes::FIRST_LEVEL;
    };

    struct FsHint
    {
        qint64 seq = -1;
        qint64 ts_ms = -1; // 毫秒时间戳
    };

    struct Result
    {
        QVector<double> y;  // 单通道输出（µV）
        double fs_used = 0.0;//实际帧率
        bool artifact_marked = false;
        double ptp_uv = 0.0;
        double absmax_uv = 0.0;
        double railed_ratio = 0.0;
        QString debug;
    };

public:
    EegPreprocessPipeline();
    explicit EegPreprocessPipeline(const Options &opt);

    void setOptions(const Options &opt);
    const Options &options() const;

    // 单通道预处理：陷波 -> 带通 -> (可选) 降采样
    Result process(const QVector<double> &x_uv, const FsHint &hint);

    double fsEstimate() const;
    void reset();

private:
    void updateFsEstimate(const FsHint &hint, QString &dbg);
    static double clamp(double v, double lo, double hi);
    static void calcPtpAndAbsMax(const QVector<double> &x, double &ptp, double &absmax);

private:
    Options m_opt;
    double m_fs_smooth = 250.0;
    bool m_has_last = false;
    FsHint m_last;
};

