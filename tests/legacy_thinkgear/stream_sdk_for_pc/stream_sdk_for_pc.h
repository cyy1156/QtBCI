#ifndef STREAM_SDK_FOR_PC_H
#define STREAM_SDK_FOR_PC_H

#include <QObject>
#include "../thinkgear.h"
#include <QApplication>
#include <QDebug>
#include <iostream>
#include <limits>
#include <cstdlib>
#include <ctime>
#include <QDateTime>
#include <core/pictureandalgbuffer.h>

#include <cstdint> // uint32_t
class Stream_sdk_for_pc : public QObject
{
    Q_OBJECT
public:
    explicit Stream_sdk_for_pc(QObject *parent = nullptr);
    ~Stream_sdk_for_pc();
     PictureAndALgBuffer &buffer() { return data_pool; }
public:
    bool Init(char* comPortName, int serialBaudrate, int serialDataFormat);
    bool ReadOnePackage();//每次读一个包
    bool ReadMorePackeage();//尽可能读完当前包
    bool GetRawValue(double& v);
    bool GetSingal(short& v);
    bool GetATT(short& v);
    bool GetMed(short& v);
    bool GetDelta(uint32_t& v);
    bool Gettheta(uint32_t& v);
    bool GetLowAlphal(uint32_t& v);
    bool GetHeightAlphal(uint32_t& v);
    bool GetLowbeta(uint32_t& v);
    bool GetHeightbeta(uint32_t& v);
    bool GetLowGamma(uint32_t& v);
    bool GetMiddleGame(uint32_t& v);



private:
    PictureAndALgBuffer data_pool;
    BigPackageSample bigpackagesample;
    RawSample rawsample;

    int dllVersion =0;
    int connectionId=-3;
    int packetsRead =0;
    int errCode=0;
    int count=0;

signals:
};

#endif // STREAM_SDK_FOR_PC_H
