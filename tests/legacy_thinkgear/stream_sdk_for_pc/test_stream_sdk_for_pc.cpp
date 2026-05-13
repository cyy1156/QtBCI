
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
void wait()
{
    std::cout<<"\n";
    std::cout<<"Press the ENTER key...\n"<<std::flush;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();

}
double ADCtoMicroV(short rawADC)
{
    // 官方公式：原始ADC计数 → 微伏 μV
    return rawADC * 0.22;
}
int runtest()
{

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

    dllVersion=TG_GetVersion();
    qDebug()<<"Stream SDK for PC version: "<<dllVersion;

    connectionId=TG_GetNewConnectionId();
    if(connectionId<0)
    {
        std::fprintf(stderr, "ERROR: TG_GetNewConnectionId() returned %d.\n", connectionId);
        std::cout << "Press Enter to exit..." << std::flush;
        wait();
        std::exit(EXIT_FAILURE);
    }

    comPortName="COM7";
    errCode=TG_Connect(
        connectionId,
        comPortName,
        TG_BAUD_57600,
        TG_STREAM_PACKETS
        );
    if(errCode!=0)
    {
        qDebug()<<"串口配置有错";
        return -1;
    }
    secondsToRun=5;
    startTime1=time(nullptr);
    int rawport[513];
    qDebug()<<"读一个包解析一次";
    while(difftime(time(nullptr),startTime1)<secondsToRun)
    {



             packetsRead=TG_ReadPackets(connectionId,1);//每次只解析一个
            if(packetsRead==1)

             {
                 if (TG_GetValueStatus(connectionId, TG_DATA_POOR_SIGNAL) != 0) {
                    short signal = (short)TG_GetValue(connectionId, TG_DATA_POOR_SIGNAL);
                     qDebug()<<"信号质量"<<signal;
                }



                 if(TG_GetValueStatus(connectionId,TG_DATA_RAW)!=0)
                {
                    QDateTime now=QDateTime::currentDateTime();
                    currTimeStr= now.toString(Qt::TextDate);
                    double v=TG_GetValue(connectionId,TG_DATA_RAW);
                    short raw=(short)v;//符号是ADC不是电压值
                    double rawvalue=ADCtoMicroV(raw);
                    //rawport[count++]=(int)raw;
                    count++;
                    qDebug()<<count<<"|"<<currTimeStr<<"|"<<"原始信号(μV)"<<rawvalue;
                 }


                 if(TG_GetValueStatus(connectionId,TG_DATA_ATTENTION)!=0){

                    short ATT = (short)TG_GetValue(connectionId, TG_DATA_ATTENTION);
                     qDebug()<<"专注度Attention(值在0-100)"<<ATT;
                }
                 if (TG_GetValueStatus(connectionId, TG_DATA_MEDITATION) != 0) {
                     short med = (short)TG_GetValue(connectionId, TG_DATA_MEDITATION);
                     qDebug()<<"放松度Meditation值(0到100之间)"<<med;
                 }
                 if(count==513)
                 {
                     count=0;
                 }
                 if (TG_GetValueStatus(connectionId, TG_DATA_DELTA) != 0) {
                     //相对功率（自己与自己比）
                     uint32_t delta      = static_cast<uint32_t>(TG_GetValue(connectionId, TG_DATA_DELTA));
                     uint32_t theta      = static_cast<uint32_t>(TG_GetValue(connectionId, TG_DATA_THETA));
                     uint32_t lowAlpha   = static_cast<uint32_t>(TG_GetValue(connectionId, TG_DATA_ALPHA1));
                     uint32_t highAlpha  = static_cast<uint32_t>(TG_GetValue(connectionId, TG_DATA_ALPHA2));
                     uint32_t lowBeta    = static_cast<uint32_t>(TG_GetValue(connectionId, TG_DATA_BETA1));
                     uint32_t highBeta   = static_cast<uint32_t>(TG_GetValue(connectionId, TG_DATA_BETA2));
                     uint32_t lowGamma   = static_cast<uint32_t>(TG_GetValue(connectionId, TG_DATA_GAMMA1));
                     uint32_t middleGamma= static_cast<uint32_t>(TG_GetValue(connectionId, TG_DATA_GAMMA2));
                     qDebug() << "EEG Power(dec):"
                              << "Delta（频率范围0.5–2.75 Hz）" << delta
                              << "Theta（频率范围3.5–6.75 Hz	）" << theta
                              << "LowAlpha（频率范围7.5–9.25 Hz）" << lowAlpha
                              << "HighAlpha（频率范围10–11.75 Hz	）" << highAlpha
                              << "LowBeta（频率范围13–16.75 Hz	）" << lowBeta
                              << "HighBeta（频率范围18–29.75 Hz）" << highBeta
                              << "LowGamma（频率范围31–39.75 Hz）" << lowGamma
                              << "MiddleGamma（频率范围41–49.75 Hz）" << middleGamma;
                 }
            }



    }

    int atuoraw[513];
    startTime2=time(nullptr);
    qDebug()<<"尽可能的读包";
    while(difftime(time(nullptr),startTime2)<secondsToRun)
    {

        packetsRead=TG_ReadPackets(connectionId,-1);//尽可能的读

        if(packetsRead>0)

        {
            if(count<512){
            if(TG_GetValueStatus(connectionId,TG_DATA_RAW)!=0)
            {
                QDateTime now=QDateTime::currentDateTime();
                currTimeStr= now.toString(Qt::TextDate);
                double v=TG_GetValue(connectionId,TG_DATA_RAW);
                short raw=(short)v;
                //rawport[count++]=(int)raw;
                count++;
                qDebug()<<count<<"|"<<currTimeStr<<"|"<<"原始信号"<<raw;
            }

            }

            if(count=512){
            if (TG_GetValueStatus(connectionId, TG_DATA_POOR_SIGNAL) != 0) {
                short signal = (short)TG_GetValue(connectionId, TG_DATA_POOR_SIGNAL);
                qDebug()<<"信号质量"<<signal;
            }

            if(TG_GetValueStatus(connectionId,TG_DATA_ATTENTION)!=0){

                short ATT = (short)TG_GetValue(connectionId, TG_DATA_ATTENTION);
                qDebug()<<"专注度Attention(值在0-100)"<<ATT;
            }
            if (TG_GetValueStatus(connectionId, TG_DATA_MEDITATION) != 0) {
                short med = (short)TG_GetValue(connectionId, TG_DATA_MEDITATION);
                qDebug()<<"放松度Meditation值(0到100之间)"<<med;
            }

            if (TG_GetValueStatus(connectionId, TG_DATA_DELTA) != 0) {
                double delta = (double)TG_GetValue(connectionId, TG_DATA_DELTA);
                qDebug()<<"delta的值"<<delta;
            }
            if (TG_GetValueStatus(connectionId, TG_DATA_THETA) != 0) {
                double theta = (double)TG_GetValue(connectionId, TG_DATA_THETA);
                qDebug()<<"theta的值"<<theta;
            }
            if (TG_GetValueStatus(connectionId, TG_DATA_ALPHA1) != 0) {
                double alpha1 = (double)TG_GetValue(connectionId, TG_DATA_ALPHA1);
                qDebug()<<"low alphal的值"<<alpha1;
            }
            if (TG_GetValueStatus(connectionId, TG_DATA_ALPHA2) != 0) {
                double alpha2 = (double)TG_GetValue(connectionId, TG_DATA_ALPHA2);
                qDebug()<<"height alphal的值"<<alpha2;
            }
            if (TG_GetValueStatus(connectionId, TG_DATA_BETA1) != 0) {
                double beta1 = (double)TG_GetValue(connectionId, TG_DATA_BETA1);
                qDebug()<<"low betal的值"<<beta1;
            }
            if (TG_GetValueStatus(connectionId, TG_DATA_BETA2) != 0) {
                double beta2 = (double)TG_GetValue(connectionId, TG_DATA_BETA2);
                qDebug()<<"height betal的值"<<beta2;
            }
            if (TG_GetValueStatus(connectionId, TG_DATA_GAMMA1) != 0) {
                double GAMMA1 = (double)TG_GetValue(connectionId, TG_DATA_GAMMA1);
                qDebug()<<"low gamma的值"<<GAMMA1;
            }
            if (TG_GetValueStatus(connectionId, TG_DATA_GAMMA2) != 0) {
                double GAMMA2 = (double)TG_GetValue(connectionId, TG_DATA_GAMMA2);
                qDebug()<<"height gamma的值"<<GAMMA2;
            }
                count=0;

        }
    }

    }




    qDebug()<<"数据收集完成";
    TG_Disconnect(connectionId);
    TG_FreeConnection(connectionId);

    return 0;
}
