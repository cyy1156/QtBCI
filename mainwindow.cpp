#include "mainwindow.h"
#include "ui_mainwindow.h"

// 旧单链路对象（ThinkGearLinkTester）相关 include，注释保留
// #include "thinkgear/thinkgearlinktester.h"

#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QLineEdit>
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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

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

    // 右侧区域上下比例：图像 60% / 日志 40%（日志区更大）
    if (ui->verticalLayout)
    {
        ui->verticalLayout->setStretch(0, 6); // widget_4: 图像
        ui->verticalLayout->setStretch(1, 4); // widget_5: 日志
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
            m_customPlot->setNotAntialiasedElements(QCP::aeAll); // 降低 CPU 占用
            m_customPlot->xAxis->setLabel(QStringLiteral("Sample"));
            m_customPlot->yAxis->setLabel(QStringLiteral("uV"));
            m_customPlot->xAxis->setRange(0, m_plotCacheMax);
            m_customPlot->yAxis->setRange(-80, 80); // 固定量程避免每帧自适应卡顿

            layout->replaceWidget(ui->listWidget_picture, m_customPlot);
            ui->listWidget_picture->hide();
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
            });
            connect(m_btnPauseAutoScroll, &QPushButton::clicked, this, [this]() {
                m_autoScrollEnabled = !m_autoScrollEnabled;
                if (!m_btnPauseAutoScroll) return;
                if (m_autoScrollEnabled) {
                    m_pausedNewLogCount = 0;
                    m_btnPauseAutoScroll->setText(QStringLiteral("暂停自动滚动"));
                    if (m_logText && m_logText->verticalScrollBar())
                        m_logText->verticalScrollBar()->setValue(m_logText->verticalScrollBar()->maximum());
                } else {
                    m_btnPauseAutoScroll->setText(QStringLiteral("恢复自动滚动"));
                }
            });
        }
    }

    m_plotUiTimer.setInterval(30); // ~20 FPS，进一步减轻卡顿
    m_plotUiTimer.setTimerType(Qt::CoarseTimer);
    connect(&m_plotUiTimer, &QTimer::timeout, this, &MainWindow::onPlotRefreshTick);
    m_plotUiTimer.start();
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
        if (m_plotCache.size() > m_plotCacheMax) {
            m_plotCache.remove(0, m_plotCache.size() - m_plotCacheMax);
        }
        m_plotDirty = true;

        // 2) 这里是 UI 线程，后续可直接用 m_plotCache 更新 QCustomPlot
        // 目前使用定时器统一刷新，避免每个 chunk 都重绘导致 UI 卡顿

        // 3) 预处理后数据也进入同一个 CSV 队列
        const QString ts = QDateTime::currentDateTime().toString(Qt::TextDate);
        for (int i = 0; i < pc.y.size(); ++i) {
            LogItem li;
            li.tsMs = ts;
            li.kind = QStringLiteral("preproc");
            li.seq = pc.seqStart + static_cast<quint64>(i);
            li.rawInt16 = 0;
            li.signalQuality = 255;
            li.rawUv = std::numeric_limits<double>::quiet_NaN();
            li.preprocUv = pc.y[i];
            m_logBuffer.push(li);
        }
    });
    connect(m_alg, &AlgorithmEngine::algoResultReady, this, [this](const AlgoResult &res){
        appendLogLine(QStringLiteral("[ALG] seq=%1 score=%2 label=%3")
                          .arg(res.seqEnd)
                          .arg(res.score, 0, 'f', 3)
                          .arg(res.label));
    });

    // A/B -> LogBuffer（示例：在各自回调里 push）
    connect(m_acq, &AcquisitionEngine::rawPacketReady, this, [this](const RawPacket &p){
        LogItem li;
        li.tsMs = p.tsMs;
        li.kind = QStringLiteral("raw");
        li.seq = p.seq;
        li.rawInt16 = p.rawInt16;
        li.signalQuality = p.signalQuality;
        li.rawUv = p.rawUv;
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
    connect(m_acq, &AcquisitionEngine::statusMessage, this, [this](const QString &msg){
        appendLogLine(QStringLiteral("[ACQ][INFO] %1").arg(msg));
    });
    connect(m_acq, &AcquisitionEngine::warningMessage, this, [this](const QString &msg){
        appendLogLine(QStringLiteral("[ACQ][WARN] %1").arg(msg));
    });
    connect(m_acq, &AcquisitionEngine::runningChanged, this, [this](bool running){
        m_acqRunning = running;
    });
    connect(m_csvWorker, &CsvLogWorker::workerInfo, this, [this](const QString &msg){
        appendLogLine(QStringLiteral("[CSV][INFO] %1").arg(msg));
    });
    connect(m_csvWorker, &CsvLogWorker::workerError, this, [this](const QString &msg){
        appendLogLine(QStringLiteral("[CSV][ERROR] %1").arg(msg));
    });

    // 启动线程后再启动worker
    connect(m_logThread, &QThread::started, m_csvWorker, &CsvLogWorker::start);

    m_acqThread->start();
    m_algThread->start();
    m_logThread->start();
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
//操作txt日志写入
void MainWindow::appendUiActionLog(const QString &category, const QString &message)
{
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
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("三线程对象未初始化，请先调用 initThreads()"));
        return;
    }
    if (m_acqRunning) {
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

    // 操作日志 TXT
    if (m_uiTxtPath.isEmpty())
        m_uiTxtPath = makeDefaultUiTxtPath();
    appendUiActionLog(QStringLiteral("UI"), QStringLiteral("点击开始"));
   // updateLogCsvPathUi();
    updateSavePathUi();

    bool ok =false;
    const QString port =QInputDialog::getText(
        this,
        QStringLiteral("串口"),
        QStringLiteral("端口名（如 COM7）:"),
        QLineEdit::Normal,
        QStringLiteral("COM7"),
        &ok);
    if(!ok||port.trimmed().isEmpty())
        return;

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
    QMetaObject::invokeMethod(m_acq, [this, port]() {
        m_acq->start(port.trimmed(), 57600);
    }, Qt::QueuedConnection);

    ui->statusbar->showMessage(QStringLiteral("已打开 %1 @57600").arg(port.trimmed()), 5000);
}
void MainWindow::on_pushButton_stop_clicked()
{
    // 三线程主链停止
    if (!m_acqRunning) {
        ui->statusbar->showMessage(QStringLiteral("当前未在采集"), 2000);
    }
    if (m_acq) {
        QMetaObject::invokeMethod(m_acq, "stop", Qt::QueuedConnection);
    }
    if (m_csvWorker) {
        QMetaObject::invokeMethod(m_csvWorker, "stop", Qt::QueuedConnection);
    }

    // 旧单链路停止，注释保留
    /*
    if(m_tester)
        m_tester->stop();
    */
    //状态栏提醒3s之后自动消失
    ui->statusbar->showMessage(QStringLiteral("已经停止"),3000);
    appendUiActionLog(QStringLiteral("UI"), QStringLiteral("点击停止"));
}
void MainWindow::on_pushButton_save_clicked()
{
    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("保存配置"));
    dlg.setMinimumWidth(520);
    auto *lay = new QVBoxLayout(&dlg);

    auto *cbEnableCsv = new QCheckBox(QStringLiteral("开始采集时启用 EEG CSV 保存"), &dlg);
    cbEnableCsv->setChecked(m_csvLoggingEnable);
    auto *cbEnableUiTxt = new QCheckBox(QStringLiteral("启用操作日志 TXT 保存"), &dlg);
    cbEnableUiTxt->setChecked(m_uiTxtLoggingEnabled);
    auto *cbEnablePsv =new QCheckBox(QStringLiteral("开始采集时启用 经过Welch 非参数功率谱估计(PSD)算法保存"),&dlg);
    cbEnablePsv->setChecked(m_psdLoggingEnable);
    auto *cbEnableFft = new QCheckBox(QStringLiteral("开始采集时启用 FFT 频谱（五频段幅值）CSV 保存"), &dlg);
    cbEnableFft->setChecked(m_fftLoggingEnable);

    auto *labEeg = new QLabel(QStringLiteral("EEG CSV：\n%1")
                                  .arg(m_eegCsvPath.isEmpty() ? QStringLiteral("未设置") : m_eegCsvPath), &dlg);
    labEeg->setWordWrap(true);
    auto *labTxt = new QLabel(QStringLiteral("操作 TXT：\n%1")
                                  .arg(m_uiTxtPath.isEmpty() ? QStringLiteral("未设置") : m_uiTxtPath), &dlg);
    labTxt->setWordWrap(true);
    auto *labPsd =new QLabel(QStringLiteral("PSD CSV: \n%1").arg(m_psdCsvPath.isEmpty()?QStringLiteral("未设置"):m_psdCsvPath),&dlg);
    labPsd->setWordWrap(true);
    auto *labFft = new QLabel(QStringLiteral("FFT CSV：\n%1")
                                  .arg(m_fftCsvPath.isEmpty() ? QStringLiteral("未设置") : m_fftCsvPath), &dlg);
    labFft->setWordWrap(true);

    auto *btnNewEeg = new QPushButton(QStringLiteral("新建 EEG CSV"), &dlg);
    auto *btnOpenEeg = new QPushButton(QStringLiteral("打开 EEG CSV"), &dlg);
    auto *btnNewTxt = new QPushButton(QStringLiteral("新建 操作TXT"), &dlg);
    auto *btnOpenTxt = new QPushButton(QStringLiteral("打开 操作TXT"), &dlg);
    auto *btnNewPsd =new QPushButton(QStringLiteral("新建 PSD CSV"),&dlg);
    auto *btnOpenPsd =new QPushButton(QStringLiteral("打开 PSD CSV "),&dlg);
    auto *btnNewFft = new QPushButton(QStringLiteral("新建 FFT CSV"), &dlg);
    auto *btnOpenFft = new QPushButton(QStringLiteral("打开 FFT CSV"), &dlg);


    auto refresh = [&]() {
        labEeg->setText(QStringLiteral("EEG CSV：\n%1")
                            .arg(m_eegCsvPath.isEmpty() ? QStringLiteral("未设置") : m_eegCsvPath));
        labTxt->setText(QStringLiteral("操作 TXT：\n%1")
                            .arg(m_uiTxtPath.isEmpty() ? QStringLiteral("未设置") : m_uiTxtPath));
        labPsd->setText(QStringLiteral("PSD CSV： \n%1")
                             .arg(m_psdCsvPath.isEmpty()?QStringLiteral("未设置"):m_psdCsvPath));
        labFft->setText(QStringLiteral("FFT CSV：\n%1")
                            .arg(m_fftCsvPath.isEmpty() ? QStringLiteral("未设置") : m_fftCsvPath));
    };

    connect(btnNewEeg, &QPushButton::clicked, &dlg, [&]() {
        m_eegCsvPath = makeDefaultEegCsvPath();
        refresh();
    });
    connect(btnNewTxt, &QPushButton::clicked, &dlg, [&]() {
        m_uiTxtPath = makeDefaultUiTxtPath();
        refresh();
    });
    connect(btnNewPsd,&QPushButton::clicked,&dlg,[&](){
        m_psdCsvPath =makeDefaultPsdCsvPath();
        refresh();
    });
    connect(btnNewFft, &QPushButton::clicked, &dlg, [&]() {
        m_fftCsvPath = makeDefaultFftCsvPath();
        refresh();
    });


    connect(btnOpenEeg, &QPushButton::clicked, &dlg, [&]() {
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
    });

    connect(btnOpenTxt, &QPushButton::clicked, &dlg, [&]() {
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
    });
    connect(btnOpenPsd,&QPushButton::clicked,&dlg,[&](){
        if(m_psdCsvPath.isEmpty()||!QFileInfo::exists(m_psdCsvPath))
            {
            QMessageBox::information(&dlg,QStringLiteral("提示"),QStringLiteral("PSD CSV 文件不存在。"));
            return;
        }

#if defined(Q_OS_WIN)
    QProcess::startDetached(QStringLiteral("explorer.exe"),
                            {QStringLiteral("/select,"),QDir::toNativeSeparators(m_psdCsvPath) });
#else
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_PsdCsvPath));
#endif
    });

    connect(btnOpenFft, &QPushButton::clicked, &dlg, [&]() {
        if (m_fftCsvPath.isEmpty() || !QFileInfo::exists(m_fftCsvPath))
        {
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

    lay->addWidget(cbEnableCsv);
    lay->addWidget(cbEnableUiTxt);
    lay->addWidget(cbEnablePsv);
    lay->addWidget(cbEnableFft);
    lay->addWidget(labEeg);
    lay->addWidget(btnNewEeg);
    lay->addWidget(btnOpenEeg);
    lay->addSpacing(8);
    lay->addWidget(labTxt);
    lay->addWidget(btnNewTxt);
    lay->addWidget(btnOpenTxt);
    lay->addSpacing(8);
    lay->addWidget(labPsd);
    lay->addWidget(btnNewPsd);
    lay->addWidget(btnOpenPsd);
    lay->addSpacing(8);
    lay->addWidget(labFft);
    lay->addWidget(btnNewFft);
    lay->addWidget(btnOpenFft);

    auto *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    lay->addWidget(box);

    if (dlg.exec() != QDialog::Accepted)
        return;

    m_csvLoggingEnable = cbEnableCsv->isChecked();
    m_uiTxtLoggingEnabled = cbEnableUiTxt->isChecked();
    m_psdLoggingEnable=cbEnablePsv->isChecked();
    m_fftLoggingEnable = cbEnableFft->isChecked();
    if (m_csvLoggingEnable && m_eegCsvPath.isEmpty())
        m_eegCsvPath = makeDefaultEegCsvPath();
    if (m_uiTxtPath.isEmpty())
        m_uiTxtPath = makeDefaultUiTxtPath();
    if(m_psdLoggingEnable&&m_psdCsvPath.isEmpty())
        m_psdCsvPath=makeDefaultPsdCsvPath();
    if (m_fftLoggingEnable && m_fftCsvPath.isEmpty())
        m_fftCsvPath = makeDefaultFftCsvPath();

    updateSavePathUi();
    appendUiActionLog(QStringLiteral("UI"), QStringLiteral("保存配置：csvEnable=%1, eeg=%2, txt=%3")
                                                .arg(m_csvLoggingEnable).arg(m_eegCsvPath, m_uiTxtPath));

}
void MainWindow::onSecondReport(quint64 secIndex, int rawPerSec, int framePerSec, int warnPerSec, bool pass)
{
    const QString line = QStringLiteral("[%1] raw/s=%2 frame/s=%3 warn/s=%4 pass=%5")
    .arg(secIndex)
        .arg(rawPerSec)
        .arg(framePerSec)
        .arg(warnPerSec)
        .arg(pass ? QStringLiteral("OK") : QStringLiteral("NO"));
    appendLogLine(line);
    ui->statusbar->showMessage(line, 1000);//状态栏临时显示 1 秒。
    appendUiActionLog(QStringLiteral("STAT"), line);
}

void MainWindow::onTestMessage(const QString &msg)
{
    appendLogLine(msg);
    appendUiActionLog(QStringLiteral("SYS"), msg);
}

 void MainWindow::on_pushButton_clear_clicked()
{
    const auto ret = QMessageBox::question(
        this,
        QStringLiteral("确认"),
        QStringLiteral("清除并重置会话？这会停止并重新开始采集，seq 从 0 重新计数。"));
    if (ret != QMessageBox::Yes)
        return;

    // 这里你可保存上次端口，或弹窗重新选择
    const QString port = QStringLiteral("COM7");
    restartSessionWithReset(port, 57600);

    appendUiActionLog(QStringLiteral("UI"), QStringLiteral("点击清除并重置会话"));
}
void MainWindow::on_pushButton_openLogCsv_clicked()
{
    // 先留空也可以，至少先通过链接
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
        const int n = m_plotCache.size();
        if (n <= 0)
            return;

        QVector<double> x(n), y(n);
        for (int i = 0; i < n; ++i)
        {
            x[i] = i;
            y[i] = m_plotCache[i];
        }
        m_customPlot->graph(0)->setData(x, y, true);
        m_customPlot->xAxis->setRange(qMax(0, n - m_plotCacheMax), qMax(m_plotCacheMax, n));
        // y 轴使用固定量程，避免每帧 min/max 计算导致卡顿
        m_customPlot->replot(QCustomPlot::rpQueuedReplot);
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
    if (!m_acq || !m_alg || !m_csvWorker)
        return;

    // 1) 停止采集与写盘
    QMetaObject::invokeMethod(m_acq, "stop", Qt::QueuedConnection);
    QMetaObject::invokeMethod(m_csvWorker, "stop", Qt::QueuedConnection);

    // 2) 清算法状态（必须在算法线程执行）
    QMetaObject::invokeMethod(m_alg, "resetState", Qt::QueuedConnection);

    // 3) 清 UI 缓存（主线程）
    m_plotCache.clear();
    m_plotDirty = false;
    if (m_customPlot && m_customPlot->graphCount() > 0) {
        m_customPlot->graph(0)->data()->clear();
        m_customPlot->replot(QCustomPlot::rpQueuedReplot);
    }

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

















