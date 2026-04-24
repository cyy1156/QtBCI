# 算法线程接入 FFT 频谱（记录与 UI 配置）（代码示例）

本文完全参照你当前 `PSD` 的接入方式（独立计算类 + 算法线程发信号 + 日志线程写 CSV + 保存对话框开关/新建/打开），新增一个 **FFT 频谱功能**。

目标：

- 对预处理后的单通道窗口 `y` 做 **FFT**（不做 Welch / 不做 PSD）。
- 输出一个“频谱结果”（推荐：只记录关键频点/峰值；可选：记录全频点）。
- 新增 `fft_*.csv` 写盘文件，并在 UI 保存对话框里提供开关、新建、打开。

> 注意：FFT 输出是 **频点级**（每个窗口 `N/2+1` 个点），如果你每个窗口全写，会非常大。本文提供两种策略：  
> - **策略 A（推荐）**：窗口级一行（峰值频率 + 峰值幅度 + 若干目标频点）  
> - **策略 B（可选）**：频点级多行（每行一个频点），并提供降频写盘节流

---

## 1) 你已有的底层 FFT API（无需新库）

你工程已包含：

- `DataFilter::perform_fft(double *data, int data_len, int window, int *fft_len)`  
  返回 `std::complex<double>*`，长度 `fft_len = N/2 + 1`，需要 `delete[]`。
- `DataFilter::get_nearest_power_of_two(int value)`（选 N）
- `WindowOperations::HANNING`（窗函数）

---

## 2) 新增 FFT 结果结构（示例：五个频段“幅值”）

建议新增 `FftResult`（与 `SpectrumResult` 平行），窗口级输出：

文件：`core/algorithmengine.h`

```cpp
struct FftResult
{
    QString tsMs;
    quint64 seqStart = 0;
    quint64 seqEnd = 0;
    double fsUsed = 0.0;
    int nfft = 0;

    // 五个频段（FFT 频谱“幅值”特征，推荐写盘）
    // 定义：在对应频带内，取 |X[k]| 的代表值（本文默认：取最大幅值 peak）。
    // 注意：这不是 PSD/功率，也不做积分/总和；它只是“频谱幅值”。
    double delta = 0.0;
    double theta = 0.0;
    double alpha = 0.0;
    double beta  = 0.0;
    double gamma = 0.0;

    // 可选附加：峰值频率/峰值幅度（用于调试）
    double peakFreqHz = 0.0;
    double peakMag = 0.0;

    double rms = 0.0;
    QString label; // clean / artifact
};
Q_DECLARE_METATYPE(FftResult)
```

并在 `AlgorithmEngine` 新增信号：

```cpp
signals:
    void fftReady(const FftResult &res);
```

---

## 3) 把 FFT 计算独立成一个类（与 PsdFeatureExtractor 同风格）

建议新建：

- `preprocessing/fft_feature_extractor.h`（或 `core/fftfeatureextractor.h`，按你的 CMake 习惯）
- `preprocessing/fft_feature_extractor.cpp`

### 3.1 头文件示例：`fft_feature_extractor.h`（输出五频段）

```cpp
#pragma once

#include <QVector>

struct FftFeatureResult
{
    bool ok = false;
    int nfft = 0;
    int fftLen = 0;
    double df = 0.0;

    // 五个频段（按 |X| 取代表幅值；默认：频带内最大幅值）
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
```

### 3.2 源文件示例：`fft_feature_extractor.cpp`

