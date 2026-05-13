# BrainFlow 预处理依赖（与 `CMakeLists.txt` 同目录）

主工程 `CMakeLists.txt` 将 **`OPENBCI_VENDOR_ROOT`** 设为 **`${CMAKE_CURRENT_SOURCE_DIR}/openbci`**，即本目录与 `main.cpp`、`device/` 等并列。若你**只把内层 `QtBCI/` 文件夹推送到 GitHub**，请把 BrainFlow 链接所需内容一并放在此 `openbci/` 下并提交。

## 目录约定

```text
openbci/
  brainflow_headers/
    utils_inc/          # brainflow_array.h、brainflow_constants.h 等
    data_handler_inc/   # data_handler.h
  lib/
    mingw_64/           # MinGW 64 位 Kit 示例（MSVC 可另建 msvc2019_64 等并在 CMake 中增加候选路径）
      DataHandler.lib
      DataHandler.dll
```

可从官方 [BrainFlow](https://github.com/brainflow-dev/brainflow) 源码树复制 `src/utils/inc` 与 `src/data_handler/inc` 下的头文件到 `brainflow_headers/`；`DataHandler` 预编译库需与本地 Qt Kit 工具链、位数一致，并与头文件版本匹配。

若本目录缺少库文件，CMake 仍会尝试 **`preprocessing/`** 或仓库外的 **`../brainflow-master/...`** 作为兼容路径（不推荐用于「仅推送内层目录」的协作方式）。
