#include "thinkgearlinktester.h"
#include "device/serialport.h"
#include "thinkgear/thinkgearframeassembler.h"
#include "thinkgear/thinkgearpayloadparser.h"
#include "thinkgear/rawtouvprocessor.h"
#include <QDateTime>
#include <QDebug>
static int n = 0;
ThinkGearLinkTester::ThinkGearLinkTester(QObject *parent)
    : QObject{parent}
{
    m_serial= new SerialPort(this);
    m_assembler=new ThinkGearFrameAssembler(this);
    m_parser= new ThinkGearPayloadParser(this);
    m_converter =new RawtOutUvProcessor (this);

    //链路链接
    connect(m_serial,&SerialPort::dataReceived,
            m_assembler,&ThinkGearFrameAssembler::onBytes);
    connect(m_assembler,&ThinkGearFrameAssembler::frameReady,
             this,&ThinkGearLinkTester::onFrameReady);
    connect(m_assembler,&ThinkGearFrameAssembler::frameReady,
            m_parser,&ThinkGearPayloadParser::parseFrame);
    connect(m_parser,&ThinkGearPayloadParser::rawReceived,
            m_converter,&RawtOutUvProcessor::onRaw);
    connect(m_converter,&RawtOutUvProcessor::uvValueReady,
            this,&ThinkGearLinkTester::onRawUvReady);
    connect(m_parser, &ThinkGearPayloadParser::parseWarning,
            this, [this](const QString &msg) {
                ++m_warningCountSec;
             static int printed = 0;
        if (printed < 10) {
            qWarning().noquote() << "[parseWarning]" << msg;
            ++printed;
        }

            });

    connect(m_serial,&SerialPort::portError,
            this,&ThinkGearLinkTester::onPortError);

    connect(m_serial, &SerialPort::dataReceived, this, [this](const QByteArray &chunk) {
        if (++n > 10)  // 只打前 50 包
            return;
        qDebug().noquote() << n << SerialPort::toHexSpaced(chunk)<<"\n";
    });
    m_tick.setInterval(1000);
    connect(&m_tick,&QTimer::timeout,this,
            &ThinkGearLinkTester::onTick1s);

    connect(m_parser,&ThinkGearPayloadParser::rawReceived,this,&ThinkGearLinkTester::onRawInt16Ready);

}
void ThinkGearLinkTester::onRawInt16Ready(qint16 raw)
{
    m_lastRawInt16=raw;
}
ThinkGearLinkTester::~ThinkGearLinkTester()
{
    stop();

}

