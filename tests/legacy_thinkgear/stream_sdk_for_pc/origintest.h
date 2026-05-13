#ifndef ORIGINTEST_H
#define ORIGINTEST_H
#include <ctime>
#include <QString>
class origintest
{
public:
    origintest();
    void run();
public:
     bool m_flag=false;
private:
    char* comPortName =NULL;
    int dllVersion =0;
    int connectionId=0;
    int packetsRead =0;
    int errCode=0;

    double secondsToRun=0;
    time_t startTime1 =0;
    time_t startTime2 =0;
    time_t currTime =0;
    QString currTimeStr=NULL;
    int set_filter_falg=0;
    int count=0;

};

#endif // ORIGINTEST_H
