/****************************************************************************
** Meta object code from reading C++ file 'acquisitionengine.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../core/acquisitionengine.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'acquisitionengine.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN17AcquisitionEngineE_t {};
} // unnamed namespace

template <> constexpr inline auto AcquisitionEngine::qt_create_metaobjectdata<qt_meta_tag_ZN17AcquisitionEngineE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "AcquisitionEngine",
        "rawPacketReady",
        "",
        "RawPacket",
        "pkt",
        "warningMessage",
        "msg",
        "statusMessage",
        "runningChanged",
        "running",
        "streamGapDetected",
        "missedSamples",
        "previousSeq",
        "currentSeq",
        "eventWallMs",
        "linkDiagnostic",
        "category",
        "message",
        "start",
        "portName",
        "baudRate",
        "startWithConfig",
        "SerialPortConfig",
        "cfg",
        "stop",
        "onAssemblerFrame",
        "frame",
        "onRxBufferOverflowed",
        "previousSize",
        "onChecksumFailure",
        "totalSoFar",
        "onLengthResync",
        "onRawInt16",
        "raw",
        "onSignalQuality",
        "v",
        "onUvReady",
        "uv"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'rawPacketReady'
        QtMocHelpers::SignalData<void(const RawPacket &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'warningMessage'
        QtMocHelpers::SignalData<void(const QString &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 },
        }}),
        // Signal 'statusMessage'
        QtMocHelpers::SignalData<void(const QString &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 },
        }}),
        // Signal 'runningChanged'
        QtMocHelpers::SignalData<void(bool)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 9 },
        }}),
        // Signal 'streamGapDetected'
        QtMocHelpers::SignalData<void(quint64, quint64, quint64, qint64)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::ULongLong, 11 }, { QMetaType::ULongLong, 12 }, { QMetaType::ULongLong, 13 }, { QMetaType::LongLong, 14 },
        }}),
        // Signal 'linkDiagnostic'
        QtMocHelpers::SignalData<void(const QString &, const QString &, qint64)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 16 }, { QMetaType::QString, 17 }, { QMetaType::LongLong, 14 },
        }}),
        // Slot 'start'
        QtMocHelpers::SlotData<void(const QString &, qint32)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 19 }, { QMetaType::Int, 20 },
        }}),
        // Slot 'startWithConfig'
        QtMocHelpers::SlotData<void(const SerialPortConfig &)>(21, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 22, 23 },
        }}),
        // Slot 'stop'
        QtMocHelpers::SlotData<void()>(24, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onAssemblerFrame'
        QtMocHelpers::SlotData<void(const QByteArray &)>(25, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QByteArray, 26 },
        }}),
        // Slot 'onRxBufferOverflowed'
        QtMocHelpers::SlotData<void(int)>(27, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 28 },
        }}),
        // Slot 'onChecksumFailure'
        QtMocHelpers::SlotData<void(quint64)>(29, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::ULongLong, 30 },
        }}),
        // Slot 'onLengthResync'
        QtMocHelpers::SlotData<void(quint64)>(31, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::ULongLong, 30 },
        }}),
        // Slot 'onRawInt16'
        QtMocHelpers::SlotData<void(qint16)>(32, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Short, 33 },
        }}),
        // Slot 'onSignalQuality'
        QtMocHelpers::SlotData<void(quint8)>(34, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::UChar, 35 },
        }}),
        // Slot 'onUvReady'
        QtMocHelpers::SlotData<void(double)>(36, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 37 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AcquisitionEngine, qt_meta_tag_ZN17AcquisitionEngineE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject AcquisitionEngine::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17AcquisitionEngineE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17AcquisitionEngineE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN17AcquisitionEngineE_t>.metaTypes,
    nullptr
} };

void AcquisitionEngine::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AcquisitionEngine *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->rawPacketReady((*reinterpret_cast<std::add_pointer_t<RawPacket>>(_a[1]))); break;
        case 1: _t->warningMessage((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->statusMessage((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->runningChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 4: _t->streamGapDetected((*reinterpret_cast<std::add_pointer_t<quint64>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<quint64>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<quint64>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<qint64>>(_a[4]))); break;
        case 5: _t->linkDiagnostic((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<qint64>>(_a[3]))); break;
        case 6: _t->start((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<qint32>>(_a[2]))); break;
        case 7: _t->startWithConfig((*reinterpret_cast<std::add_pointer_t<SerialPortConfig>>(_a[1]))); break;
        case 8: _t->stop(); break;
        case 9: _t->onAssemblerFrame((*reinterpret_cast<std::add_pointer_t<QByteArray>>(_a[1]))); break;
        case 10: _t->onRxBufferOverflowed((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 11: _t->onChecksumFailure((*reinterpret_cast<std::add_pointer_t<quint64>>(_a[1]))); break;
        case 12: _t->onLengthResync((*reinterpret_cast<std::add_pointer_t<quint64>>(_a[1]))); break;
        case 13: _t->onRawInt16((*reinterpret_cast<std::add_pointer_t<qint16>>(_a[1]))); break;
        case 14: _t->onSignalQuality((*reinterpret_cast<std::add_pointer_t<quint8>>(_a[1]))); break;
        case 15: _t->onUvReady((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< RawPacket >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (AcquisitionEngine::*)(const RawPacket & )>(_a, &AcquisitionEngine::rawPacketReady, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (AcquisitionEngine::*)(const QString & )>(_a, &AcquisitionEngine::warningMessage, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (AcquisitionEngine::*)(const QString & )>(_a, &AcquisitionEngine::statusMessage, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (AcquisitionEngine::*)(bool )>(_a, &AcquisitionEngine::runningChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (AcquisitionEngine::*)(quint64 , quint64 , quint64 , qint64 )>(_a, &AcquisitionEngine::streamGapDetected, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (AcquisitionEngine::*)(const QString & , const QString & , qint64 )>(_a, &AcquisitionEngine::linkDiagnostic, 5))
            return;
    }
}

const QMetaObject *AcquisitionEngine::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AcquisitionEngine::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17AcquisitionEngineE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int AcquisitionEngine::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    }
    return _id;
}

// SIGNAL 0
void AcquisitionEngine::rawPacketReady(const RawPacket & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void AcquisitionEngine::warningMessage(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void AcquisitionEngine::statusMessage(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void AcquisitionEngine::runningChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void AcquisitionEngine::streamGapDetected(quint64 _t1, quint64 _t2, quint64 _t3, qint64 _t4)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2, _t3, _t4);
}

// SIGNAL 5
void AcquisitionEngine::linkDiagnostic(const QString & _t1, const QString & _t2, qint64 _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1, _t2, _t3);
}
QT_WARNING_POP
