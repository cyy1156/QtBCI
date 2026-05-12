/****************************************************************************
** Meta object code from reading C++ file 'csvlogworker.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../core/csvlogworker.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'csvlogworker.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN12CsvLogWorkerE_t {};
} // unnamed namespace

template <> constexpr inline auto CsvLogWorker::qt_create_metaobjectdata<qt_meta_tag_ZN12CsvLogWorkerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "CsvLogWorker",
        "workerError",
        "",
        "msg",
        "workerInfo",
        "drained",
        "items",
        "queueSizeAfter",
        "droppedCount",
        "start",
        "stop",
        "onSpectrumResult",
        "SpectrumResult",
        "sp",
        "onFftResult",
        "FftResult",
        "fr",
        "noteLossEventWallMs",
        "wallMs",
        "onAcquisitionStreamGap",
        "missed",
        "prevSeq",
        "curSeq",
        "eventWallMs",
        "onAcquisitionLinkDiag",
        "category",
        "message",
        "onFlushTick"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'workerError'
        QtMocHelpers::SignalData<void(const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'workerInfo'
        QtMocHelpers::SignalData<void(const QString &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'drained'
        QtMocHelpers::SignalData<void(int, int, int)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 6 }, { QMetaType::Int, 7 }, { QMetaType::Int, 8 },
        }}),
        // Slot 'start'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'stop'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onSpectrumResult'
        QtMocHelpers::SlotData<void(const SpectrumResult &)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 12, 13 },
        }}),
        // Slot 'onFftResult'
        QtMocHelpers::SlotData<void(const FftResult &)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 15, 16 },
        }}),
        // Slot 'noteLossEventWallMs'
        QtMocHelpers::SlotData<void(qint64)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::LongLong, 18 },
        }}),
        // Slot 'onAcquisitionStreamGap'
        QtMocHelpers::SlotData<void(quint64, quint64, quint64, qint64)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::ULongLong, 20 }, { QMetaType::ULongLong, 21 }, { QMetaType::ULongLong, 22 }, { QMetaType::LongLong, 23 },
        }}),
        // Slot 'onAcquisitionLinkDiag'
        QtMocHelpers::SlotData<void(const QString &, const QString &, qint64)>(24, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 25 }, { QMetaType::QString, 26 }, { QMetaType::LongLong, 23 },
        }}),
        // Slot 'onFlushTick'
        QtMocHelpers::SlotData<void()>(27, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<CsvLogWorker, qt_meta_tag_ZN12CsvLogWorkerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject CsvLogWorker::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12CsvLogWorkerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12CsvLogWorkerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN12CsvLogWorkerE_t>.metaTypes,
    nullptr
} };

void CsvLogWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<CsvLogWorker *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->workerError((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->workerInfo((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->drained((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[3]))); break;
        case 3: _t->start(); break;
        case 4: _t->stop(); break;
        case 5: _t->onSpectrumResult((*reinterpret_cast<std::add_pointer_t<SpectrumResult>>(_a[1]))); break;
        case 6: _t->onFftResult((*reinterpret_cast<std::add_pointer_t<FftResult>>(_a[1]))); break;
        case 7: _t->noteLossEventWallMs((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1]))); break;
        case 8: _t->onAcquisitionStreamGap((*reinterpret_cast<std::add_pointer_t<quint64>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<quint64>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<quint64>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<qint64>>(_a[4]))); break;
        case 9: _t->onAcquisitionLinkDiag((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<qint64>>(_a[3]))); break;
        case 10: _t->onFlushTick(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (CsvLogWorker::*)(const QString & )>(_a, &CsvLogWorker::workerError, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (CsvLogWorker::*)(const QString & )>(_a, &CsvLogWorker::workerInfo, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (CsvLogWorker::*)(int , int , int )>(_a, &CsvLogWorker::drained, 2))
            return;
    }
}

const QMetaObject *CsvLogWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CsvLogWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12CsvLogWorkerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int CsvLogWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void CsvLogWorker::workerError(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void CsvLogWorker::workerInfo(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void CsvLogWorker::drained(int _t1, int _t2, int _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2, _t3);
}
QT_WARNING_POP
