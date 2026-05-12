#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Net/networkconfigdialog.h"
#include "serialconfigdialog.h"
#include "saveconfigdialog.h"
#include "chartmodedialog.h"

// 旧单链路对象（ThinkGearLinkTester）相关 include，注释保留
// #include "thinkgear/thinkgearlinktester.h"
#include <core/algorithmengine.h>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QStringConverter>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QDialog>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <limits>
#include <algorithm>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QSettings>
#include <QComboBox>
#include <QFormLayout>
#include <QAction>
#include <QToolTip>
#include <QSharedPointer>

namespace {

double bandPointTimeKey(qint64 wallMs, const QString &tsMs)
{
    if (wallMs > 0)
        return static_cast<double>(wallMs) / 1000.0;
    if (!tsMs.isEmpty()) {
        const QDateTime dt = QDateTime::fromString(tsMs, QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"));
        if (dt.isValid())
            return static_cast<double>(dt.toMSecsSinceEpoch()) / 1000.0;
    }
    return 0.0;
}

void applySampleIndexXAxis(QCustomPlot *plot)
{
    if (!plot)
        return;
    plot->xAxis->setTicker(QSharedPointer<QCPAxisTicker>(new QCPAxisTicker));
    plot->xAxis->setLabel(QStringLiteral("Sample"));
}

void applyLocalDateTimeXAxis(QCustomPlot *plot)
{
    if (!plot)
        return;
    QSharedPointer<QCPAxisTickerDateTime> dtTicker(new QCPAxisTickerDateTime);
    dtTicker->setDateTimeFormat(QStringLiteral("HH:mm:ss.zzz\nyyyy-MM-dd"));
    dtTicker->setDateTimeSpec(Qt::LocalTime);
    plot->xAxis->setTicker(dtTicker);
    plot->xAxis->setLabel(QStringLiteral("Time (local)"));
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    loadSerialSettings();

    if (ui->menu) {
        auto *actSerial = new QAction(QStringLiteral("串口设置"), this);
        connect(actSerial, &QAction::triggered, this, [this]() {
            showSerialConfigDialog();
        });
        ui->menu->addAction(actSerial);
        auto *actNet = new QAction(QStringLiteral("网络设置"), this);
        connect(actNet, &QAction::triggered, this, [this]() {
            showNetworkConfigDialog();
        });
        ui->menu->addAction(actNet);
    }

    // 控制面板四个按钮放大到约 1.5 倍
    auto scaleButton = [](QPushButton *btn) {
        if (!btn) return;
        const QSize s = btn->sizeHint();
        btn->setMinimumSize(static_cast<int>(s.width() * 1.25),
                            static_cast<int>(s.height() * 1.25));

        QFont f = btn->font();
        if (f.pointSizeF() > 0) {
            f.setPointSizeF(f.pointSizeF() * 2.0);
        } else if (f.pixelSize() > 0) {
            f.setPixelSize(static_cast<int>(f.pixelSize() * 2.0));
        } else {
            f.setPointSize(20); // 兜底
        }
        btn->setFont(f);
    };
    scaleButton(ui->pushButton_start);
    scaleButton(ui->pushButton_stop);
    scaleButton(ui->pushButton_clear);
    scaleButton(ui->pushButton_save);
    scaleButton(ui->pushButton_serialConfig);
    if (ui->pushButton_network)
        scaleButton(ui->pushButton_network);
    if (auto *btn = findChild<QPushButton *>(QStringLiteral("pushButton_picture")))
        scaleButton(btn);
    if (auto *btn = findChild<QPushButton *>(QStringLiteral("pushButton_changeChart")))
        scaleButton(btn);
    if (ui->pushButton_serialConfig) {
        connect(ui->pushButton_serialConfig, &QPushButton::clicked, this, [this]() {
            showSerialConfigDialog();
        });
    }
    if (ui->pushButton_network) {
        connect(ui->pushButton_network, &QPushButton::clicked, this, [this]() {
            showNetworkConfigDialog();
        });
    }
    if (auto *btn = findChild<QPushButton *>(QStringLiteral("pushButton_picture"))) {
        connect(btn, &QPushButton::clicked, this, &MainWindow::on_pushButton_picture_clicked);
    }
    if (auto *btn = findChild<QPushButton *>(QStringLiteral("pushButton_changeChart"))) {
        connect(btn, &QPushButton::clicked, this, &MainWindow::on_pushButton_picture_clicked);
    }

    // 右侧区域上下比例：图像 70% / 日志 30%
    if (ui->verticalLayoutRight)
    {
        ui->verticalLayoutRight->setStretch(0, 7);
        ui->verticalLayoutRight->setStretch(1, 3);
    }

    // 用 QCustomPlot 替换原有 listWidget_picture，做更美观的实时曲线
    if (ui->listWidget_picture && ui->listWidget_picture->parentWidget())
    {
        QWidget *container = ui->listWidget_picture->parentWidget();
        auto *layout = qobject_cast<QBoxLayout *>(container->layout());
        if (layout)
        {
            m_customPlot = new QCustomPlot(container);
            m_customPlot->setObjectName(QStringLiteral("customPlot_eeg"));
            m_customPlot->setBackground(QBrush(QColor(20, 24, 31)));
            m_customPlot->axisRect()->setBackground(QColor(20, 24, 31));
            m_customPlot->legend->setVisible(false);
            m_customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop | Qt::AlignRight);

            QPen axisPen(QColor(180, 190, 205));
            m_customPlot->xAxis->setBasePen(axisPen);
            m_customPlot->yAxis->setBasePen(axisPen);
            m_customPlot->xAxis->setTickPen(axisPen);
            m_customPlot->yAxis->setTickPen(axisPen);
            m_customPlot->xAxis->setSubTickPen(axisPen);
            m_customPlot->yAxis->setSubTickPen(axisPen);
            m_customPlot->xAxis->setTickLabelColor(QColor(210, 220, 235));
            m_customPlot->yAxis->setTickLabelColor(QColor(210, 220, 235));
            m_customPlot->xAxis->grid()->setPen(QPen(QColor(60, 70, 85), 1, Qt::DotLine));
            m_customPlot->yAxis->grid()->setPen(QPen(QColor(60, 70, 85), 1, Qt::DotLine));

            m_customPlot->addGraph();
            QPen wavePen(QColor(0, 210, 170));
            wavePen.setWidth(1); // 线细一点
            m_customPlot->graph(0)->setPen(wavePen);
            m_customPlot->graph(0)->setAntialiasedFill(false);
            m_customPlot->graph(0)->setSelectable(QCP::stDataRange);
            m_customPlot->setNotAntialiasedElements(QCP::aeAll); // 降低 CPU 占用
            m_customPlot->xAxis->setLabel(QStringLiteral("Sample"));
            m_customPlot->yAxis->setLabel(QStringLiteral("uV"));
            m_customPlot->xAxis->setRange(0, m_plotCacheMax);
            m_customPlot->yAxis->setRange(-80, 80); // 固定量程避免每帧自适应卡顿
            setupPlotInteractions();

            layout->replaceWidget(ui->listWidget_picture, m_customPlot);
            layout->removeWidget(ui->listWidget_picture);
            ui->listWidget_picture->hide();
            ui->listWidget_picture->setParent(nullptr);
        }
    }

    // 右下日志区：替换成 RealCtrl 风格 QPlainTextEdit + 工具条
    if (ui->listWidget_text && ui->listWidget_text->parentWidget())
    {
        QWidget *container = ui->listWidget_text->parentWidget();
        auto *layout = qobject_cast<QBoxLayout *>(container->layout());
        if (layout)
        {
            auto *logWrap = new QWidget(container);
            logWrap->setObjectName(QStringLiteral("logWrap"));
            logWrap->setStyleSheet(QStringLiteral(
                "QWidget#logWrap {"
                "background-color: rgb(20, 23, 39);"
                "border: 1px solid rgb(68, 90, 130);"
                "}"
            ));
            auto *v = new QVBoxLayout(logWrap);
            v->setContentsMargins(6, 6, 6, 6);
            v->setSpacing(6);

            auto *tool = new QHBoxLayout();
            tool->setContentsMargins(0, 0, 0, 0);
            tool->setSpacing(6);
            m_btnClearLog = new QPushButton(QStringLiteral("清空日志"), logWrap);
            m_btnPauseAutoScroll = new QPushButton(QStringLiteral("暂停自动滚动"), logWrap);
            m_btnClearLog->setFixedHeight(26);
            m_btnPauseAutoScroll->setFixedHeight(26);
            const QString miniBtn = QStringLiteral(
                "QPushButton { color:#E5EEFF; background-color:rgba(255,255,255,0.10);"
                "border:1px solid rgba(107,168,255,0.45); border-radius:10px; padding:2px 10px; }"
                "QPushButton:hover { background-color:rgba(107,168,255,0.20); }"
                "QPushButton:pressed { background-color:rgba(107,168,255,0.30); }");
            m_btnClearLog->setStyleSheet(miniBtn);
            m_btnPauseAutoScroll->setStyleSheet(miniBtn);
            tool->addWidget(m_btnClearLog);
            tool->addWidget(m_btnPauseAutoScroll);
            tool->addStretch();

            m_logText = new QPlainTextEdit(logWrap);
            m_logText->setReadOnly(true);
            m_logText->setMaximumBlockCount(1500);
            m_logText->setStyleSheet(QStringLiteral(
                "QPlainTextEdit {"
                "background-color: rgb(20, 23, 39);"
                "color: rgb(186, 214, 255);"
                "border: 1px solid rgba(68, 90, 130, 0);"
                "font-family: Consolas, 'Courier New', monospace;"
                "font-size: 10pt;"
                "}"));
            v->addLayout(tool);
            v->addWidget(m_logText);

            layout->replaceWidget(ui->listWidget_text, logWrap);
            ui->listWidget_text->hide();

            connect(m_btnClearLog, &QPushButton::clicked, this, [this]() {
                if (!m_logText) return;
                m_logText->clear();
                m_pausedNewLogCount = 0;
                appendUiActionLog(QStringLiteral("UI"), QStringLiteral("已清空日志显示区"));
            });
            connect(m_btnPauseAutoScroll, &QPushButton::clicked, this, [this]() {
                m_autoScrollEnabled = !m_autoScrollEnabled;
                if (!m_btnPauseAutoScroll) return;
                if (m_autoScrollEnabled) {
                    m_pausedNewLogCount = 0;
                    m_btnPauseAutoScroll->setText(QStringLiteral("暂停自动滚动"));
                    if (m_logText && m_logText->verticalScrollBar())
                        m_logText->verticalScrollBar()->setValue(m_logText->verticalScrollBar()->maximum());
                    appendUiActionLog(QStringLiteral("UI"), QStringLiteral("恢复日志自动滚动"));
                } else {
                    m_btnPauseAutoScroll->setText(QStringLiteral("恢复自动滚动"));
                    appendUiActionLog(QStringLiteral("UI"), QStringLiteral("暂停日志自动滚动"));
                }
            });
        }
    }

    m_plotUiTimer.setInterval(30); // ~20 FPS，进一步减轻卡顿
    m_plotUiTimer.setTimerType(Qt::CoarseTimer);
    connect(&m_plotUiTimer, &QTimer::timeout, this, &MainWindow::onPlotRefreshTick);
    m_plotUiTimer.start();

    appendUiActionLog(QStringLiteral("INIT"),
                      QStringLiteral("主界面构造完成；绘图刷新定时器已启动(间隔30ms)"));
    appendUiActionLog(QStringLiteral("SERIAL"),
                      QStringLiteral("已从设置加载默认串口: %1 @%2")
                          .arg(m_serialCfg.portName.trimmed().isEmpty() ? QStringLiteral("(空)")
                                                                         : m_serialCfg.portName.trimmed())
                          .arg(m_serialCfg.baudRate));
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::initThreads()
{
    qRegisterMetaType<RawPacket>("RawPacket");
    qRegisterMetaType<PlotChunk>("PlotChunk");
    qRegisterMetaType<AlgoResult>("AlgoResult");
    /*告诉 Qt 元对象系统：这三个自定义类（RawPacket、PlotChunk、AlgoResult）可以在信号和槽中传递，尤其是跨线程时能正常拷贝、传输。*/
    m_acqThread = new QThread(this);
    m_algThread = new QThread(this);
    m_logThread = new QThread(this);

    m_acq = new AcquisitionEngine();
    m_alg = new AlgorithmEngine();
    m_csvWorker = new CsvLogWorker(&m_logBuffer);

    m_acq->moveToThread(m_acqThread);
    m_alg->moveToThread(m_algThread);
    m_csvWorker->moveToThread(m_logThread);

    // A -> B
    connect(m_acq, &AcquisitionEngine::rawPacketReady,
            m_alg, &AlgorithmEngine::onRawPacket, Qt::QueuedConnection);

    // B -> UI
    connect(m_alg, &AlgorithmEngine::plotChunkReady, this, [this](const PlotChunk &pc){
        // 1) 预处理后的绘图数据进入 UI 线程缓存
        m_plotCache += pc.y;
        // 按你的需求保留更完整历史，不在这里截断原始缓存。
        m_plotDirty = true;

        // 2) 这里是 UI 线程，后续可直接用 m_plotCache 更新 QCustomPlot
        // 目前使用定时器统一刷新，避免每个 chunk 都重绘导致 UI 卡顿

        // 3) 预处理后入 CSV 队列：按标称 215Hz 从 anchor 回推每点墙钟，与 raw 时间对齐
        static constexpr double kDeviceNominalFs = 215.0;
        for (int i = 0; i < pc.y.size(); ++i) {
            const quint64 seq = pc.seqStart + static_cast<quint64>(i);
            qint64 wallMs;
            if (pc.anchorWallMs >= 0) {
                if (pc.anchorSeq >= seq) {
                    const qint64 dseq = static_cast<qint64>(pc.anchorSeq - seq);
                    wallMs = pc.anchorWallMs
                             - static_cast<qint64>(qRound(dseq * (1000.0 / kDeviceNominalFs)));
                } else {
                    const qint64 dseq = static_cast<qint64>(seq - pc.anchorSeq);
                    wallMs = pc.anchorWallMs
                             + static_cast<qint64>(qRound(dseq * (1000.0 / kDeviceNominalFs)));
                }
            } else {
                wallMs = QDateTime::currentMSecsSinceEpoch();
            }
            const QString ts = QDateTime::fromMSecsSinceEpoch(wallMs, Qt::LocalTime)
                                   .toString(QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"));
            LogItem li;
            li.tsMs = ts;
            li.wallMs = wallMs;
            li.kind = QStringLiteral("preproc");
            li.seq = seq;
            li.gapSincePrev = 0;
            li.rawInt16 = 0;
            li.signalQuality = 255;
            li.rawUv = std::numeric_limits<double>::quiet_NaN();
            li.preprocUv = pc.y[i];
            m_logBuffer.push(li);
        }
    });
    connect(m_alg, &AlgorithmEngine::algoResultReady, this, [this](const AlgoResult &res){
        appendLogLine(QStringLiteral("[ALG] ts=%1 seq=%2 score=%3 label=%4")
                          .arg(res.tsMs)
                          .arg(res.seqEnd)
                          .arg(res.score, 0, 'f', 3)
                          .arg(res.label));
    });

    // A/B -> LogBuffer（示例：在各自回调里 push）
    connect(m_acq, &AcquisitionEngine::rawPacketReady, this, [this](const RawPacket &p){
        LogItem li;
        li.tsMs = p.tsMs;
        li.wallMs = p.wallMs;
        li.kind = QStringLiteral("raw");
        li.seq = p.seq;
        li.rawInt16 = p.rawInt16;
        li.signalQuality = p.signalQuality;
        li.rawUv = p.rawUv;
        li.gapSincePrev = p.gapSincePrev;
        li.preprocUv = std::numeric_limits<double>::quiet_NaN();
        m_logBuffer.push(li);
    });
    qRegisterMetaType<SpectrumResult>("SpectrumResult");
    qRegisterMetaType<FftResult>("FftResult");

    connect(m_alg, &AlgorithmEngine::spectrumReady,
            m_csvWorker, &CsvLogWorker::onSpectrumResult,
            Qt::QueuedConnection);
    connect(m_alg, &AlgorithmEngine::fftResultReady,
            m_csvWorker, &CsvLogWorker::onFftResult,
            Qt::QueuedConnection);
    connect(m_alg, &AlgorithmEngine::fftResultReady, this, [this](const FftResult &fr){
        FftBandPoint p;
        p.delta = fr.delta;
        p.theta = fr.theta;
        p.alpha = fr.alpha;
        p.beta = fr.beta;
        p.gamma = fr.gamma;
        p.seqEnd = fr.seqEnd;
        p.wallMs = fr.wallMs;
        p.tsMs = fr.tsMs;
        m_fftCache.push_back(p);
        if (m_fftCache.size() > m_featureCacheMax)
            m_fftCache.remove(0, m_fftCache.size() - m_featureCacheMax);
        if (m_chartMode == ChartMode::FftSpectrum)
            m_plotDirty = true;
    });
    connect(m_alg, &AlgorithmEngine::spectrumReady, this, [this](const SpectrumResult &sp){
        PsdBandPoint p;
        p.delta = sp.delta;
        p.theta = sp.theta;
        p.alpha = sp.alpha;
        p.beta = sp.beta;
        p.gamma = sp.gamma;
        p.seqEnd = sp.seqEnd;
        p.wallMs = sp.wallMs;
        p.tsMs = sp.tsMs;
        m_psdCache.push_back(p);
        if (m_psdCache.size() > m_featureCacheMax)
            m_psdCache.remove(0, m_psdCache.size() - m_featureCacheMax);
        if (m_chartMode == ChartMode::BandPower)
            m_plotDirty = true;
    });
    connect(m_acq, &AcquisitionEngine::statusMessage, this, [this](const QString &msg){
        appendLogLine(QStringLiteral("[ACQ][INFO] %1").arg(msg));
    });
    connect(m_acq, &AcquisitionEngine::warningMessage, this, [this](const QString &msg){
        appendLogLine(QStringLiteral("[ACQ][WARN] %1").arg(msg));
    });
    connect(m_acq, &AcquisitionEngine::runningChanged, this, [this](bool running){
        m_acqRunning = running;
    });
    connect(m_acq, &AcquisitionEngine::streamGapDetected, this,
            [this](quint64 missed, quint64 prevSeq, quint64 curSeq, qint64 /*eventWallMs*/) {
                const QString detail =
                    QStringLiteral("missed=%1 after_seq=%2 got_seq=%3")
                        .arg(missed)
                        .arg(prevSeq)
                        .arg(curSeq);
                appendUiActionLog(QStringLiteral("STREAM"),
                                  QStringLiteral("[WARN] %1").arg(detail));
            },
            Qt::QueuedConnection);
    connect(m_acq, &AcquisitionEngine::linkDiagnostic, this,
            [this](const QString &category, const QString &message, qint64 /*eventWallMs*/) {
                appendUiActionLog(category, QStringLiteral("[WARN] %1").arg(message));
            },
            Qt::QueuedConnection);
    connect(m_acq, &AcquisitionEngine::streamGapDetected, m_csvWorker,
            &CsvLogWorker::onAcquisitionStreamGap, Qt::QueuedConnection);
    connect(m_acq, &AcquisitionEngine::linkDiagnostic, m_csvWorker,
            &CsvLogWorker::onAcquisitionLinkDiag, Qt::QueuedConnection);
    connect(m_csvWorker, &CsvLogWorker::workerInfo, this, [this](const QString &msg){
        appendLogLine(QStringLiteral("[CSV][INFO] %1").arg(msg));
    });
    connect(m_csvWorker, &CsvLogWorker::workerError, this, [this](const QString &msg){
        appendLogLine(QStringLiteral("[CSV][ERROR] %1").arg(msg));
    });
    connect(m_csvWorker, &CsvLogWorker::drained, this,
            [this](int items, int queueSizeAfter, int droppedCount) {
                Q_UNUSED(items);
                Q_UNUSED(queueSizeAfter);
                if (droppedCount <= m_logDropWatermark)
                    return;
                appendUiActionLog(QStringLiteral("LOG"),
                                  QStringLiteral("[DROP] LogBuffer overflow: cumulative dropped=%1 (software queue)")
                                      .arg(droppedCount));
                m_logDropWatermark = droppedCount;
            },
            Qt::QueuedConnection);
    NetworkSettingsStore::load(&m_netSettings);
    m_udpHub = new UdpTelemetryHub(this);
    m_udpHub->applySettings(m_netSettings);
    connect(m_alg, &AlgorithmEngine::plotChunkReady, m_udpHub, &UdpTelemetryHub::sendPreprocPbc1,
            Qt::QueuedConnection);
    connect(m_alg, &AlgorithmEngine::fftResultReady, m_udpHub, &UdpTelemetryHub::sendFftPbf1,
            Qt::QueuedConnection);
    connect(m_alg, &AlgorithmEngine::spectrumReady, m_udpHub, &UdpTelemetryHub::sendPsdPbp1,
            Qt::QueuedConnection);
    // 启动线程后再启动worker
    connect(m_logThread, &QThread::started, m_csvWorker, &CsvLogWorker::start);

    m_acqThread->start();
    m_algThread->start();
    m_logThread->start();

    appendUiActionLog(QStringLiteral("INIT"),
                      QStringLiteral("三线程已启动: 采集/算法/CSV 工作线程 start 完成"));
    appendUiActionLog(QStringLiteral("NETWORK"),
                      QStringLiteral("已加载并应用网络配置: enabled=%1 host=%2 PBC1=%3 PBF1=%4 PBP1=%5 send pre=%6 fft=%7 psd=%8")
                          .arg(m_netSettings.enabled)
                          .arg(m_netSettings.host)
                          .arg(m_netSettings.portPreproc)
                          .arg(m_netSettings.portFft)
                          .arg(m_netSettings.portPsd)
                          .arg(m_netSettings.sendPreproc)
                          .arg(m_netSettings.sendFft)
                          .arg(m_netSettings.sendPsd));
}
void MainWindow::setupTesterConnections()
{
    // 旧单链路连接点，三线程主链下不启用，注释保留
    /*
    if (!m_tester)
        return;

    connect(m_tester, &ThinkGearLinkTester::secondReport,
            this, &MainWindow::onSecondReport);
    connect(m_tester, &ThinkGearLinkTester::testMessage,
            this, &MainWindow::onTestMessage);
    */
}
QString MainWindow::makeDefaultPsdCsvPath()
{
    const QString root=QStandardPaths::writableLocation(
        QStandardPaths::DocumentsLocation);
    const QString sub =QStringLiteral("QtBCI_logs");
    const QString dirPath =root+QLatin1Char('/')+sub;
    QDir().mkpath(dirPath);
    const QString name=QStringLiteral("psd_%1.csv").arg
                         (QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_hhmmss")));
    return QFileInfo(dirPath,name).absoluteFilePath();
}
/*QString MainWindow::makeDefaultCsvPath()
{
    const QString root=QStandardPaths::writableLocation(
        QStandardPaths::DocumentsLocation);
    const QString sub =QStringLiteral("QtBCI_logs");
    const QString dirPath =root+QLatin1Char('/')+sub;
    QDir().mkpath(dirPath);
    const QString name=QStringLiteral("raw_%1.csv").arg
                         (QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_hhmmss")));
    return QFileInfo(dirPath,name).absoluteFilePath();
}*/
QString MainWindow::makeDefaultEegCsvPath()
{
    const QString root = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    const QString dirPath = root + QLatin1Char('/') + QStringLiteral("QtBCI_logs");
    QDir().mkpath(dirPath);
    const QString name = QStringLiteral("eeg_%1.csv")
                             .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss")));
    return QFileInfo(dirPath, name).absoluteFilePath();
}

QString MainWindow::makeDefaultUiTxtPath()
{
    const QString root = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    const QString dirPath = root + QLatin1Char('/') + QStringLiteral("QtBCI_logs");
    QDir().mkpath(dirPath);
    const QString name = QStringLiteral("ui_%1.txt")
                             .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss")));
    return QFileInfo(dirPath, name).absoluteFilePath();
}

QString MainWindow::makeDefaultFftCsvPath()
{
    const QString root = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    const QString dirPath = root + QLatin1Char('/') + QStringLiteral("QtBCI_logs");
    QDir().mkpath(dirPath);
    const QString name = QStringLiteral("fft_%1.csv")
                             .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss")));
    return QFileInfo(dirPath, name).absoluteFilePath();
}
void MainWindow::updateSavePathUi()
{
    const QString eeg = m_eegCsvPath.isEmpty() ? QStringLiteral("未设置") : m_eegCsvPath;
    const QString txt = m_uiTxtPath.isEmpty() ? QStringLiteral("未设置") : m_uiTxtPath;
    const QString psd = m_psdCsvPath.isEmpty() ? QStringLiteral("未设置") : m_psdCsvPath;
    const QString fft = m_fftCsvPath.isEmpty() ? QStringLiteral("未设置") : m_fftCsvPath;
    ui->statusbar->showMessage(
        QStringLiteral("EEG保存:%1 | 操作TXT:%2 |PSD保存:%3 |FFT保存:%4 | EEG:%5 | UI:%6 | PSD:%7 | FFT:%8")
            .arg(m_csvLoggingEnable ? QStringLiteral("开") : QStringLiteral("关"))
            .arg(m_uiTxtLoggingEnabled ? QStringLiteral("开") : QStringLiteral("关"))
            .arg(m_psdLoggingEnable? QStringLiteral("开") : QStringLiteral("关"))
            .arg(m_fftLoggingEnable ? QStringLiteral("开") : QStringLiteral("关"))
            .arg(eeg)
            .arg(txt)
            .arg(psd)
            .arg(fft),
        5000);
}
// 操作类日志：始终写入界面日志区；在开启操作 TXT 且路径有效时追加写入磁盘。
void MainWindow::appendUiActionLog(const QString &category, const QString &message)
{
    appendLogLine(QStringLiteral("[%1] %2").arg(category, message));

    if (!m_uiTxtLoggingEnabled)
        return;
    if (m_uiTxtPath.isEmpty())
        return;

    QFile f(m_uiTxtPath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        return;

    QTextStream out(&f);
    out.setEncoding(QStringConverter::Utf8);
    const QString ts = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"));
    out << "[" << ts << "] "
        << "[" << category << "] "
        << message << '\n';
}
void MainWindow::attachLinkTester(ThinkGearLinkTester *tester)
{
    // 旧单链路入口，三线程主链下不启用，注释保留
    /*
    m_tester=tester;
    setupTesterConnections();
    */
    Q_UNUSED(tester);
}
/*void MainWindow::updateLogCsvPathUi()
{
    if(m_currentCsvPath.isEmpty())
        ui->statusbar->showMessage(QStringLiteral("CSV:本次未启用"),3000);
    else
        ui->statusbar->showMessage(QStringLiteral("CSV: %1").arg(m_currentCsvPath),5000);


}*/
void MainWindow::on_pushButton_start_clicked()
{
    // 三线程主链：线程A采集 + 线程C写盘
    if (!m_acq || !m_csvWorker) {
        appendUiActionLog(QStringLiteral("UI"), QStringLiteral("开始失败: 三线程未初始化，请先调用 initThreads()"));
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("三线程对象未初始化，请先调用 initThreads()"));
        return;
    }
    if (m_acqRunning) {
        appendUiActionLog(QStringLiteral("UI"), QStringLiteral("开始忽略: 采集已在运行，请先停止"));
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("采集已经在运行中，请先停止。"));
        return;
    }

    // 旧单链路逻辑，注释保留
    /*
    m_tester->setCsvLoggingEnabled(m_csvLoggingEnable);
    if (m_csvLoggingEnable) {
        if (m_eegCsvPath.isEmpty())
            m_eegCsvPath = makeDefaultEegCsvPath();
        m_tester->setCsvOutputPath(m_eegCsvPath);
    } else {
        m_tester->setCsvOutputPath(QString());
    }
    */

    if (m_alg) {
        QMetaObject::invokeMethod(m_alg, [this]() {
            m_alg->setRunning(true);
        }, Qt::QueuedConnection);
    }

    if (m_uiTxtPath.isEmpty())
        m_uiTxtPath = makeDefaultUiTxtPath();
    updateSavePathUi();

    if (m_serialCfg.portName.trimmed().isEmpty()) {
        if (!showSerialConfigDialog()) {
            appendUiActionLog(QStringLiteral("UI"), QStringLiteral("开始取消: 未完成串口配置或校验未通过"));
            return;
        }
    }

    // 线程C：按开关显式启动/停止 CSV worker
    if (m_csvLoggingEnable) {
        if (m_eegCsvPath.isEmpty())
            m_eegCsvPath = makeDefaultEegCsvPath();
        QMetaObject::invokeMethod(m_csvWorker, [this]() {
            m_csvWorker->stop();
            m_csvWorker->setOutputPath(m_eegCsvPath);
            m_csvWorker->start();
        }, Qt::QueuedConnection);
    } else {
        QMetaObject::invokeMethod(m_csvWorker, "stop", Qt::QueuedConnection);
    }
    /*if(m_psdLoggingEnable){
        if(m_psdCsvPath.isEmpty())
            m_psdCsvPath =makeDefaultPsdCsvPath();
        QMetaObject::invokeMethod(m_csvWorker,[this](){
            m_csvWorker->stop();
            m_csvWorker->setOutputPath(m_psdCsvPath);
            m_csvWorker->start();
        },Qt::QueuedConnection);
    }*/
    QMetaObject::invokeMethod(m_csvWorker, [this]() {
        if (m_psdLoggingEnable) {
            if (m_psdCsvPath.isEmpty())
                m_psdCsvPath = makeDefaultPsdCsvPath();
            m_csvWorker->setSpectrumOutputPath(m_psdCsvPath); // 只设置 PSD 路径
        } else {
            m_csvWorker->setSpectrumOutputPath(QString());     // 关闭 PSD 写盘
        }
    }, Qt::QueuedConnection);

    QMetaObject::invokeMethod(m_csvWorker, [this]() {
        if (m_fftLoggingEnable) {
            if (m_fftCsvPath.isEmpty())
                m_fftCsvPath = makeDefaultFftCsvPath();
            m_csvWorker->setFftOutputPath(m_fftCsvPath); // FFT CSV
        } else {
            m_csvWorker->setFftOutputPath(QString());     // 关闭 FFT 写盘
        }
    }, Qt::QueuedConnection);
    // 线程A：启动采集
    const SerialPortConfig cfg = m_serialCfg;
    QMetaObject::invokeMethod(m_acq, [this, cfg]() {
        m_acq->startWithConfig(cfg);
    }, Qt::QueuedConnection);

    appendUiActionLog(QStringLiteral("UI"),
                      QStringLiteral("开始采集: %1 @%2 | EEG_CSV=%3 UI_TXT=%4 PSD_CSV=%5 FFT_CSV=%6")
                          .arg(cfg.portName.trimmed())
                          .arg(cfg.baudRate)
                          .arg(m_csvLoggingEnable ? QStringLiteral("开") : QStringLiteral("关"))
                          .arg(m_uiTxtLoggingEnabled ? QStringLiteral("开") : QStringLiteral("关"))
                          .arg(m_psdLoggingEnable ? QStringLiteral("开") : QStringLiteral("关"))
                          .arg(m_fftLoggingEnable ? QStringLiteral("开") : QStringLiteral("关")));

    ui->statusbar->showMessage(QStringLiteral("已打开 %1 @%2")
                                   .arg(cfg.portName.trimmed())
                                   .arg(cfg.baudRate), 5000);
}
void MainWindow::on_pushButton_stop_clicked()
{
    const bool wasRunning = m_acqRunning;
    // 三线程主链停止
    if (!wasRunning) {
        ui->statusbar->showMessage(QStringLiteral("当前未在采集"), 2000);
    }
    if (m_alg) {
        // 先关算法门控，丢弃停止后的迟到队列包，避免下次 start 突然冒旧数据
        QMetaObject::invokeMethod(m_alg, [this]() {
            m_alg->setRunning(false);
            m_alg->resetState();
        }, Qt::QueuedConnection);
    }
    if (m_acq) {
        QMetaObject::invokeMethod(m_acq, "stop", Qt::QueuedConnection);
    }
    if (m_csvWorker) {
        QMetaObject::invokeMethod(m_csvWorker, "stop", Qt::QueuedConnection);
    }
    // 清日志队列，避免旧数据在下次 start 时再次写盘
    m_logBuffer.clear(true);
    m_logDropWatermark = 0;
    // 按你的需求：停止时保留当前图像，不清空 UI 缓存。
    // 如需清空，使用“清除”按钮。

    // 旧单链路停止，注释保留
    /*
    if(m_tester)
        m_tester->stop();
    */
    //状态栏提醒3s之后自动消失
    ui->statusbar->showMessage(QStringLiteral("已经停止"),3000);
    appendUiActionLog(QStringLiteral("UI"),
                      wasRunning ? QStringLiteral("点击停止")
                                 : QStringLiteral("点击停止（采集未在运行，仍已执行停止与队列清理）"));
}
void MainWindow::on_pushButton_save_clicked()
{
    SaveConfigDialog dlg(this);
    dlg.loadState(m_csvLoggingEnable,
                  m_uiTxtLoggingEnabled,
                  m_psdLoggingEnable,
                  m_fftLoggingEnable,
                  m_eegCsvPath,
                  m_uiTxtPath,
                  m_psdCsvPath,
                  m_fftCsvPath);

    connect(&dlg, &SaveConfigDialog::newEegClicked, this, [this, &dlg]() {
        m_eegCsvPath = makeDefaultEegCsvPath();
        dlg.refreshPathLabels(m_eegCsvPath, m_uiTxtPath, m_psdCsvPath, m_fftCsvPath);
        appendUiActionLog(QStringLiteral("UI"), QStringLiteral("新建 EEG_CSV 路径: %1").arg(m_eegCsvPath));
    });
    connect(&dlg, &SaveConfigDialog::newTxtClicked, this, [this, &dlg]() {
        m_uiTxtPath = makeDefaultUiTxtPath();
        dlg.refreshPathLabels(m_eegCsvPath, m_uiTxtPath, m_psdCsvPath, m_fftCsvPath);
        appendUiActionLog(QStringLiteral("UI"), QStringLiteral("新建 操作TXT 路径: %1").arg(m_uiTxtPath));
    });
    connect(&dlg, &SaveConfigDialog::newPsdClicked, this, [this, &dlg]() {
        m_psdCsvPath = makeDefaultPsdCsvPath();
        dlg.refreshPathLabels(m_eegCsvPath, m_uiTxtPath, m_psdCsvPath, m_fftCsvPath);
        appendUiActionLog(QStringLiteral("UI"), QStringLiteral("新建 PSD_CSV 路径: %1").arg(m_psdCsvPath));
    });
    connect(&dlg, &SaveConfigDialog::newFftClicked, this, [this, &dlg]() {
        m_fftCsvPath = makeDefaultFftCsvPath();
        dlg.refreshPathLabels(m_eegCsvPath, m_uiTxtPath, m_psdCsvPath, m_fftCsvPath);
        appendUiActionLog(QStringLiteral("UI"), QStringLiteral("新建 FFT_CSV 路径: %1").arg(m_fftCsvPath));
    });

    connect(&dlg, &SaveConfigDialog::openEegClicked, this, [this, &dlg]() {
        if (m_eegCsvPath.isEmpty() || !QFileInfo::exists(m_eegCsvPath)) {
            QMessageBox::information(&dlg, QStringLiteral("提示"), QStringLiteral("EEG CSV 文件不存在。"));
            return;
        }
#if defined(Q_OS_WIN)
        QProcess::startDetached(QStringLiteral("explorer.exe"),
                                {QStringLiteral("/select,"), QDir::toNativeSeparators(m_eegCsvPath)});
#else
        QDesktopServices::openUrl(QUrl::fromLocalFile(m_eegCsvPath));
#endif
        appendUiActionLog(QStringLiteral("UI"), QStringLiteral("已打开 EEG_CSV 所在位置: %1").arg(m_eegCsvPath));
    });
    connect(&dlg, &SaveConfigDialog::openTxtClicked, this, [this, &dlg]() {
        if (m_uiTxtPath.isEmpty() || !QFileInfo::exists(m_uiTxtPath)) {
            QMessageBox::information(&dlg, QStringLiteral("提示"), QStringLiteral("操作 TXT 文件不存在。"));
            return;
        }
#if defined(Q_OS_WIN)
        QProcess::startDetached(QStringLiteral("explorer.exe"),
                                {QStringLiteral("/select,"), QDir::toNativeSeparators(m_uiTxtPath)});
#else
        QDesktopServices::openUrl(QUrl::fromLocalFile(m_uiTxtPath));
#endif
        appendUiActionLog(QStringLiteral("UI"), QStringLiteral("已打开 操作TXT 所在位置: %1").arg(m_uiTxtPath));
    });
    connect(&dlg, &SaveConfigDialog::openPsdClicked, this, [this, &dlg]() {
        if (m_psdCsvPath.isEmpty() || !QFileInfo::exists(m_psdCsvPath)) {
            QMessageBox::information(&dlg, QStringLiteral("提示"), QStringLiteral("PSD CSV 文件不存在。"));
            return;
        }
#if defined(Q_OS_WIN)
        QProcess::startDetached(QStringLiteral("explorer.exe"),
                                {QStringLiteral("/select,"), QDir::toNativeSeparators(m_psdCsvPath)});
#else
        QDesktopServices::openUrl(QUrl::fromLocalFile(m_psdCsvPath));
#endif
        appendUiActionLog(QStringLiteral("UI"), QStringLiteral("已打开 PSD_CSV 所在位置: %1").arg(m_psdCsvPath));
    });
    connect(&dlg, &SaveConfigDialog::openFftClicked, this, [this, &dlg]() {
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
        appendUiActionLog(QStringLiteral("UI"), QStringLiteral("已打开 FFT_CSV 所在位置: %1").arg(m_fftCsvPath));
    });
    connect(&dlg, &SaveConfigDialog::serialConfigClicked, this, [this]() {
        appendUiActionLog(QStringLiteral("UI"), QStringLiteral("从保存配置对话框打开串口设置"));
        showSerialConfigDialog();
    });

    if (dlg.exec() != QDialog::Accepted) {
        appendUiActionLog(QStringLiteral("UI"), QStringLiteral("保存配置已取消"));
        return;
    }

    m_csvLoggingEnable = dlg.csvLoggingEnabled();
    m_uiTxtLoggingEnabled = dlg.uiTxtLoggingEnabled();
    m_psdLoggingEnable = dlg.psdLoggingEnabled();
    m_fftLoggingEnable = dlg.fftLoggingEnabled();
    if (m_csvLoggingEnable && m_eegCsvPath.isEmpty())
        m_eegCsvPath = makeDefaultEegCsvPath();
    if (m_uiTxtPath.isEmpty())
        m_uiTxtPath = makeDefaultUiTxtPath();
    if(m_psdLoggingEnable&&m_psdCsvPath.isEmpty())
        m_psdCsvPath=makeDefaultPsdCsvPath();
    if (m_fftLoggingEnable && m_fftCsvPath.isEmpty())
        m_fftCsvPath = makeDefaultFftCsvPath();

    updateSavePathUi();
    appendUiActionLog(QStringLiteral("UI"),
                      QStringLiteral("保存配置: EEG_CSV=%1, UI_TXT=%2, PSD_CSV=%3, FFT_CSV=%4 | eeg=%5 | uiTxt=%6 | psd=%7 | fft=%8")
                          .arg(m_csvLoggingEnable ? QStringLiteral("开") : QStringLiteral("关"))
                          .arg(m_uiTxtLoggingEnabled ? QStringLiteral("开") : QStringLiteral("关"))
                          .arg(m_psdLoggingEnable ? QStringLiteral("开") : QStringLiteral("关"))
                          .arg(m_fftLoggingEnable ? QStringLiteral("开") : QStringLiteral("关"))
                          .arg(m_eegCsvPath)
                          .arg(m_uiTxtPath)
                          .arg(m_psdCsvPath)
                          .arg(m_fftCsvPath));

}

void MainWindow::loadSerialSettings()
{
    QSettings settings(QStringLiteral("QtBCI"), QStringLiteral("QtBCI"));
    m_serialCfg.portName = settings.value(QStringLiteral("serial/portName"), QStringLiteral("COM7")).toString();
    m_serialCfg.baudRate = settings.value(QStringLiteral("serial/baudRate"), 57600).toInt();
    m_serialCfg.dataBits = static_cast<QSerialPort::DataBits>(
        settings.value(QStringLiteral("serial/dataBits"), static_cast<int>(QSerialPort::Data8)).toInt());
    m_serialCfg.parity = static_cast<QSerialPort::Parity>(
        settings.value(QStringLiteral("serial/parity"), static_cast<int>(QSerialPort::NoParity)).toInt());
    m_serialCfg.stopBits = static_cast<QSerialPort::StopBits>(
        settings.value(QStringLiteral("serial/stopBits"), static_cast<int>(QSerialPort::OneStop)).toInt());
    m_serialCfg.flowControl = static_cast<QSerialPort::FlowControl>(
        settings.value(QStringLiteral("serial/flowControl"), static_cast<int>(QSerialPort::NoFlowControl)).toInt());
}

void MainWindow::saveSerialSettings()
{
    QSettings settings(QStringLiteral("QtBCI"), QStringLiteral("QtBCI"));
    settings.setValue(QStringLiteral("serial/portName"), m_serialCfg.portName);
    settings.setValue(QStringLiteral("serial/baudRate"), m_serialCfg.baudRate);
    settings.setValue(QStringLiteral("serial/dataBits"), static_cast<int>(m_serialCfg.dataBits));
    settings.setValue(QStringLiteral("serial/parity"), static_cast<int>(m_serialCfg.parity));
    settings.setValue(QStringLiteral("serial/stopBits"), static_cast<int>(m_serialCfg.stopBits));
    settings.setValue(QStringLiteral("serial/flowControl"), static_cast<int>(m_serialCfg.flowControl));
}

void MainWindow::applyNetworkStreamToHub()
{
    if (!m_udpHub)
        return;
    m_udpHub->applySettings(m_netSettings);
}

void MainWindow::showNetworkConfigDialog()
{
    appendUiActionLog(QStringLiteral("NETWORK"), QStringLiteral("打开网络设置对话框"));
    NetworkConfigDialog dlg(this);
    dlg.setFromSettings(m_netSettings);
    if (dlg.exec() != QDialog::Accepted) {
        appendUiActionLog(QStringLiteral("NETWORK"), QStringLiteral("取消网络设置，未保存"));
        return;
    }
    m_netSettings = dlg.toSettings();
    NetworkSettingsStore::save(m_netSettings);
    applyNetworkStreamToHub();
    appendUiActionLog(QStringLiteral("NETWORK"),
                      QStringLiteral("网络: enabled=%1 host=%2 ports PBC1=%3 PBF1=%4 PBP1=%5 send pre=%6 fft=%7 psd=%8")
                          .arg(m_netSettings.enabled)
                          .arg(m_netSettings.host)
                          .arg(m_netSettings.portPreproc)
                          .arg(m_netSettings.portFft)
                          .arg(m_netSettings.portPsd)
                          .arg(m_netSettings.sendPreproc)
                          .arg(m_netSettings.sendFft)
                          .arg(m_netSettings.sendPsd));
}

bool MainWindow::showSerialConfigDialog()
{
    appendUiActionLog(QStringLiteral("SERIAL"), QStringLiteral("打开串口设置对话框"));
    SerialConfigDialog dlg(this);
    dlg.setFromConfig(m_serialCfg);
    if (dlg.exec() != QDialog::Accepted) {
        appendUiActionLog(QStringLiteral("SERIAL"), QStringLiteral("取消串口设置，未保存"));
        return false;
    }

    m_serialCfg = dlg.toConfig();

    if (m_serialCfg.portName.isEmpty() || m_serialCfg.baudRate <= 0) {
        appendUiActionLog(QStringLiteral("SERIAL"), QStringLiteral("串口参数无效，未保存（端口为空或波特率无效）"));
        QMessageBox::warning(this, QStringLiteral("参数无效"), QStringLiteral("请设置有效的端口和波特率。"));
        return false;
    }

    saveSerialSettings();
    appendUiActionLog(QStringLiteral("SERIAL"),
                      QStringLiteral("串口配置: %1 @%2, data=%3, parity=%4, stop=%5, flow=%6")
                          .arg(m_serialCfg.portName)
                          .arg(m_serialCfg.baudRate)
                          .arg(static_cast<int>(m_serialCfg.dataBits))
                          .arg(static_cast<int>(m_serialCfg.parity))
                          .arg(static_cast<int>(m_serialCfg.stopBits))
                          .arg(static_cast<int>(m_serialCfg.flowControl)));
    return true;
}
void MainWindow::onSecondReport(quint64 secIndex, int rawPerSec, int framePerSec, int warnPerSec, bool pass)
{
    const QString line = QStringLiteral("[%1] raw/s=%2 frame/s=%3 warn/s=%4 pass=%5")
    .arg(secIndex)
        .arg(rawPerSec)
        .arg(framePerSec)
        .arg(warnPerSec)
        .arg(pass ? QStringLiteral("OK") : QStringLiteral("NO"));
    ui->statusbar->showMessage(line, 1000);//状态栏临时显示 1 秒。
    appendUiActionLog(QStringLiteral("STAT"), line);
}

void MainWindow::onTestMessage(const QString &msg)
{
    appendUiActionLog(QStringLiteral("SYS"), msg);
}

 void MainWindow::on_pushButton_clear_clicked()
{
    const auto ret = QMessageBox::question(
        this,
        QStringLiteral("确认"),
        QStringLiteral("清除并重置会话？这会停止并重新开始采集，seq 从 0 重新计数。"));
    if (ret != QMessageBox::Yes) {
        appendUiActionLog(QStringLiteral("UI"), QStringLiteral("清除会话: 已取消"));
        return;
    }

    // 这里你可保存上次端口，或弹窗重新选择
    const QString port = QStringLiteral("COM7");
    restartSessionWithReset(port, 57600);
    m_fftCache.clear();
    m_psdCache.clear();

    appendUiActionLog(QStringLiteral("UI"), QStringLiteral("点击清除并重置会话"));
}
void MainWindow::on_pushButton_openLogCsv_clicked()
{
    if (m_eegCsvPath.isEmpty()) {
        appendUiActionLog(QStringLiteral("UI"),
                          QStringLiteral("打开 EEG_CSV: 路径未设置，请在「保存」中设置或开始采集后生成默认路径"));
        if (ui && ui->statusbar)
            ui->statusbar->showMessage(QStringLiteral("EEG CSV 路径未设置"), 3000);
        return;
    }
    if (!QFileInfo::exists(m_eegCsvPath)) {
        appendUiActionLog(QStringLiteral("UI"),
                          QStringLiteral("打开 EEG_CSV: 文件不存在 %1").arg(m_eegCsvPath));
        if (ui && ui->statusbar)
            ui->statusbar->showMessage(QStringLiteral("文件不存在"), 3000);
        return;
    }
#if defined(Q_OS_WIN)
    QProcess::startDetached(QStringLiteral("explorer.exe"),
                            {QStringLiteral("/select,"), QDir::toNativeSeparators(m_eegCsvPath)});
#else
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_eegCsvPath));
#endif
    appendUiActionLog(QStringLiteral("UI"), QStringLiteral("已打开 EEG_CSV 所在位置: %1").arg(m_eegCsvPath));
}

void MainWindow::onPlotRefreshTick()
{
    if (!m_plotDirty)
        return;
    m_plotDirty = false;

    if (!ui)
        return;

    // 优先用 QCustomPlot 在 UI 线程刷新曲线
    if (m_customPlot && m_customPlot->graphCount() > 0)
    {
        switch (m_chartMode) {
        case ChartMode::RawTime:
            renderRawChart();
            break;
        case ChartMode::FftSpectrum:
            renderFftChart();
            break;
        case ChartMode::BandPower:
            renderBandPowerChart();
            break;
        }
        return;
    }

    // 兜底：若 QCustomPlot 未初始化，继续用旧 listWidget 预览
    if (ui->listWidget_picture)
    {
        ui->listWidget_picture->clear();
        const int n = m_plotCache.size();
        const int step = qMax(1, n / 128);
        for (int i = 0; i < n; i += step)
            ui->listWidget_picture->addItem(QString::number(m_plotCache[i], 'f', 2));
        ui->listWidget_picture->scrollToBottom();
    }
}

void MainWindow::appendLogLine(const QString &line)
{
    if (m_logText)
    {
        auto *bar = m_logText->verticalScrollBar();
        const int oldPos = bar ? bar->value() : 0;
        m_logText->appendPlainText(line);
        if (bar)
        {
            if (m_autoScrollEnabled)
            {
                bar->setValue(bar->maximum());
            }
            else
            {
                bar->setValue(oldPos);
                ++m_pausedNewLogCount;
                if (m_btnPauseAutoScroll)
                {
                    m_btnPauseAutoScroll->setText(
                        QStringLiteral("恢复自动滚动 (%1)").arg(m_pausedNewLogCount));
                }
            }
        }
        return;
    }

    // 兜底：新日志面板未初始化时继续使用旧控件
    if (ui->listWidget_text)
    {
        ui->listWidget_text->addItem(line);
        ui->listWidget_text->scrollToBottom();
    }
}

void MainWindow::restartSessionWithReset(const QString &port, qint32 baud)
{
    Q_UNUSED(port);
    Q_UNUSED(baud);
    appendUiActionLog(QStringLiteral("SESSION"),
                      QStringLiteral("清除会话: 停止采集与写盘、重置算法状态、清空曲线缓存"));
    if (!m_acq || !m_alg || !m_csvWorker)
        return;

    // 1) 停止采集与写盘
    QMetaObject::invokeMethod(m_acq, "stop", Qt::QueuedConnection);
    QMetaObject::invokeMethod(m_csvWorker, "stop", Qt::QueuedConnection);

    // 2) 清算法状态（必须在算法线程执行）
    QMetaObject::invokeMethod(m_alg, "resetState", Qt::QueuedConnection);

    // 3) 清 UI 缓存（主线程）
    m_plotCache.clear();
    m_fftCache.clear();
    m_psdCache.clear();
    clearPlotDisplay();

    // 4) 重新配置写盘路径（主CSV + PSD CSV）
    if (m_csvLoggingEnable && m_eegCsvPath.isEmpty())
        m_eegCsvPath = makeDefaultEegCsvPath();
    if (m_psdLoggingEnable && m_psdCsvPath.isEmpty())
        m_psdCsvPath = makeDefaultPsdCsvPath();
    if (m_fftLoggingEnable && m_fftCsvPath.isEmpty())
        m_fftCsvPath = makeDefaultFftCsvPath();

    QMetaObject::invokeMethod(m_csvWorker, [this]() {
        if (m_csvLoggingEnable) {
            m_csvWorker->setOutputPath(m_eegCsvPath); // 主 CSV
            m_csvWorker->start();
        }
        if (m_psdLoggingEnable) {
            m_csvWorker->setSpectrumOutputPath(m_psdCsvPath); // PSD CSV
        } else {
            m_csvWorker->setSpectrumOutputPath(QString());
        }

        if (m_fftLoggingEnable) {
            m_csvWorker->setFftOutputPath(m_fftCsvPath); // FFT CSV
        } else {
            m_csvWorker->setFftOutputPath(QString());
        }
    }, Qt::QueuedConnection);

    // 5) 再启动采集（AcquisitionEngine::start 内会 resetSequence）
   /* QMetaObject::invokeMethod(m_acq, [this, port, baud]() {
        m_acq->start(port.trimmed(), baud);
    }, Qt::QueuedConnection);*/
}

void MainWindow::on_pushButton_picture_clicked()
{
    showChartModeDialog();
}

bool MainWindow::showChartModeDialog()
{
    appendUiActionLog(QStringLiteral("UI"), QStringLiteral("打开图表模式对话框"));
    ChartModeDialog dlg(this);
    dlg.setChartMode(static_cast<int>(m_chartMode));
    if (dlg.exec() != QDialog::Accepted) {
        appendUiActionLog(QStringLiteral("UI"), QStringLiteral("图表模式设置已取消"));
        return false;
    }

    m_chartMode = static_cast<ChartMode>(dlg.selectedChartMode());
    QString modeName;
    switch (m_chartMode) {
    case ChartMode::RawTime:
        modeName = QStringLiteral("预处理时域");
        break;
    case ChartMode::FftSpectrum:
        modeName = QStringLiteral("FFT 频谱");
        break;
    case ChartMode::BandPower:
        modeName = QStringLiteral("功率频段(PSD)");
        break;
    }
    appendUiActionLog(QStringLiteral("UI"), QStringLiteral("图表模式已切换: %1").arg(modeName));
    // 切换图表类型时，重置到自动跟随，避免沿用上一种图的历史坐标导致“看不见曲线”。
    m_chartAutoFollow = true;
    m_plotDirty = true;
    return true;
}

void MainWindow::ensureGraphCount(int count)
{
    if (!m_customPlot)
        return;
    while (m_customPlot->graphCount() < count) {
        m_customPlot->addGraph();
        m_customPlot->graph(m_customPlot->graphCount() - 1)->setSelectable(QCP::stDataRange);
    }
}

void MainWindow::setupPlotInteractions()
{
    if (!m_customPlot)
        return;
    m_customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    m_customPlot->axisRect()->setRangeDrag(Qt::Horizontal);
    m_customPlot->axisRect()->setRangeZoom(Qt::Horizontal);

    connect(m_customPlot->xAxis, qOverload<const QCPRange &>(&QCPAxis::rangeChanged), this, [this](const QCPRange &) {
        if (m_updatingPlotRange)
            return;
        m_chartAutoFollow = false;
    });
    connect(m_customPlot, &QCustomPlot::mouseDoubleClick, this, [this](QMouseEvent *) {
        m_chartAutoFollow = true;
        m_plotDirty = true;
        appendUiActionLog(QStringLiteral("PLOT"), QStringLiteral("双击图表: 恢复 X 轴自动跟随"));
    });
    if (!m_clickMarker) {
        m_clickMarker = new QCPItemTracer(m_customPlot);
        m_clickMarker->setClipToAxisRect(true);
        m_clickMarker->position->setType(QCPItemPosition::ptPlotCoords);
        m_clickMarker->position->setAxes(m_customPlot->xAxis, m_customPlot->yAxis);
        m_clickMarker->setStyle(QCPItemTracer::tsCircle);
        m_clickMarker->setPen(QPen(QColor(255, 80, 80), 2));
        m_clickMarker->setBrush(QBrush(QColor(255, 80, 80)));
        m_clickMarker->setSize(10);
        m_clickMarker->setVisible(false);
        m_clickMarker->setLayer(QStringLiteral("overlay"));
    }
    connect(
        m_customPlot,
        &QCustomPlot::plottableClick,
        this,
        [this](QCPAbstractPlottable *plottable, int dataIndex, QMouseEvent *event) {
            if (!plottable || dataIndex < 0 || !event)
                return;
            auto *i1d = plottable->interface1D();
            if (!i1d || dataIndex >= i1d->dataCount())
                return;
            const double x = i1d->dataMainKey(dataIndex);
            const double y = i1d->dataMainValue(dataIndex);
            const QString name = plottable->name().isEmpty() ? QStringLiteral("curve") : plottable->name();
            QString xShow;
            if (m_chartMode == ChartMode::FftSpectrum || m_chartMode == ChartMode::BandPower) {
                const QDateTime dt = QDateTime::fromMSecsSinceEpoch(qRound64(x * 1000.0), Qt::LocalTime);
                xShow = dt.isValid() ? dt.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"))
                                     : QString::number(x, 'f', 3);
            } else {
                xShow = QString::number(x, 'f', 2);
            }
            const QString text = QStringLiteral("%1\nt=%2, y=%3").arg(name).arg(xShow).arg(y, 0, 'f', 3);
            if (m_clickMarker) {
                m_clickMarker->position->setCoords(x, y);
                m_clickMarker->setVisible(true);
                m_customPlot->replot(QCustomPlot::rpQueuedReplot);
            }
            QToolTip::showText(event->globalPosition().toPoint(), text, m_customPlot);
            if (ui && ui->statusbar) {
                ui->statusbar->showMessage(QStringLiteral("%1: t=%2, y=%3").arg(name).arg(xShow).arg(y, 0, 'f', 3),
                                           2500);
            }
        }
    );
}

void MainWindow::clearPlotDisplay()
{
    m_plotDirty = false;
    m_chartAutoFollow = true;
    if (!m_customPlot)
        return;
    if (m_clickMarker)
        m_clickMarker->setVisible(false);
    for (int i = 0; i < m_customPlot->graphCount(); ++i) {
        if (m_customPlot->graph(i))
            m_customPlot->graph(i)->data()->clear();
    }
    m_customPlot->replot(QCustomPlot::rpQueuedReplot);
}

void MainWindow::renderRawChart()
{
    if (!m_customPlot)
        return;
    const int n = m_plotCache.size();
    if (n <= 0)
        return;

    ensureGraphCount(1);
    const int visibleCount = qMax(64, m_rawFixedDisplayCount);
    QVector<double> x(n), y(n);
    for (int i = 0; i < n; ++i) {
        x[i] = i;
        y[i] = m_plotCache[i];
    }
    m_customPlot->graph(0)->setData(x, y, true);
    for (int i = 1; i < m_customPlot->graphCount(); ++i)
        m_customPlot->graph(i)->data()->clear();
    m_customPlot->legend->setVisible(false);
    m_customPlot->graph(0)->setName(QStringLiteral("Raw/Preproc"));
    m_customPlot->graph(0)->setPen(QPen(QColor(0, 210, 170), 1));
    applySampleIndexXAxis(m_customPlot);
    m_customPlot->yAxis->setLabel(QStringLiteral("uV"));

    // Y 轴按当前窗口数据自适应，保证负半轴也能完整看到波形。
    double yMin = -1.0;
    double yMax = 1.0;
    if (!y.isEmpty()) {
        const auto mm = std::minmax_element(y.cbegin(), y.cend());
        yMin = *mm.first;
        yMax = *mm.second;
    }
    const double maxAbs = qMax(qAbs(yMin), qAbs(yMax));
    const double baseAbs = qBound(5.0, maxAbs * 1.15, 200.0);
    m_updatingPlotRange = true;
    m_customPlot->yAxis->setRange(-baseAbs, baseAbs);
    m_updatingPlotRange = false;

    if (m_chartAutoFollow) {
        const int end = n;
        m_updatingPlotRange = true;
        m_customPlot->xAxis->setRange(qMax(0, end - visibleCount), qMax(visibleCount, end));
        m_updatingPlotRange = false;
    }
    m_customPlot->replot(QCustomPlot::rpQueuedReplot);
}

void MainWindow::renderFftChart()
{
    if (!m_customPlot)
        return;
    const int n = m_fftCache.size();
    if (n <= 0)
        return;
    ensureGraphCount(5);
    applyLocalDateTimeXAxis(m_customPlot);
    QVector<double> x(n), d(n), t(n), a(n), b(n), g(n);
    for (int i = 0; i < n; ++i) {
        x[i] = bandPointTimeKey(m_fftCache[i].wallMs, m_fftCache[i].tsMs);
        d[i] = m_fftCache[i].delta;
        t[i] = m_fftCache[i].theta;
        a[i] = m_fftCache[i].alpha;
        b[i] = m_fftCache[i].beta;
        g[i] = m_fftCache[i].gamma;
    }
    m_customPlot->graph(0)->setData(x, d, true);
    m_customPlot->graph(1)->setData(x, t, true);
    m_customPlot->graph(2)->setData(x, a, true);
    m_customPlot->graph(3)->setData(x, b, true);
    m_customPlot->graph(4)->setData(x, g, true);
    m_customPlot->graph(0)->setName(QStringLiteral("δ Delta"));
    m_customPlot->graph(1)->setName(QStringLiteral("θ Theta"));
    m_customPlot->graph(2)->setName(QStringLiteral("α Alpha"));
    m_customPlot->graph(3)->setName(QStringLiteral("β Beta"));
    m_customPlot->graph(4)->setName(QStringLiteral("γ Gamma"));
    m_customPlot->graph(0)->setPen(QPen(QColor(93, 173, 226), 2));   // 蓝
    m_customPlot->graph(1)->setPen(QPen(QColor(88, 214, 141), 2));   // 绿
    m_customPlot->graph(2)->setPen(QPen(QColor(245, 176, 65), 2));   // 橙
    m_customPlot->graph(3)->setPen(QPen(QColor(236, 112, 99), 2));   // 红
    m_customPlot->graph(4)->setPen(QPen(QColor(175, 122, 197), 2));  // 紫
    m_customPlot->legend->setVisible(true);
    m_customPlot->legend->setBrush(QColor(20, 24, 31, 180));
    m_customPlot->legend->setBorderPen(QPen(QColor(80, 90, 110)));
    m_customPlot->legend->setTextColor(QColor(220, 230, 245));
    m_customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop | Qt::AlignRight);
    m_customPlot->yAxis->setLabel(QStringLiteral("FFT Amp"));
    if (m_chartAutoFollow) {
        const int visibleCount = qMin(m_featureCacheMax, 120);
        const int i0 = qMax(0, n - visibleCount);
        const double xMin = x[i0];
        const double xMax = x[n - 1];
        const double span = qMax(xMax - xMin, 1e-6);
        const double pad = qMax(0.05 * span, 0.05);
        m_updatingPlotRange = true;
        m_customPlot->xAxis->setRange(xMin - pad, xMax + pad);
        m_updatingPlotRange = false;
    } else {
        const auto xr = m_customPlot->xAxis->range();
        if (xr.upper < x.first() || xr.lower > x.last()) {
            const int visibleCount = qMin(m_featureCacheMax, 120);
            const int i0 = qMax(0, n - visibleCount);
            const double xMin = x[i0];
            const double xMax = x[n - 1];
            const double span = qMax(xMax - xMin, 1e-6);
            const double pad = qMax(0.05 * span, 0.05);
            m_updatingPlotRange = true;
            m_customPlot->xAxis->setRange(xMin - pad, xMax + pad);
            m_updatingPlotRange = false;
        }
    }
    auto minMax = std::minmax_element(d.cbegin(), d.cend());
    double yMin = *minMax.first, yMax = *minMax.second;
    auto updateRange = [&yMin, &yMax](const QVector<double> &v){
        if (v.isEmpty()) return;
        const auto mm = std::minmax_element(v.cbegin(), v.cend());
        yMin = qMin(yMin, *mm.first);
        yMax = qMax(yMax, *mm.second);
    };
    updateRange(t); updateRange(a); updateRange(b); updateRange(g);
    if (yMax - yMin < 0.5) {
        yMax += 0.25;
        yMin -= 0.25;
    }
    const double pad = (yMax - yMin) * 0.15;
    m_customPlot->yAxis->setRange(yMin - pad, yMax + pad);
    m_customPlot->replot(QCustomPlot::rpQueuedReplot);
}

