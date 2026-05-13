/*#include "changetest.h"

#include "mainwindow.h"
#include "stream_sdk_for_pc/stream_sdk_for_pc.h"
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
#include <QtGlobal>
#include <limits>
#include <cmath>
#include <cstdint>
#include<stream_sdk_for_pc/origintest.h>
namespace test {
    changtest::changtest() {
     in = new Stream_sdk_for_pc();
    char defaultComPort[] = "COM7";
    char* comPortName = defaultComPort;
    int serialBaudrate = TG_BAUD_57600;
    int serialDataFormat = TG_STREAM_PACKETS;

    if (!in->Init(comPortName, serialBaudrate, serialDataFormat)) {
        qDebug() << "初始化失败，请确认串口是否正确。可用方式：程序名 COMx";
        delete in;
        m_flag=false;

    }
    else{
        qDebug() << "当前串口:" << comPortName;
        m_flag=true;}
    }

    void changtest::run()
    {
        while (difftime(time(nullptr), startTime1) < secondsToRun)
        {
            if( !(in->ReadMorePackeage()))
            {
                count++;
                QDateTime now = QDateTime::currentDateTime();
                currTimeStr = now.toString(Qt::TextDate);
               // qDebug()<<count<< "|" << currTimeStr << "|"<<"丢包";

                    if (count >= 513)
                        count = 0;

                    // 可以加个小延时，避免CPU占用过高
                    // QThread::msleep(5);
                    continue;


            }

            // 1) RAW：有更新才计数
            if (in->GetRawValue(eeg.raw)) {

                QDateTime now = QDateTime::currentDateTime();
                currTimeStr = now.toString(Qt::TextDate);
                ++count;
                if (count >= 513)
                    count = 0;
                qDebug() << count << "|" << currTimeStr << "|" << "原始信号(μV)" << eeg.raw;
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
    }


    void changtest::SampleAndDisplay()
    {
        while (difftime(time(nullptr), startTime1) < secondsToRun)
        {
            if( !(in->ReadOnePackage()))
            {
                count++;
                QDateTime now = QDateTime::currentDateTime();
                currTimeStr = now.toString(Qt::TextDate);
                // qDebug()<<count<< "|" << currTimeStr << "|"<<"丢包";

                if (count >= 513)
                    count = 1;

                // 可以加个小延时，避免CPU占用过高
                // QThread::msleep(5);
                continue;


            }

            // 1) RAW：有更新才计数
            if (in->GetRawValue(eeg.raw)) {
                RawSample s;

                QDateTime now = QDateTime::currentDateTime();
                currTimeStr = now.toString(Qt::TextDate);
                ++count;
                if (count >= 513)
                    count = 1;
                s.raw_count=count;
                s.raw=eeg.raw;
                s.time=currTimeStr;
                in->buffer().appendRawvalue(s);

                //qDebug() << count << "|" << currTimeStr << "|" << "原始信号(μV)" << eeg.raw;
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
                         << "| Meditation(0-100)" << eeg.med;*/

                /*qDebug() << "EEG Power(dec):"<<"\n"
                         << "Delta（频率范围0.5–2.75 Hz" << eeg.band[(int)Band::Delta]<<"\n"
                         <<  "Theta（频率范围3.5–6.75 Hz）" << eeg.band[(int)Band::Theta]<<"\n"
                         << "LowAlpha（频率范围7.5–9.25 Hz）" << eeg.band[(int)Band::Alpha1]<<"\n"
                         << "HighAlpha（频率范围10–11.75 Hz	）" << eeg.band[(int)Band::Alpha2]<<"\n"
                         << "LowBeta（频率范围13–16.75 Hz	）" << eeg.band[(int)Band::Beta1]<<"\n"
                         << "HighBeta（频率范围18–29.75 Hz）" << eeg.band[(int)Band::Beta2]<<"\n"
                         << "LowGamma（频率范围31–39.75 Hz）" << eeg.band[(int)Band::Gamma1]<<"\n"
                         << "MiddleGamma（频率范围41–49.75 Hz）" << eeg.band[(int)Band::Gamma2];
                count++;
                if(count==513)
                {
                    count=1;
                }
                BigPackageSample s;
                s.signal=eeg.signal;
                s.att=eeg.att;
                s.med=eeg.med;
                s.raw_count_at_update=count;
                s.time=currTimeStr;
                s.band[(int)Band::Delta]=eeg.band[(int)Band::Delta];
                s.band[(int)Band::Theta]=eeg.band[(int)Band::Theta];
                s.band[(int)Band::Alpha1]=eeg.band[(int)Band::Alpha1];
                s.band[(int)Band::Alpha2]=eeg.band[(int)Band::Alpha2];
                s.band[(int)Band::Beta1]=eeg.band[(int)Band::Beta1];
                s.band[(int)Band::Beta2]=eeg.band[(int)Band::Beta2];
                s.band[(int)Band::Gamma1]= eeg.band[(int)Band::Gamma1];
                s.band[(int)Band::Gamma2]=eeg.band[(int)Band::Gamma2];
                //in->buffer().appendBigpackage(s);

            }

        }

        qDebug() << "数据收集完成";
        int count=0;
        QVector<RawSample> win = in->buffer().window_RawSample();
        for (const RawSample &s : qAsConst(win)) {
            qDebug() << s.raw_count<<"|"
                     << "样本 raw(uV)=" << s.raw
                     << " time=" << s.time;
            count++;
        }

        QVector<BigPackageSample> bigWin = in->buffer().window_BigPackageSample();

        for (const BigPackageSample &b : qAsConst(bigWin)) {
            qDebug() << "---- 大包字段更新 ----" << b.time;
            qDebug() << "信号质量(0-200)" << b.signal<<"\n"
                     << "| Attention(0-100)" << b.att<<"\n"
                     << "| Meditation(0-100)" << b.med;

            qDebug() << "EEG Power(dec):"<<"\n"
                         << "Delta（频率范围0.5–2.75 Hz" << b.band[(int)Band::Delta]<<"\n"
                         <<  "Theta（频率范围3.5–6.75 Hz）" << b.band[(int)Band::Theta]<<"\n"
                         << "LowAlpha（频率范围7.5–9.25 Hz）" << b.band[(int)Band::Alpha1]<<"\n"
                         << "HighAlpha（频率范围10–11.75 Hz	）" << b.band[(int)Band::Alpha2]<<"\n"
                         << "LowBeta（频率范围13–16.75 Hz	）" << b.band[(int)Band::Beta1]<<"\n"
                         << "HighBeta（频率范围18–29.75 Hz）" << b.band[(int)Band::Beta2]<<"\n"
                         << "LowGamma（频率范围31–39.75 Hz）" << b.band[(int)Band::Gamma1]<<"\n"
                         << "MiddleGamma（频率范围41–49.75 Hz）" << b.band[(int)Band::Gamma2];
        }
        qDebug()<<"当前有"<<count<<"个样本";
        delete in;


    }

};


*/

