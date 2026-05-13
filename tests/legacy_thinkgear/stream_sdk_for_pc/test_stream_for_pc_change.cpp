
#include "stream_sdk_for_pc.h"
#include "../thinkgear.h"
#include <QObject>
#include <array>
#include <QApplication>
#include <QDebug>
#include <iostream>
#include <limits>
#include <cstdlib>
#include <ctime>
#include <QDateTime>
#include <limits>
#include <cmath>
#include <cstdint>
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
int runtest()
{
    double secondsToRun = 5;
    time_t startTime1 = time(nullptr);
    QString currTimeStr;
    int count = 0; // RAW 样本计数
    int noPacketRounds = 0;

    Stream_sdk_for_pc* in = new Stream_sdk_for_pc();
    EegFrame eeg;
    char defaultComPort[] = "COM7";
    char* comPortName =  defaultComPort;
    int serialBaudrate = TG_BAUD_57600;
    int serialDataFormat = TG_STREAM_PACKETS;

    if (!in->Init(comPortName, serialBaudrate, serialDataFormat)) {
        qDebug() << "初始化失败，请确认串口是否正确。可用方式：程序名 COMx";
        delete in;
        return -1;
    }
    qDebug() << "当前串口:" << comPortName;

    while (difftime(time(nullptr), startTime1) < secondsToRun)
    {
        if( !(in->ReadOnePackage()))
        {
            count++;
            QDateTime now = QDateTime::currentDateTime();
            currTimeStr = now.toString(Qt::TextDate);
            qDebug()<<count<< "|" << currTimeStr << "|"<<"丢包";

        }

        // 1) RAW：有更新才计数
        if (in->GetRawValue(eeg.raw)) {
            noPacketRounds = 0;
            QDateTime now = QDateTime::currentDateTime();
            currTimeStr = now.toString(Qt::TextDate);
            ++count;
            qDebug() << count << "|" << currTimeStr << "|" << "原始信号(μV)" << eeg.raw;
        } else {
            ++noPacketRounds;
            if (noPacketRounds % 500 == 0) {
                //qDebug() << "尚未读到RAW数据，请检查设备佩戴、电极接触和串口号。当前串口:" << comPortName;
            }
        }

        // 2) 大包字段：按状态位更新，避免“恰好第513次时未更新”导致全 0
        bool gotBigField = false;
        if (in->GetSingal(eeg.signal)) gotBigField = true;
        if (in->GetATT(eeg.att)) gotBigField = true;
        if (in->GetMed(eeg.med)) gotBigField = true;

        uint32_t v = 0;
        if (in->GetDelta(v))       { eeg.band[(int)Band::Delta] = v; gotBigField = true; }
        if (in->Gettheta(v))       { eeg.band[(int)Band::Theta] = v; gotBigField = true; }
        if (in->GetLowAlphal(v))   { eeg.band[(int)Band::Alpha1] = v; gotBigField = true; }
        if (in->GetHeightAlphal(v)){ eeg.band[(int)Band::Alpha2] = v; gotBigField = true; }
        if (in->GetLowbeta(v))     { eeg.band[(int)Band::Beta1] = v; gotBigField = true; }
        if (in->GetHeightbeta(v))  { eeg.band[(int)Band::Beta2] = v; gotBigField = true; }
        if (in->GetLowGamma(v))    { eeg.band[(int)Band::Gamma1] = v; gotBigField = true; }
        if (in->GetMiddleGame(v))  { eeg.band[(int)Band::Gamma2] = v; gotBigField = true; }

        if (gotBigField) {
            QDateTime now = QDateTime::currentDateTime();
            currTimeStr = now.toString(Qt::TextDate);
            qDebug() << "---- 大包字段更新 ----" << currTimeStr;
            qDebug() << "信号质量(0-200)" << eeg.signal<<"\n"
                     << "| Attention(0-100)" << eeg.att<<"\n"
                     << "| Meditation(0-100)" << eeg.med;

            qDebug() << "EEG Power(dec):"<<"\n"
                     << "Delta（频率范围0.5–2.75 Hz" << eeg.band[(int)Band::Delta]<<"\n"
                     <<  "Theta（频率范围3.5–6.75 Hz）" << eeg.band[(int)Band::Theta]<<"\n"
                     << "LowAlpha（频率范围7.5–9.25 Hz）" << eeg.band[(int)Band::Alpha1]<<"\n"
                     << "HighAlpha（频率范围10–11.75 Hz	）" << eeg.band[(int)Band::Alpha2]<<"\n"
                     << "LowBeta（频率范围13–16.75 Hz	）" << eeg.band[(int)Band::Beta1]<<"\n"
                     << "HighBeta（频率范围18–29.75 Hz）" << eeg.band[(int)Band::Beta2]<<"\n"
                     << "LowGamma（频率范围31–39.75 Hz）" << eeg.band[(int)Band::Gamma1]<<"\n"
                     << "MiddleGamma（频率范围41–49.75 Hz）" << eeg.band[(int)Band::Gamma2];
        }
        if(count==513)
        {
            count=0;
        }
    }

    qDebug() << "数据收集完成";
    delete in;

    return 0;
}
