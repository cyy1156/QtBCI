#pragma once

#include <QCoreApplication>
#include <QSettings>
#include <QString>

/// 可移植配置：与 QtBCI.exe 同目录的 QtBCI.ini（IniFormat）。
/// 须在 QApplication 构造之后再使用（以便 applicationDirPath() 正确）。
inline QString appIniFilePath()
{
    return QCoreApplication::applicationDirPath() + QStringLiteral("/QtBCI.ini");
}

inline QSettings userSettings()
{
    return QSettings(appIniFilePath(), QSettings::IniFormat);
}
