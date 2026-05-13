#include "threadteast.h"
#include "stream_sdk_for_pc.h"
#include <thread>
#include <atomic>
#include <chrono>
#include <QDateTime>
/*std::atomic<bool> g_running{true};
threadteast::threadteast() {}
void collectThreadFunc_RawOne(Stream_sdk_for_pc *sdk)
{
    uint64_t rawCount=0;

    while(g_running)
    {
        if(!sdk->ReadOnePackage())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;

        }

        double v=0.0;
        if(sdk->GetRawValue(v))
        {
            RawSample s;
            s.raw=v;
            QDateTime now = QDateTime::currentDateTime();
            QString currTimeStr = now.toString(Qt::TextDate);
            s.time=currTimeStr;

            s.raw_count =++rawCount;
            if(s.raw_count>513)
            {
                s.raw_count=1;
            }
            sdk->buffer().appendRawvalue(s);
        }
    }
}
void collectTheadFunc_Rawmore(Stream_sdk_for_pc *sdk)
{
    uint64_t rawCount=0;
    while(g_running)
    {
        if(!sdk->ReadMorePackeage())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        double v=0.0;
        if(sdk->GetRawValue(v))
        {
            RawSample s;
            s.raw = v;
           // s.time=QDateTime::currentMSecsSinceEpoch();
            s.raw_count =++rawCount;
            sdk->buffer().appendRawvalue(s);
        }
    }


}
void collectTheadFunc_BigOne(Stream_sdk_for_pc *sdk)
{
    uint64_t count_at_update = 0;
    while(g_running)
    {
        if(!sdk->ReadOnePackage())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        short att=0;
        short med=0;
        short singal=0;
        uint32_t Delta = 0;
        uint32_t Theta =0;
        uint32_t Alpha1=0;
        uint32_t Alpha2=0;
        uint32_t Beta1=0;
        uint32_t Beta2=0;
        uint32_t Gamma1=0;
        uint32_t Gamma2=0;
        bool m_Bigflag=false;

        sdk->GetMed(med);
        sdk->GetDelta(Delta);
        sdk->Gettheta(Theta);
        sdk->GetHeightAlphal(Alpha1);
        sdk->GetLowAlphal(Alpha2);
        sdk->GetLowbeta(Beta1);
        sdk->GetHeightbeta(Beta2);
        sdk->GetLowGamma(Gamma1);
        sdk->GetMiddleGame(Gamma2);
        if(sdk->GetSingal(singal)&&sdk->GetATT(att))
        {
            BigPackageSample s;
            s.att=att;
            s.med=med;
            s.signal=singal;
            s.band[(int)Band::Delta]=Delta;
            s.band[(int)Band::Theta]=Theta;
            s.band[(int)Band::Alpha1]=Alpha1;
            s.band[(int)Band::Alpha2]=Alpha2;
            s.band[(int)Band::Beta1]=Beta1;
            s.band[(int)Band::Beta2]=Beta2;
            s.band[(int)Band::Gamma1]=Gamma1;
            s.band[(int)Band::Gamma2]=Gamma2;
            s.time = QDateTime::currentDateTime().toString(Qt::TextDate);

            if(s.raw_count_at_update>513)
            {
                s.raw_count_at_update=1;
            }
            s.raw_count_at_update =++count_at_update;
           // sdk->buffer().appendBigpackage(s);
        }



    }
}
void collectTheadFunc_BigMore(Stream_sdk_for_pc *sdk)
{
    uint64_t count_at_update = 0;
    while(g_running)
    {
        if(!sdk->ReadMorePackeage())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        short att=0;
        short med=0;
        short singal=0;
        uint32_t Delta = 0;
        uint32_t Theta =0;
        uint32_t Alpha1=0;
        uint32_t Alpha2=0;
        uint32_t Beta1=0;
        uint32_t Beta2=0;
        uint32_t Gamma1=0;
        uint32_t Gamma2=0;
        bool m_Bigflag=false;

        sdk->GetMed(med);
        sdk->GetDelta(Delta);
        sdk->Gettheta(Theta);
        sdk->GetHeightAlphal(Alpha1);
        sdk->GetLowAlphal(Alpha2);
        sdk->GetLowbeta(Beta1);
        sdk->GetHeightbeta(Beta2);
        sdk->GetLowGamma(Gamma1);
        sdk->GetMiddleGame(Gamma2);
        if(sdk->GetSingal(singal)&&sdk->GetATT(att))
        {
            BigPackageSample s;
            s.att=att;
            s.med=med;
            s.signal=singal;
            s.band[(int)Band::Delta]=Delta;
            s.band[(int)Band::Theta]=Theta;
            s.band[(int)Band::Alpha1]=Alpha1;
            s.band[(int)Band::Alpha2]=Alpha2;
            s.band[(int)Band::Beta1]=Beta1;
            s.band[(int)Band::Beta2]=Beta2;
            s.band[(int)Band::Gamma1]=Gamma1;
            s.band[(int)Band::Gamma2]=Gamma2;
            s.time = QDateTime::currentDateTime().toString(Qt::TextDate);
            s.raw_count_at_update =++count_at_update;
            if(s.raw_count_at_update>513)
            {
                s.raw_count_at_update=1;
            }
            sdk->buffer().appendBigpackage(s);
        }



    }
}
void displayThreadFunc_Raw(Stream_sdk_for_pc *sdk)
{
    while(g_running)
    {
        QVector<RawSample> win=sdk->buffer().window_RawSample();
        qDebug()<<"当前缓存里面有"<<win.size()<<"个样本\n";
        if(!win.isEmpty())
        {
            const RawSample &last =win.last();
            qDebug() << last.raw_count<<"|"
                     << "最新样本 raw(uV)=" << last.raw
                     << " time=" << last.time;


        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
void threadteast::run()
{
    Stream_sdk_for_pc sdk;
    char portName[]="COM7";
    if(!sdk.Init(portName,TG_BAUD_115200,TG_STREAM_PACKETS))
    {
        qDebug()<<"Init failed";
        return;

    }
    g_running = true;
    std::thread t_collect(collectThreadFunc_RawOne,&sdk);
    std::thread t_display(displayThreadFunc_Raw,&sdk);
    std::this_thread::sleep_for(std::chrono::seconds(10));     // ★ 关键：跑 10 秒
    g_running = false;
    if (t_collect.joinable()) t_collect.join();
    if (t_display.joinable()) t_display.join();


}*/
