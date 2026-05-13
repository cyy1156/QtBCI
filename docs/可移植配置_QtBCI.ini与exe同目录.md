# 可移植配置：`QtBCI.ini` 与可执行文件同目录

本文说明为何将程序配置固定为 **`QtBCI.ini`（与 `QtBCI.exe` 同目录）**，以及在本工程中的约定、路径行为与注意事项。

---

## 1. 设计目标

| 目标 | 说明 |
|------|------|
| **干净** | 不往 Windows 注册表、不往 `%APPDATA%` 写散落配置；用户目录里不出现「找不到的」应用数据。 |
| **方便** | 拷贝整个文件夹（`QtBCI.exe` + `QtBCI.ini` + 依赖 DLL）即可换机使用；备份、还原、版本对比只需看一个文本文件。 |
| **协作友好** | 协作者或测试人员一眼知道配置在哪；排障时可直接打开 `QtBCI.ini` 核对串口、网络等键值。 |

与「默认 `QSettings(组织名, 应用名)`」相比：在 Windows 上通常会落到**注册表**或用户配置目录，对**绿色版 / U 盘携带 / 仅分发一个文件夹**不够直观。

---

## 2. 文件位置约定

- **文件名**：`QtBCI.ini`（固定，便于文档与脚本引用）。
- **目录**：与当前运行的 **`QtBCI.exe` 所在目录**相同，即  
  `QCoreApplication::applicationDirPath() + "/QtBCI.ini"`。
- **格式**：INI（`QSettings::IniFormat`），人类可读、可手动编辑（注意保存为 UTF-8 时中文键值更稳妥）。

**注意**：若将 `exe` 安装在 **`C:\Program Files\...`** 等受保护目录，普通用户可能**无写权限**，首次保存会失败。便携分发时建议将整包解压到用户有写权限的目录（如桌面、文档、自定义 `D:\Apps\QtBCI\`）。

---

## 3. 本工程中的配置范围

以下逻辑通过**统一的 `QSettings` 构造方式**指向同一 `QtBCI.ini`（实现见 `core/app_settings.h`）：

| 分组（INI 节/路径前缀） | 内容 | 代码位置（概念） |
|-------------------------|------|------------------|
| `serial/*` | 串口名、波特率、数据位、校验等 | `MainWindow::loadSerialSettings` / `saveSerialSettings` |
| `network/*` | UDP 开关、主机、三端口、各流发送开关等 | `NetworkSettingsStore::load` / `save` |

键名与原先使用 `QSettings("QtBCI","QtBCI")` 时保持一致，仅**存储位置**从注册表/用户目录改为 exe 旁 INI。

---

## 4. 程序启动顺序要求

必须在**第一次**使用 `QSettings` 读写配置之前，完成 `QCoreApplication` 的路径初始化：

1. 构造 `QApplication`（或至少保证 `QCoreApplication::applicationDirPath()` 已可用）。
2. 之后所有配置读写均通过 **`userSettings()`**（定义于 `core/app_settings.h`）打开同一 INI 路径。

若在 `main()` 里过早静态初始化里读配置，可能导致路径异常，应避免。

---

## 5. 从旧位置迁移（可选）

若用户机器上仍存在旧版 **注册表 / 用户目录** 中的 `QtBCI` 配置，可在首次启动时：

1. 若 `QtBCI.ini` **不存在**，且检测到旧存储中有 `serial/*` 或 `network/*`，则读入并写入新 INI 一次；
2. 或提供一次性「导入」说明，让用户手动复制键值。

当前版本以**新装绿色包**为主时可不做自动迁移，仅在发布说明中写一句「旧版设置需重新选一次串口」即可。

---

## 6. 与部署、Git 的关系

| 场景 | 建议 |
|------|------|
| **Git 仓库** | 不要将个人本机的 `QtBCI.ini` 提交进版本库；在 `.gitignore` 中忽略 `QtBCI.ini`（若仓库根即 exe 目录）或发布包内的该文件。 |
| **发布 zip** | 可提供 `QtBCI.ini.example`（示例键值），用户复制为 `QtBCI.ini` 后按需修改。 |
| **Qt 部署** | `windeployqt` 后把 `QtBCI.ini` 与 `exe` 放在同一输出目录，与 `DataHandler.dll` 等并列，整夹分发。 |

---

## 7. 小结

- 配置文件固定为 **`exe` 同目录下的 `QtBCI.ini`**，INI 格式，便于携带与排障。  
- 实现上统一使用 **`QSettings(IniFormat, iniPath)`**，路径基于 **`applicationDirPath()`**。  
- 注意 **Program Files** 写权限问题；绿色包解压到可写目录即可。

若后续增加更多模块（主题、语言、设备列表等），继续在**同一 `QtBCI.ini`** 下增加分组即可，无需再引入第二份配置文件路径。
