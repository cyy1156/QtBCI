/****************************************************************************
** Meta object code from reading C++ file 'rawtouvprocessor.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../thinkgear/rawtouvprocessor.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rawtouvprocessor.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN18RawtOutUvProcessorE_t {};
} // unnamed namespace

template <> constexpr inline auto RawtOutUvProcessor::qt_create_metaobjectdata<qt_meta_tag_ZN18RawtOutUvProcessorE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "RawtOutUvProcessor",
        "uvSampleReady",
        "",
        "UvSample",
        "sample",
        "uvValueReady",
        "uv",
        "onRaw",
        "rawValue",
        "onRawWithMeta"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'uvSampleReady'
        QtMocHelpers::SignalData<void(const UvSample &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'uvValueReady'
        QtMocHelpers::SignalData<void(double)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 6 },
        }}),
        // Slot 'onRaw'
        QtMocHelpers::SlotData<void(qint16)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Short, 8 },
        }}),
        // Slot 'onRawWithMeta'
        QtMocHelpers::SlotData<void(qint16)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Short, 8 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<RawtOutUvProcessor, qt_meta_tag_ZN18RawtOutUvProcessorE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject RawtOutUvProcessor::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18RawtOutUvProcessorE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18RawtOutUvProcessorE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN18RawtOutUvProcessorE_t>.metaTypes,
    nullptr
} };

void RawtOutUvProcessor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<RawtOutUvProcessor *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->uvSampleReady((*reinterpret_cast<std::add_pointer_t<UvSample>>(_a[1]))); break;
        case 1: _t->uvValueReady((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 2: _t->onRaw((*reinterpret_cast<std::add_pointer_t<qint16>>(_a[1]))); break;
        case 3: _t->onRawWithMeta((*reinterpret_cast<std::add_pointer_t<qint16>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (RawtOutUvProcessor::*)(const UvSample & )>(_a, &RawtOutUvProcessor::uvSampleReady, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (RawtOutUvProcessor::*)(double )>(_a, &RawtOutUvProcessor::uvValueReady, 1))
            return;
    }
}

const QMetaObject *RawtOutUvProcessor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *RawtOutUvProcessor::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18RawtOutUvProcessorE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int RawtOutUvProcessor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void RawtOutUvProcessor::uvSampleReady(const UvSample & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void RawtOutUvProcessor::uvValueReady(double _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}
QT_WARNING_POP
