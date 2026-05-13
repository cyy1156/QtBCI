# Legacy ThinkGear 厂商 SDK 与旧示例（不参与主工程构建）

本目录存放 **NeuroSky ThinkGear Stream SDK for PC** 相关头文件（`thinkgear.h`）、依赖 `TG_*` API 的 `stream_sdk_for_pc/` 示例，以及旧的 `ThinkGearLinkTester` 单链路测试代码。

- **主程序 `QtBCI` 的 CMake 不引用本目录**，拉取仓库后默认构建不包含 ThinkGear 官方 `thinkgear64.lib`。
- 若需在本地单独编译其中示例，请自行建立工程，将 `thinkgear64.lib` / `thinkgear64.dll` 加入链接，并为包含路径添加本目录及 `stream_sdk_for_pc/`。
- 自研串口组帧与解析已迁移至主工程 `device/serial_eeg/`，与厂商 DLL 无关。
