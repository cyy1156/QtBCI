#include "logwbuffer.h"

LogBuffer::LogBuffer(int capacity)
    : m_capacity(capacity > 0 ? capacity : 1) {}
bool LogBuffer::push(const LogItem &items)
{
    QMutexLocker locker(&m_mutex);
    if(m_q.size()>=m_capacity)
    {
        ++m_dropped;
        return false;

    }
    m_q.enqueue(items);
    return true;
}
QList<LogItem> LogBuffer::drainBatch(int maxItems)
{
    QMutexLocker locker(&m_mutex);
    QList<LogItem> out;
    const int n = qMin(maxItems, m_q.size());
    out.reserve(n);
    for (int i = 0; i < n; ++i) out.push_back(m_q.dequeue());
    return out;
}

int LogBuffer::droppedCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_dropped;
}
int LogBuffer::size() const
{
    QMutexLocker locker(&m_mutex);
    return m_q.size();
}

void LogBuffer::clear(bool resetDropped)
{
    QMutexLocker locker(&m_mutex);
    m_q.clear();
    if (resetDropped)
        m_dropped = 0;
}