```cpp
#include "fft_feature_extractor.h"
#include "data_filter.h"
#include "brainflow_constants.h"

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

// 频带幅值：在 [f0,f1] 内取 |X[k]| 的最大值（更符合你想要的“幅值大小”）
static double bandPeakMag(const std::complex<double> *X, int fftLen, double df, double f0, double f1)
{
    if (!X || fftLen <= 1 || df <= 0.0)
        return 0.0;
    const int k0 = nearestBin(std::min(f0, f1), df, fftLen - 1);
    const int k1 = nearestBin(std::max(f0, f1), df, fftLen - 1);
    double peak = 0.0;
    for (int k = k0; k <= k1; ++k)
        peak = std::max(peak, std::abs(X[k])); // |X[k]|
    return peak;
}

// 如果你不想“取最大”，而是“频段里随便取一个幅值”，建议明确规则，避免在循环里反复覆盖：
// - 规则 A：取频段中心频率对应的 bin（更稳定）
// - 规则 B：取第一个落入频段的 bin（更接近你最初的伪代码，但不推荐）
// 下面给出“中心 bin”的参考实现：
static double bandCenterMag(const std::complex<double> *X, int fftLen, double df, double f0, double f1)
{
    if (!X || fftLen <= 1 || df <= 0.0)
        return 0.0;
    const double fc = 0.5 * (std::min(f0, f1) + std::max(f0, f1));
    const int k = nearestBin(fc, df, fftLen - 1);
    return std::abs(X[k]);
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

    std::vector<double> sig;
    sig.reserve(N);
    for (int i = 0; i < N; ++i) sig.push_back(y[i]);

    int fftLen = 0;
    std::complex<double> *X = DataFilter::perform_fft(sig.data(), N, m_opt.window, &fftLen);

    const double df = (double)fs / (double)N;
    out.nfft = N;
    out.fftLen = fftLen;
    out.df = df;

    // 1) 五频段幅值（频带内取最大 |X|）
    out.delta = bandPeakMag(X, fftLen, df, m_opt.fDeltaLo, m_opt.fDeltaHi);
    out.theta = bandPeakMag(X, fftLen, df, m_opt.fThetaLo, m_opt.fThetaHi);
    out.alpha = bandPeakMag(X, fftLen, df, m_opt.fAlphaLo, m_opt.fAlphaHi);
    out.beta  = bandPeakMag(X, fftLen, df, m_opt.fBetaLo,  m_opt.fBetaHi);
    out.gamma = bandPeakMag(X, fftLen, df, m_opt.fGammaLo, m_opt.fGammaHi);

    // 2) 可选：峰值频率/峰值幅度（用于调试）
    int peakK = 1;
    double peakMag = 0.0;
    for (int k = 1; k < fftLen; ++k)
    {
        const double mag = std::abs(X[k]);
        if (mag > peakMag)
        {
            peakMag = mag;
            peakK = k;
        }
    }
    out.peakMag = peakMag;
    out.peakFreqHz = peakK * df;

    delete[] X;
    out.ok = true;
    return out;
}
```

---

## 4) 算法线程调用 FFT 类并发信号（示例）

文件：`core/algorithmengine.h` 增加成员：

```cpp
FftFeatureExtractor m_fft;
```

文件：`core/algorithmengine.cpp` 在你已得到 `y/rms/label/fsUsed` 之后：

```cpp
FftResult fr;
fr.tsMs = r.tsMs;
fr.seqStart = pc.seqStart;
fr.seqEnd = pc.seqEnd;
fr.fsUsed = res.fs_used;
fr.rms = rms;
fr.label = r.label;

const int fs = std::max(1, (int)std::lround(res.fs_used));
const auto f = m_fft.compute(y, fs);
if (f.ok)
{
    fr.nfft = f.nfft;
    fr.peakFreqHz = f.peakFreqHz;
    fr.peakMag = f.peakMag;
    fr.delta = f.delta;
    fr.theta = f.theta;
    fr.alpha = f.alpha;
    fr.beta  = f.beta;
    fr.gamma = f.gamma;
}

emit fftReady(fr);
```

---

## 5) 日志线程写 FFT CSV（窗口级一行，推荐）

建议复用你现在 `CsvLogWorker` 的“第二文件句柄”模式，新增一套 FFT 文件句柄：

### 5.1 `core/csvlogworker.h` 增加接口（示例）

```cpp
struct FftResult;

void setFftOutputPath(const QString &csvPath) { m_fftCsvPath = csvPath; }

public slots:
    void onFftResult(const FftResult &fr);

private:
    void writeFftHeaderIfNeeded();
    void writeFftRow(const FftResult &fr);

private:
    QString m_fftCsvPath;
    QFile m_fftFile;
    QTextStream m_fftOut;
    bool m_fftHeaderWritten = false;
```

### 5.2 `core/csvlogworker.cpp` 写盘实现（示例）

```cpp
void CsvLogWorker::writeFftHeaderIfNeeded()
{
    if (!m_fftFile.isOpen() || m_fftHeaderWritten)
        return;
    if (m_fftFile.size() == 0)
    {
        m_fftOut << "tsMs,seqStart,seqEnd,fsUsed,nfft,rms,delta,theta,alpha,beta,gamma,peakFreqHz,peakMag,label\n";
        m_fftOut.flush();
    }
    m_fftHeaderWritten = true;
}

void CsvLogWorker::writeFftRow(const FftResult &fr)
{
    m_fftOut
        << fr.tsMs << ','
        << fr.seqStart << ','
        << fr.seqEnd << ','
        << QString::number(fr.fsUsed, 'f', 3) << ','
        << fr.nfft << ','
        << QString::number(fr.rms, 'f', 6) << ','
        << QString::number(fr.delta, 'f', 6) << ','
        << QString::number(fr.theta, 'f', 6) << ','
        << QString::number(fr.alpha, 'f', 6) << ','
        << QString::number(fr.beta, 'f', 6) << ','
        << QString::number(fr.gamma, 'f', 6) << ','
        << QString::number(fr.peakFreqHz, 'f', 3) << ','
        << QString::number(fr.peakMag, 'f', 6) << ','
        << fr.label
        << '\n';
}

void CsvLogWorker::onFftResult(const FftResult &fr)
{
    if (QThread::currentThread() != thread())
        return;

    // 关闭开关时主动关闭
    if (m_fftCsvPath.isEmpty())
    {
        if (m_fftFile.isOpen())
        {
            m_fftOut.flush();
            m_fftFile.flush();
            m_fftFile.close();
        }
        m_fftHeaderWritten = false;
        return;
    }

    if (!m_fftFile.isOpen())
    {
        m_fftFile.setFileName(m_fftCsvPath);
        if (!m_fftFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        {
            emit workerError(QStringLiteral("FFT CSV open failed: %1").arg(m_fftFile.errorString()));
            return;
        }
        m_fftOut.setDevice(&m_fftFile);
        m_fftOut.setEncoding(QStringConverter::Utf8);
        m_fftHeaderWritten = false;
    }

    writeFftHeaderIfNeeded();
    writeFftRow(fr);
    m_fftOut.flush();
    m_fftFile.flush();
}
```

