



#include <QObject>
#include <array>
#include <QApplication>
#include <QDebug>
#include"stream_sdk_for_pc/stream_sdk_for_pc.h"
#include <cstdlib>



#include<stream_sdk_for_pc/origintest.h>
namespace test {
enum class Band : int {
    Delta = 0,
    Theta,
    Alpha1,
    Alpha2,
    Beta1,
    Beta2,
    Gamma1,
    Gamma2,
    Count
};

struct EegFrame {
    std::array<double, static_cast<int>(Band::Count)> band{}; // 默认全 0
    double raw = 0.0;   // μV
    short signal = 0;
    short att = 0;
    short med = 0;
};
class changtest
{
public:
    changtest() ;
    void run();
    void SampleAndDisplay();
public:
    bool m_flag=false;
private:
    EegFrame eeg;
    Stream_sdk_for_pc* in ;
    double secondsToRun = 3;
    time_t startTime1 = time(nullptr);
    QString currTimeStr;
    int count = 1; // RAW 样本计数


};

}
