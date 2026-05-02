#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QTimer>
#include <QPlainTextEdit>
#include <QPointF>
#include "QCustomPlot/qcustomplot.h"
#include<thinkgear/thinkgearlinktester.h>
#include<QFile>
#include<QTextStream>
#include<core/acquisitionengine.h>
#include<core/algorithmengine.h>
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void attachLinkTester(ThinkGearLinkTester *tester);

private slots:
    void on_pushButton_start_clicked();
    void on_pushButton_stop_clicked();
    void on_pushButton_clear_clicked();
    void on_pushButton_save_clicked();
    /** 快捷打开当前 CSV（或所在文件夹） */
    void on_pushButton_openLogCsv_clicked();
    void onSecondReport(quint64 secIndex, int rawPerSec, int framePerSec, int warnPerSec, bool pass);
    void onTestMessage(const QString &msg);
    void onPlotRefreshTick();
    void on_pushButton_picture_clicked();

private:
    void setupTesterConnections();
    /** 默认目录下的 CSV 路径（每次「开始」可生成新文件名） */
    static QString makeDefaultCsvPath();
    void updateLogCsvPathUi();

    Ui::MainWindow *ui;
    ThinkGearLinkTester *m_tester = nullptr;
    /** 最近一次为「写 CSV」生成的绝对路径；不保存时为空 */
    QString m_currentCsvPath;

    bool m_csvLoggingEnable =false;
    bool m_uiTxtLoggingEnabled = true;
    bool m_psdLoggingEnable = false;
    bool m_fftLoggingEnable = false;
    // EEG 文件路径（传给 ThinkGearLinkTester/CsvLogWorker）

    QString m_eegCsvPath;
    // 操作日志 txt 路径（MainWindow 自己写）
    QString m_uiTxtPath;
    QString m_psdCsvPath;
    QString m_fftCsvPath;

    static QString makeDefaultEegCsvPath();
    static QString makeDefaultPsdCsvPath();
    static QString makeDefaultUiTxtPath();
    static QString makeDefaultFftCsvPath();

    void updateSavePathUi();
    void appendUiActionLog(const QString &category, const QString &message);
    void appendLogLine(const QString &line);
    void loadSerialSettings();
    void saveSerialSettings();
    bool showSerialConfigDialog();
    bool showChartModeDialog();
    void ensureGraphCount(int count);
    void clearPlotDisplay();
    void setupPlotInteractions();
    void renderRawChart();
    void renderFftChart();
    void renderBandPowerChart();
public:
    void restartSessionWithReset(const QString &port, qint32 baud);

    void initThreads();

    QThread *m_acqThread = nullptr;
    QThread *m_algThread = nullptr;
    QThread *m_logThread = nullptr;

    AcquisitionEngine *m_acq = nullptr;
    AlgorithmEngine *m_alg = nullptr;
    CsvLogWorker *m_csvWorker = nullptr;

    LogBuffer m_logBuffer{8192};
    enum class ChartMode {
        RawTime = 0,
        FftSpectrum = 1,
        BandPower = 2
    };
    struct FftBandPoint {
        double delta = 0.0;
        double theta = 0.0;
        double alpha = 0.0;
        double beta = 0.0;
        double gamma = 0.0;
        quint64 seqEnd = 0;
    };
    struct PsdBandPoint {
        double delta = 0.0;
        double theta = 0.0;
        double alpha = 0.0;
        double beta = 0.0;
        double gamma = 0.0;
        quint64 seqEnd = 0;
    };

    ChartMode m_chartMode = ChartMode::RawTime;
    QVector<double> m_plotCache;
    QVector<FftBandPoint> m_fftCache;
    QVector<PsdBandPoint> m_psdCache;
    int m_plotCacheMax = 20000;
    int m_rawFixedDisplayCount = 600;
    int m_featureCacheMax = 512;
    QTimer m_plotUiTimer;
    bool m_plotDirty = false;
    bool m_chartAutoFollow = true;
    bool m_updatingPlotRange = false;
    QCustomPlot *m_customPlot = nullptr;
    QCPItemTracer *m_clickMarker = nullptr;
    QPlainTextEdit *m_logText = nullptr;
    QPushButton *m_btnClearLog = nullptr;
    QPushButton *m_btnPauseAutoScroll = nullptr;
    bool m_autoScrollEnabled = true;
    int m_pausedNewLogCount = 0;
    bool m_acqRunning = false;
    SerialPortConfig m_serialCfg;
};
#endif // MAINWINDOW_H