---

## 6) MainWindow：连接 + 保存对话框扩展（FFT 开关/新建/打开）

### 6.1 `initThreads()` 增加连接

```cpp
qRegisterMetaType<FftResult>("FftResult");

connect(m_alg, &AlgorithmEngine::fftReady,
        m_csvWorker, &CsvLogWorker::onFftResult,
        Qt::QueuedConnection);
```

### 6.2 `mainwindow.h` 增加成员

```cpp
bool m_fftLoggingEnable = false;
QString m_fftCsvPath;

static QString makeDefaultFftCsvPath();
```

### 6.3 默认路径函数（示例）

```cpp
QString MainWindow::makeDefaultFftCsvPath()
{
    const QString root = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    const QString dirPath = root + QLatin1Char('/') + QStringLiteral("QtBCI_logs");
    QDir().mkpath(dirPath);
    const QString name = QStringLiteral("fft_%1.csv")
                             .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss")));
    return QFileInfo(dirPath, name).absoluteFilePath();
}
```

### 6.4 保存对话框新增 FFT 项（示例片段）

```cpp
auto *cbEnableFft = new QCheckBox(QStringLiteral("开始采集时启用 FFT 频谱 CSV 保存"), &dlg);
cbEnableFft->setChecked(m_fftLoggingEnable);

auto *labFft = new QLabel(QStringLiteral("FFT CSV：\n%1")
                              .arg(m_fftCsvPath.isEmpty() ? QStringLiteral("未设置") : m_fftCsvPath), &dlg);
labFft->setWordWrap(true);

auto *btnNewFft = new QPushButton(QStringLiteral("新建 FFT CSV"), &dlg);
auto *btnOpenFft = new QPushButton(QStringLiteral("打开 FFT CSV"), &dlg);

connect(btnNewFft, &QPushButton::clicked, &dlg, [&]() {
    m_fftCsvPath = makeDefaultFftCsvPath();
    labFft->setText(QStringLiteral("FFT CSV：\n%1").arg(m_fftCsvPath));
});

connect(btnOpenFft, &QPushButton::clicked, &dlg, [&]() {
    if (m_fftCsvPath.isEmpty() || !QFileInfo::exists(m_fftCsvPath)) {
        QMessageBox::information(&dlg, QStringLiteral("提示"), QStringLiteral("FFT CSV 文件不存在。"));
        return;
    }
#if defined(Q_OS_WIN)
    QProcess::startDetached(QStringLiteral("explorer.exe"),
                            {QStringLiteral("/select,"), QDir::toNativeSeparators(m_fftCsvPath)});
#else
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_fftCsvPath));
#endif
});

// OK 后
m_fftLoggingEnable = cbEnableFft->isChecked();
if (m_fftLoggingEnable && m_fftCsvPath.isEmpty())
    m_fftCsvPath = makeDefaultFftCsvPath();
```

### 6.5 开始按钮下发 FFT 路径（示例片段）

```cpp
QMetaObject::invokeMethod(m_csvWorker, [this]() {
    if (m_fftLoggingEnable) {
        if (m_fftCsvPath.isEmpty())
            m_fftCsvPath = makeDefaultFftCsvPath();
        m_csvWorker->setFftOutputPath(m_fftCsvPath);
    } else {
        m_csvWorker->setFftOutputPath(QString());
    }
}, Qt::QueuedConnection);
```

---

## 7) 策略 B：如果你一定要“全频点写盘”（不推荐）

频点级写盘会非常大，建议至少加两个节流：

1. **只每秒写一次**（在算法线程里用计数器/时间戳判断）  
2. **每行写一个频点**：`tsMs, seqStart, nfft, fsUsed, k, freqHz, mag`

如果你确定需要，我可以再给出“频点级 CSV”的完整 worker 示例（含节流）。