void MainWindow::renderBandPowerChart()
{
    if (!m_customPlot)
        return;
    const int n = m_psdCache.size();
    if (n <= 0)
        return;
    ensureGraphCount(5);
    applyLocalDateTimeXAxis(m_customPlot);
    QVector<double> x(n), d(n), t(n), a(n), b(n), g(n);
    for (int i = 0; i < n; ++i) {
        x[i] = bandPointTimeKey(m_psdCache[i].wallMs, m_psdCache[i].tsMs);
        d[i] = m_psdCache[i].delta;
        t[i] = m_psdCache[i].theta;
        a[i] = m_psdCache[i].alpha;
        b[i] = m_psdCache[i].beta;
        g[i] = m_psdCache[i].gamma;
    }
    m_customPlot->graph(0)->setData(x, d, true);
    m_customPlot->graph(1)->setData(x, t, true);
    m_customPlot->graph(2)->setData(x, a, true);
    m_customPlot->graph(3)->setData(x, b, true);
    m_customPlot->graph(4)->setData(x, g, true);
    m_customPlot->graph(0)->setName(QStringLiteral("δ Delta"));
    m_customPlot->graph(1)->setName(QStringLiteral("θ Theta"));
    m_customPlot->graph(2)->setName(QStringLiteral("α Alpha"));
    m_customPlot->graph(3)->setName(QStringLiteral("β Beta"));
    m_customPlot->graph(4)->setName(QStringLiteral("γ Gamma"));
    m_customPlot->graph(0)->setPen(QPen(QColor(52, 152, 219), 2));   // 蓝
    m_customPlot->graph(1)->setPen(QPen(QColor(46, 204, 113), 2));   // 绿
    m_customPlot->graph(2)->setPen(QPen(QColor(241, 196, 15), 2));   // 黄
    m_customPlot->graph(3)->setPen(QPen(QColor(231, 76, 60), 2));    // 红
    m_customPlot->graph(4)->setPen(QPen(QColor(155, 89, 182), 2));   // 紫
    m_customPlot->legend->setVisible(true);
    m_customPlot->legend->setBrush(QColor(20, 24, 31, 180));
    m_customPlot->legend->setBorderPen(QPen(QColor(80, 90, 110)));
    m_customPlot->legend->setTextColor(QColor(220, 230, 245));
    m_customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop | Qt::AlignRight);
    m_customPlot->yAxis->setLabel(QStringLiteral("Band Power"));
    if (m_chartAutoFollow) {
        const int visibleCount = qMin(m_featureCacheMax, 120);
        const int i0 = qMax(0, n - visibleCount);
        const double xMin = x[i0];
        const double xMax = x[n - 1];
        const double span = qMax(xMax - xMin, 1e-6);
        const double pad = qMax(0.05 * span, 0.05);
        m_updatingPlotRange = true;
        m_customPlot->xAxis->setRange(xMin - pad, xMax + pad);
        m_updatingPlotRange = false;
    } else {
        const auto xr = m_customPlot->xAxis->range();
        if (xr.upper < x.first() || xr.lower > x.last()) {
            const int visibleCount = qMin(m_featureCacheMax, 120);
            const int i0 = qMax(0, n - visibleCount);
            const double xMin = x[i0];
            const double xMax = x[n - 1];
            const double span = qMax(xMax - xMin, 1e-6);
            const double pad = qMax(0.05 * span, 0.05);
            m_updatingPlotRange = true;
            m_customPlot->xAxis->setRange(xMin - pad, xMax + pad);
            m_updatingPlotRange = false;
        }
    }
    auto minMax = std::minmax_element(d.cbegin(), d.cend());
    double yMin = *minMax.first, yMax = *minMax.second;
    auto updateRange = [&yMin, &yMax](const QVector<double> &v){
        if (v.isEmpty()) return;
        const auto mm = std::minmax_element(v.cbegin(), v.cend());
        yMin = qMin(yMin, *mm.first);
        yMax = qMax(yMax, *mm.second);
    };
    updateRange(t); updateRange(a); updateRange(b); updateRange(g);
    if (yMax - yMin < 0.5) {
        yMax += 0.25;
        yMin -= 0.25;
    }
    const double pad = (yMax - yMin) * 0.15;
    m_customPlot->yAxis->setRange(yMin - pad, yMax + pad);
    m_customPlot->replot(QCustomPlot::rpQueuedReplot);
}

















