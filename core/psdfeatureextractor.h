#ifndef PSDFEATUREEXTRACTOR_H
#define PSDFEATUREEXTRACTOR_H
#include <QVector>

struct PsdFeatureResult
{
    bool ok = false;
    int psdLen = 0;
    double delta = 0.0;
    double theta = 0.0;
    double alpha = 0.0;
    double beta = 0.0;
    double gamma = 0.0;
    double alphaBetaRatio = 0.0;
};

// 必须放在类外：避免嵌套 struct + 默认成员初始化器触发编译器限制
struct PsdFeatureExtractorOptions
{
    int minSamples = 32;
    int minNfft = 32;
    int maxNfft = 512;
    int window = 1; // WindowOperations::HANNING
    double eps = 1e-12;

    double fDeltaLo = 1.0,  fDeltaHi = 4.0;
    double fThetaLo = 4.0,  fThetaHi = 8.0;
    double fAlphaLo = 8.0,  fAlphaHi = 13.0;
    double fBetaLo  = 13.0, fBetaHi  = 30.0;
    double fGammaLo = 30.0, fGammaHi = 45.0;
};

class PsdFeatureExtractor
{
public:
    using Options = PsdFeatureExtractorOptions;

    PsdFeatureExtractor();
    explicit PsdFeatureExtractor(const Options &opt);

    void setOptions(const Options &opt);
    const Options &options() const;

    PsdFeatureResult compute(const QVector<double> &y, int fs) const;

private:
    Options m_opt;
};
#endif // PSDFEATUREEXTRACTOR_H
