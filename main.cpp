//#include "mainwindow.h"
// 旧测试入口（与三线程主链无关），注释保留
// #include"stream_sdk_for_pc/threadteast.h"
// #include"stream_sdk_for_pc/changetest.h"
// #include<stream_sdk_for_pc/origintest.h>
// #include"thinkgear/thinkgearlinktester.h"
#include "mainwindow.h"
#include<QApplication>
int main(int argc, char *argv[])

{

    QApplication app(argc, argv);
    MainWindow w;


    /*auto *tester=new ThinkGearLinkTester(&w);
    w.attachLinkTester(tester);*/
    w.initThreads();
    w.show();
    return app.exec();



   /* auto *m_linkTester = new ThinkGearLinkTester(&app);

    QObject::connect(m_linkTester, &ThinkGearLinkTester::secondReport,
                     m_linkTester,
                     [](quint64 secIndex, int rawPerSec, int framePerSec, int warnPerSec, bool pass) {
                         qDebug() << "[REPORT]"
                                  << "sec" << secIndex
                                  << "raw/s" << rawPerSec
                                  << "frame/s" << framePerSec
                                  << "warn/s" << warnPerSec
                                  << "pass" << pass;
                     });

    QObject::connect(m_linkTester, &ThinkGearLinkTester::testMessage,
                     m_linkTester,
                     [](const QString &m) { qDebug() << m; });

    // 注意：这里用 start 还是 strat 取决于你头文件最终名字
    m_linkTester->setTargetRawPerSec(256);
    m_linkTester->setTolerance(25);
    m_linkTester->start("COM7", 57600);*/

    //return app.exec();
    /*origintest* test=new origintest();
    if(test->m_flag)
    {
        test->run();

    }*/
    /*test::changtest* changetest;
    changetest=new test::changtest();
    if(changetest->m_flag)
    {
        changetest->SampleAndDisplay();
    }*/
   /*threadteast *test;
   test->run();*/

    //return 0;
}

