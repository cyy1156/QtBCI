#ifndef ALGORITHMENGINE_H
#define ALGORITHMENGINE_H
#include <core/psdfeatureextractor.h>
#include <QObject>
#include "core/pictureandalgbuffer.h"
#include "core/acquisitionengine.h"
#include "preprocessing/eeg_preprocess_pipeline.h"
#include "core/fftfeatureextractor.h"
struct RawPacket;
struct PlotChunk { QVector<double> y; quint64 seqStart=0; quint64 seqEnd=0; };
struct AlgoResult { QString tsMs=0; quint64 seqEnd=0; double score=0.0; QString label; };
struct FftResult
{
    QString tsMs;
    quint64 seqStart = 0;
    quint64 seqEnd = 0;
    double fsUsed = 0.0;
    int    nfft=0;

    double delta = 0.0;
    double theta = 0.0;
    double alpha = 0.0;
    double beta = 0.0;
    double gamma = 0.0;


    double rms = 0.0;//数值越大，说明这一窗口波形整体振幅越大（可能是更强的脑电活动，也可能是伪影/肌电/体动导致幅值变大）
    QString label; // clean / artifact
};
struct SpectrumResult
{
    QString tsMs;
    quint64 seqStart = 0;
    quint64 seqEnd = 0;
    double fsUsed = 0.0;

    double delta = 0.0;
    double theta = 0.0;
    double alpha = 0.0;
    double beta = 0.0;
    double gamma = 0.0;
    double alphaBetaRatio = 0.0;

    double rms = 0.0;//数值越大，说明这一窗口波形整体振幅越大（可能是更强的脑电活动，也可能是伪影/肌电/体动导致幅值变大）
    QString label; // clean / artifact
};
Q_DECLARE_METATYPE(SpectrumResult)
Q_DECLARE_METATYPE(PlotChunk)
Q_DECLARE_METATYPE(AlgoResult)
Q_DECLARE_METATYPE(FftResult);

class AlgorithmEngine : public QObject {
    Q_OBJECT
public:
    explicit AlgorithmEngine(QObject *parent=nullptr);

public slots:
    void onRawPacket(const RawPacket &pkt);   // 接收线程A样本
    void setWindowSize(int n) { m_windowSize = n; }
    void resetState();
signals:
    void plotChunkReady(const PlotChunk &chunk); // 发给UI线程绘图
    void algoResultReady(const AlgoResult &res); // 发给UI和日志
    void spectrumReady(const SpectrumResult & res);
    void fftResultReady(const FftResult& res);
private:
    PictureAndALgBuffer m_buf{128*80};
    int m_windowSize = 128;
    EegPreprocessPipeline m_pp;
    PsdFeatureExtractor m_psd;
    FftFeatureExtractor m_fft;
};

#endif // ALGORITHMENGINE_H
