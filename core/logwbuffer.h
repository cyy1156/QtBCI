#ifndef LOGWBUFFER_H
#define LOGWBUFFER_H


#include <QTimer>
#include <QString>
#include <QQueue>
#include <QMutex>
#include <QtGlobal>
struct LogItem {
    QString tsMs ;
    qint64 wallMs = -1; // UTC 毫秒，与 tsMs 一致；raw 必填便于整秒统计
    QString kind;      // raw / preproc
    quint64 seq = 0;
    quint64 gapSincePrev = 0;
    qint16 rawInt16 = 0;
    quint8 signalQuality = 255; // 0~200, 255=unknown
    double rawUv = 0.0;
    double preprocUv = 0.0;
};
class LogBuffer
{
public:
    explicit LogBuffer(int capacity =4096);
    bool push(const LogItem &item);              // 满队列返回 false
    QList<LogItem> drainBatch(int maxItems);     // writer 线程批量取
    int droppedCount() const;
    int size() const;
    void clear(bool resetDropped = true);
private:
    int m_capacity = 0;
    mutable QMutex m_mutex;
    QQueue<LogItem> m_q;
    int m_dropped = 0;
};

#endif // LOGWBUFFER_H
