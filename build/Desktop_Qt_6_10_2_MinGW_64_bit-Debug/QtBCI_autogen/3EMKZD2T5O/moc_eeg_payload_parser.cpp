/****************************************************************************
** Meta object code from reading C++ file 'eeg_payload_parser.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../device/serial_eeg/eeg_payload_parser.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'eeg_payload_parser.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN22SerialEegPayloadParserE_t {};
} // unnamed namespace

template <> constexpr inline auto SerialEegPayloadParser::qt_create_metaobjectdata<qt_meta_tag_ZN22SerialEegPayloadParserE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "SerialEegPayloadParser",
        "rawReceived",
        "",
        "value",
        "signalQualityReceived",
        "attentionReceived",
        "meditationReceived",
        "blinkReceived",
        "eegPowerReceived",
        "SerialEegBandPower",
        "power",
        "parseWarning",
        "message",
        "parseFrame",
        "frame"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'rawReceived'
        QtMocHelpers::SignalData<void(qint16)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Short, 3 },
        }}),
        // Signal 'signalQualityReceived'
        QtMocHelpers::SignalData<void(quint8)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::UChar, 3 },
        }}),
        // Signal 'attentionReceived'
        QtMocHelpers::SignalData<void(quint8)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::UChar, 3 },
        }}),
        // Signal 'meditationReceived'
        QtMocHelpers::SignalData<void(quint8)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::UChar, 3 },
        }}),
        // Signal 'blinkReceived'
        QtMocHelpers::SignalData<void(quint8)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::UChar, 3 },
        }}),
        // Signal 'eegPowerReceived'
        QtMocHelpers::SignalData<void(const SerialEegBandPower &)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 9, 10 },
        }}),
        // Signal 'parseWarning'
        QtMocHelpers::SignalData<void(const QString &)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 12 },
        }}),
        // Slot 'parseFrame'
        QtMocHelpers::SlotData<void(const QByteArray &)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QByteArray, 14 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<SerialEegPayloadParser, qt_meta_tag_ZN22SerialEegPayloadParserE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject SerialEegPayloadParser::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN22SerialEegPayloadParserE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN22SerialEegPayloadParserE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN22SerialEegPayloadParserE_t>.metaTypes,
    nullptr
} };

void SerialEegPayloadParser::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SerialEegPayloadParser *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->rawReceived((*reinterpret_cast<std::add_pointer_t<qint16>>(_a[1]))); break;
        case 1: _t->signalQualityReceived((*reinterpret_cast<std::add_pointer_t<quint8>>(_a[1]))); break;
        case 2: _t->attentionReceived((*reinterpret_cast<std::add_pointer_t<quint8>>(_a[1]))); break;
        case 3: _t->meditationReceived((*reinterpret_cast<std::add_pointer_t<quint8>>(_a[1]))); break;
        case 4: _t->blinkReceived((*reinterpret_cast<std::add_pointer_t<quint8>>(_a[1]))); break;
        case 5: _t->eegPowerReceived((*reinterpret_cast<std::add_pointer_t<SerialEegBandPower>>(_a[1]))); break;
        case 6: _t->parseWarning((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->parseFrame((*reinterpret_cast<std::add_pointer_t<QByteArray>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (SerialEegPayloadParser::*)(qint16 )>(_a, &SerialEegPayloadParser::rawReceived, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (SerialEegPayloadParser::*)(quint8 )>(_a, &SerialEegPayloadParser::signalQualityReceived, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (SerialEegPayloadParser::*)(quint8 )>(_a, &SerialEegPayloadParser::attentionReceived, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (SerialEegPayloadParser::*)(quint8 )>(_a, &SerialEegPayloadParser::meditationReceived, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (SerialEegPayloadParser::*)(quint8 )>(_a, &SerialEegPayloadParser::blinkReceived, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (SerialEegPayloadParser::*)(const SerialEegBandPower & )>(_a, &SerialEegPayloadParser::eegPowerReceived, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (SerialEegPayloadParser::*)(const QString & )>(_a, &SerialEegPayloadParser::parseWarning, 6))
            return;
    }
}

const QMetaObject *SerialEegPayloadParser::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SerialEegPayloadParser::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN22SerialEegPayloadParserE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int SerialEegPayloadParser::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void SerialEegPayloadParser::rawReceived(qint16 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void SerialEegPayloadParser::signalQualityReceived(quint8 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void SerialEegPayloadParser::attentionReceived(quint8 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void SerialEegPayloadParser::meditationReceived(quint8 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void SerialEegPayloadParser::blinkReceived(quint8 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void SerialEegPayloadParser::eegPowerReceived(const SerialEegBandPower & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void SerialEegPayloadParser::parseWarning(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}
QT_WARNING_POP
