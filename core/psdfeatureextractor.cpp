
#include "data_filter.h"
#include "brainflow_constants.h"
#include <algorithm>
#include <vector>
#include "psdfeatureextractor.h"

PsdFeatureExtractor::PsdFeatureExtractor():PsdFeatureExtractor(Options())
{

}
PsdFeatureExtractor::PsdFeatureExtractor(const Options &opt)
    :m_opt(opt)
{

}
void PsdFeatureExtractor::setOptions(const Options &opt)
{
    m_opt=opt;

}
const PsdFeatureExtractor::Options &PsdFeatureExtractor::options() const
{
    return m_opt;
}
PsdFeatureResult PsdFeatureExtractor::compute(const QVector<double> &y,int fs)const
{
    PsdFeatureResult out;
    if(fs<=0 ||y.size()<m_opt.minSamples)
        return out;
    std::vector<double> sig(y.begin(),y.end());

    int nfft=DataFilter::get_nearest_power_of_two((int)sig.size());//自动选一个接近窗口长度的 2 次幂 FFT 点数。
    if(nfft>(int)sig.size())
        nfft/=2;
    nfft=std::min(nfft,m_opt.maxNfft);
    if(nfft <m_opt.minNfft) return out;

    const int overlap =nfft/2;
    int psdLen =0;
    auto psd = DataFilter::get_psd_welch(
        sig.data(),
        (int)sig.size(),
        nfft,
        overlap,
        fs,
        m_opt.window,
        &psdLen);
    /*计算 Welch PSD，返回两组数组：
psd.first：功率谱幅值 ampl
psd.second：频率轴 freq
psdLen：数组长度（nfft/2+1）*/
    out.psdLen = psdLen;
    out.delta = DataFilter::get_band_power(psd, psdLen, m_opt.fDeltaLo, m_opt.fDeltaHi);
    out.theta = DataFilter::get_band_power(psd, psdLen, m_opt.fThetaLo, m_opt.fThetaHi);
    out.alpha = DataFilter::get_band_power(psd, psdLen, m_opt.fAlphaLo, m_opt.fAlphaHi);
    out.beta  = DataFilter::get_band_power(psd, psdLen, m_opt.fBetaLo,  m_opt.fBetaHi);
    out.gamma = DataFilter::get_band_power(psd, psdLen, m_opt.fGammaLo, m_opt.fGammaHi);
    out.alphaBetaRatio = (out.beta > m_opt.eps) ? (out.alpha / out.beta) : 0.0;
    out.ok = true;

    delete[] psd.first;   // ampl
    delete[] psd.second;  // freq
    return out;
}
