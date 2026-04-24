#ifndef FFTFEATUREEXTRACTOR_H
#define FFTFEATUREEXTRACTOR_H

#include <QVector>

struct FftFeatureResult
{
    bool ok = false;
    int nfft = 0;
    int fftLen = 0;
    double df = 0.0;


    double delta = 0.0;
    double theta = 0.0;
    double alpha = 0.0;
    double beta  = 0.0;
    double gamma = 0.0;

    double peakFreqHz = 0.0;
    double peakMag = 0.0;
};

struct FftFeatureExtractorOptions
{
    int minSamples = 32;
    int minNfft = 32;
    int maxNfft = 512;
    int window = 1; // WindowOperations::HANNING

    // 五频段边界（Hz）
    double fDeltaLo = 1.0,  fDeltaHi = 4.0;
    double fThetaLo = 4.0,  fThetaHi = 8.0;
    double fAlphaLo = 8.0,  fAlphaHi = 13.0;
    double fBetaLo  = 13.0, fBetaHi  = 30.0;
    double fGammaLo = 30.0, fGammaHi = 45.0;
};

class FftFeatureExtractor
{
public:
    using Options = FftFeatureExtractorOptions;

    FftFeatureExtractor();
    explicit FftFeatureExtractor(const Options &opt);

    void setOptions(const Options &opt);
    const Options &options() const;

    // 输入：窗口波形 y（uV）+ 采样率 fs
    FftFeatureResult compute(const QVector<double> &y, int fs) const;

private:
    Options m_opt;
};
#endif // FFTFEATUREEXTRACTOR_H
