#include "stream_sdk_for_pc.h"
#include <limits>
#include <cmath>
Stream_sdk_for_pc::Stream_sdk_for_pc(QObject *parent)
    : QObject{parent}
{}
static void wait()
{
    std::cout<<"\n";
    std::cout<<"Press the ENTER key...\n"<<std::flush;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();

}
static double ADCtoMicroV(short rawADC)
{
    // 官方公式：原始ADC计数 → 微伏 μV
    return rawADC * 0.22;
}
bool Stream_sdk_for_pc::Init(char* comPortName, int serialBaudrate, int serialDataFormat){
    dllVersion=TG_GetVersion();
    qDebug()<<"Stream SDK for PC version: "<<dllVersion;
    connectionId=TG_GetNewConnectionId();

    if(connectionId<0)
    {
        std::fprintf(stderr, "ERROR: TG_GetNewConnectionId() returned %d.\n", connectionId);
        if(connectionId==-1)
        {
            qDebug()<<"连接数达到上限";
        }
        if(connectionId==-2)
        {
            qDebug()<<"内存不足";
        }
        return false;
    }


    errCode=TG_Connect(
        connectionId,
        comPortName,
        serialBaudrate,
        serialDataFormat
        );
    if(errCode==0)
    {
        qDebug()<<"成功初始化";
        return true;
    }
    if(errCode==-1){
        qDebug()<<"connectionId无效";
    }
    if(errCode==-2)
    {
        qDebug()<<"打不开串口";
    }
    if(errCode==-3)
    {
        qDebug()<<"波特率非法";
    }
    if(errCode==-4)
    {
        qDebug()<<"数据格式非法";
    }
    return false;
}
bool Stream_sdk_for_pc::ReadOnePackage()
{
    errCode=TG_ReadPackets(connectionId,1);

    if(errCode==-1)
    {
        qDebug()<<"无效句柄";
        return false;

    }
    if(errCode==-2)
    {
        //qDebug()<<"没有可读字节";
        return false;
    }
    if(errCode==-3)
    {
        qDebug()<<"I/O发生错误";
        return false;
    }
    return true;

}
bool Stream_sdk_for_pc::ReadMorePackeage()
{
    errCode=TG_ReadPackets(connectionId,-1);

    if(errCode==-1)
    {
        qDebug()<<"无效句柄";
        return false;
    }
    if(errCode==-2)
    {
        //qDebug()<<"没有可读字节";
       return false;

    }
    if(errCode==-3)
    {
        qDebug()<<"I/O发生错误";
        return false;
    }
    return true;
}
bool Stream_sdk_for_pc::GetRawValue(double& v)
{

    if(TG_GetValueStatus(connectionId,TG_DATA_RAW)!=0)
    {
        double adc = TG_GetValue(connectionId,TG_DATA_RAW);
        short raw=(short)adc;//符号是ADC不是电压值
        v = ADCtoMicroV(raw);
        return true;


    }

    return false;
}
bool Stream_sdk_for_pc::GetSingal(short& v)
{
    if (TG_GetValueStatus(connectionId, TG_DATA_POOR_SIGNAL) != 0) {
        v = (short)TG_GetValue(connectionId, TG_DATA_POOR_SIGNAL);
        //qDebug()<<"信号质量(0-200|0为良好，200为差)"<<signal;
        return true;
    }
    return false;
}
bool Stream_sdk_for_pc::GetATT(short& v)
{
    if(TG_GetValueStatus(connectionId,TG_DATA_ATTENTION)!=0){

        v = (short)TG_GetValue(connectionId, TG_DATA_ATTENTION);
        //qDebug()<<"专注度Attention(值在0-100)"<<ATT;
        return true;

    }
    return false;
}
bool Stream_sdk_for_pc::GetMed(short& v)
{


        if (TG_GetValueStatus(connectionId, TG_DATA_MEDITATION) != 0) {
            v = (short)TG_GetValue(connectionId, TG_DATA_MEDITATION);
            //qDebug()<<"放松度Meditation值(0到100之间)"<<med;
            return true;
        }


    return false;
}
bool Stream_sdk_for_pc::GetDelta(uint32_t& v)
{

    if (TG_GetValueStatus(connectionId, TG_DATA_DELTA) != 0) {
        v = static_cast<uint32_t>(TG_GetValue(connectionId, TG_DATA_DELTA));
        //qDebug()<<"delta的值"<<delta;
        return true;
    }


    return false;
}
bool Stream_sdk_for_pc::Gettheta(uint32_t& v)
{

    if (TG_GetValueStatus(connectionId, TG_DATA_THETA) != 0) {
        v = static_cast<uint32_t>(TG_GetValue(connectionId, TG_DATA_THETA));
        //qDebug()<<"theta的值"<<theta;
        return true;
    }


    return false;
}
bool Stream_sdk_for_pc::GetLowAlphal(uint32_t& v)
{

    if (TG_GetValueStatus(connectionId, TG_DATA_ALPHA1) != 0) {
        v = static_cast<uint32_t>(TG_GetValue(connectionId, TG_DATA_ALPHA1));
        //qDebug()<<"low alphal的值"<<alpha1;
        return true;
    }


    return false;
}
bool Stream_sdk_for_pc::GetHeightAlphal(uint32_t& v)
{

    if (TG_GetValueStatus(connectionId, TG_DATA_ALPHA2) != 0) {
        v = static_cast<uint32_t>(TG_GetValue(connectionId, TG_DATA_ALPHA2));
        //qDebug()<<"height alphal的值"<<alpha1;
        return true;
    }


    return false;
}
bool Stream_sdk_for_pc::GetLowbeta(uint32_t& v)
{

    if (TG_GetValueStatus(connectionId, TG_DATA_BETA1) != 0) {
        v = static_cast<uint32_t>(TG_GetValue(connectionId, TG_DATA_BETA1));
        //qDebug()<<"low betal的值"<<beta1;
        return true;
    }


    return false;
}
bool Stream_sdk_for_pc::GetHeightbeta(uint32_t& v)
{

    if (TG_GetValueStatus(connectionId, TG_DATA_BETA2) != 0) {
        v = static_cast<uint32_t>(TG_GetValue(connectionId, TG_DATA_BETA2));
        //qDebug()<<"height betal的值"<<beta1;
        return true;
    }


    return false;
}
bool Stream_sdk_for_pc::GetLowGamma(uint32_t& v)
{

    if (TG_GetValueStatus(connectionId, TG_DATA_GAMMA1) != 0) {
        v = static_cast<uint32_t>(TG_GetValue(connectionId, TG_DATA_GAMMA1));
        //qDebug()<<"low gamma的值"<<GAMMA1;
        return true;
    }


    return false;
}
bool Stream_sdk_for_pc::GetMiddleGame(uint32_t& v)
{

    if (TG_GetValueStatus(connectionId, TG_DATA_GAMMA2) != 0) {
        v = static_cast<uint32_t>(TG_GetValue(connectionId, TG_DATA_GAMMA2));
        //qDebug()<<middle gamma的值"<<GAMMA1;
        return true;
    }

    return false;
}
Stream_sdk_for_pc::~Stream_sdk_for_pc()
{
    if(connectionId>=0)
    {
        TG_Disconnect(connectionId);
        TG_FreeConnection(connectionId);
        connectionId=-3;
    }

}