bool ThinkGearLinkTester::start(const QString &portName,qint32 baudRate)
{
    if(m_serial&&m_serial->isOpen())
        m_serial->close();
    shutdownLogWriter();


    //stop();
    m_pictureandalgBuffer.clear();
    m_pictureandalgBuffer.resetAlgoCursor();
    m_pictureandalgBuffer.resetPlotCursor();
    m_seq = 0;

    if(m_csvLoggingEnabled)
        startupLogWriter();

    SerialPortConfig cfg;
    cfg.portName=portName;
    cfg.baudRate =baudRate;
    cfg.readBufferSize=32*1024;
    m_serial->setConfig(cfg);

    const bool ok =m_serial->openReadOnly();
    if(!ok)
    {
        emit testMessage(QStringLiteral("串口打开失败"));
        return false;
    }

    m_rawCountSec=0;
    m_frameCountSec=0;
    m_warningCountSec=0;
    m_secIndex=0;
    m_tick.start();
    emit testMessage(QStringLiteral("测试开始：等待每秒统计..."));
    return true;

}
void ThinkGearLinkTester::stop()
{
    if(m_tick.isActive())
        m_tick.stop();
    if(m_serial&&m_serial->isOpen())
        m_serial->close();
    shutdownLogWriter();
}
void ThinkGearLinkTester::startupLogWriter()
{
    if (!m_csvLoggingEnabled)
        return;
    if (m_csvOutputPath.trimmed().isEmpty()) {
        emit testMessage(QStringLiteral("已勾选 CSV 但未设置输出路径"));
        return;
    }
    if (m_logThread)
        return; // 已在跑（可按需改成先 shutdown 再建）

    m_logThread = new QThread(this);
    m_logWorker = new CsvLogWorker(&m_logBuffer);
    m_logWorker->setOutputPath(m_csvOutputPath);
    m_logWorker->setFlushIntervalMs(100);
    m_logWorker->setBatchSize(1024);
    m_logWorker->moveToThread(m_logThread);

    QObject::connect(m_logThread, &QThread::started, m_logWorker, &CsvLogWorker::start);
    QObject::connect(m_logThread, &QThread::finished, m_logWorker, &QObject::deleteLater);
    QObject::connect(m_logWorker, &CsvLogWorker::workerError, this, [this](const QString &msg) {
        emit testMessage(QStringLiteral("CSV: %1").arg(msg));
    });

    m_logThread->start();
}
void ThinkGearLinkTester::shutdownLogWriter()
{
    if (m_logWorker) {
        QMetaObject::invokeMethod(m_logWorker, "stop", Qt::QueuedConnection);
    }
    if (m_logThread) {
        m_logThread->quit();
        m_logThread->wait(2000);
        m_logThread->deleteLater();
        m_logThread = nullptr;
        m_logWorker = nullptr;
    }
}
void ThinkGearLinkTester::onRawUvReady(double uv)
{

    ++m_rawCountSec;
    //const QDateTime nowMs = QDateTime::currentDateTime();
    //currTimeStr = now.toString(Qt::TextDate);
    //const quint64 nowMs =QDateTime::currentMSecsSinceEpoch();
    const quint64 seq =++m_seq;
    QDateTime now = QDateTime::currentDateTime();
    currTimeStr = now.toString(Qt::TextDate);
    RawSample rs;
    rs.raw= uv;
    rs.time =currTimeStr;
    rs.raw_count=seq;
    m_pictureandalgBuffer.appendRawvalue(rs);


    LogItem li;
    const qint64 ms = QDateTime::currentMSecsSinceEpoch();
    li.wallMs = ms;
    li.tsMs = QDateTime::fromMSecsSinceEpoch(ms, Qt::LocalTime)
                  .toString(QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"));
    li.seq=seq;
    li.rawUv=uv;
    li.rawInt16=m_lastRawInt16;
    m_logBuffer.push(li);
}

void ThinkGearLinkTester::onFrameReady(const QByteArray &frame)
{
    Q_UNUSED(frame);
    ++m_frameCountSec;
}

void ThinkGearLinkTester::onParseWarning(const QString &msg)
{
    Q_UNUSED(msg);
    ++m_warningCountSec;
}

void ThinkGearLinkTester::onPortError(const QString &msg)
{
    emit testMessage(QStringLiteral("串口错误: %1").arg(msg));
}
static int flag=0;
void ThinkGearLinkTester::onTick1s()
{
    ++m_secIndex;


    m_algBlocksSec = 0;
    m_plotBlocksSec = 0;
    QVector<RawSample> chunk;
    while (m_pictureandalgBuffer.tryDequeueRawChunkForPlot(128,chunk))
    {
        ++m_plotBlocksSec;

        if(flag<5){
        qDebug() << "plot chunk size:" << chunk.size();
        for (int i = 0; i < chunk.size(); ++i) {
            const RawSample &s = chunk[i];
            qDebug() << i << "raw_count" << s.raw_count << "raw_uV" << s.raw << "time" << s.time;
        }
        flag++;
        }
    }
    while(m_pictureandalgBuffer.tryDequeueRawChunkForAlgo(128,chunk)){
        ++m_algBlocksSec;
    }
    const int low =m_targetRawPerSec-m_tolerance;
    const int hight =m_targetRawPerSec+m_tolerance;
    const bool pass =(m_rawCountSec>=low&&m_rawCountSec<=hight);
    emit secondReport(m_secIndex, m_rawCountSec, m_frameCountSec, m_warningCountSec, pass);

    // 控制台打印便于快速观察
    /*qDebug().noquote()
        << QString("sec=%1 raw/s=%2 frame/s=%3 warn/s=%4 pass=%5")
               .arg(m_secIndex)
               .arg(m_rawCountSec)
               .arg(m_frameCountSec)
               .arg(m_warningCountSec)
               .arg(pass ? "YES" : "NO");*/

    qDebug().noquote()
        << QString("[CHUNK] sec=%1 algoBlocks=%2 plotBlocks=%3 logQ=%4")
               .arg(m_secIndex)
               .arg(m_algBlocksSec)
               .arg(m_plotBlocksSec)
               .arg(m_logBuffer.size());

    // 清零进入下一秒
    m_rawCountSec = 0;
    m_frameCountSec = 0;
    m_warningCountSec = 0;
}

















