cmake_minimum_required(VERSION 3.19)
project(QtBCI LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets SerialPort PrintSupport NetWork)

qt_standard_project_setup()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

qt_add_executable(QtBCI
    MANUAL_FINALIZATION
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    thinkgear/thinkgear.h

    core/pictureandalgbuffer.h core/pictureandalgbuffer.cpp
    QCustomPlot/qcustomplot.cpp QCustomPlot/qcustomplot.h
    stream_sdk_for_pc/stream_sdk_for_pc.cpp stream_sdk_for_pc/stream_sdk_for_pc.h
    core/logwbuffer.h core/logwbuffer.cpp
    stream_sdk_for_pc/origintest.h stream_sdk_for_pc/origintest.cpp
    stream_sdk_for_pc/changetest.h stream_sdk_for_pc/changetest.cpp
    stream_sdk_for_pc/threadteast.h stream_sdk_for_pc/threadteast.cpp
    device/serialport.h device/serialport.cpp
    thinkgear/thinkgearframeassembler.h thinkgear/thinkgearframeassembler.cpp
    thinkgear/thinkgearpayloadparser.h thinkgear/thinkgearpayloadparser.cpp
    thinkgear/rawtouvprocessor.h thinkgear/rawtouvprocessor.cpp
    thinkgear/thinkgearlinktester.h thinkgear/thinkgearlinktester.cpp
    core/csvlogworker.h core/csvlogworker.cpp
    core/acquisitionengine.h core/acquisitionengine.cpp
    core/algorithmengine.h core/algorithmengine.cpp
    preprocessing/data_filter.h preprocessing/data_filter.cpp
    preprocessing/eeg_preprocess_pipeline.h preprocessing/eeg_preprocess_pipeline.cpp
    core/psdfeatureextractor.h core/psdfeatureextractor.cpp
    core/fftfeatureextractor.h core/fftfeatureextractor.cpp
    Net/preprocchunkudpsender.h Net/preprocchunkudpsender.cpp


)

# QCustomPlot 的翻译单元非常大，在 MinGW 下容易触发 "too many sections / file too big"。
# 只对该源文件加编译器参数，避免影响 AutoMOC / 预定义宏生成（仅 MinGW）。
if(MINGW)
    set_source_files_properties(
        QCustomPlot/qcustomplot.cpp
        PROPERTIES COMPILE_FLAGS "-Wa,-mbig-obj"
    )
endif()

target_link_libraries(QtBCI
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::SerialPort
        Qt::PrintSupport
        Qt::Network
)

#
# ThinkGear Stream SDK for PC（仅 x64）
#
# 说明：
# - 你现在把 `thinkgear.h` 放到了 `QtBCI/QtBCI/thinkgear/thinkgear.h`
# - 这里同时需要 x64 的 `thinkgear64.lib` + `thinkgear64.dll` 用于链接与运行时拷贝
#
set(THINKGEAR_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/thinkgear")
set(PREPROCESSING_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/preprocessing")

set(THINKGEAR_INC_DIR "${THINKGEAR_ROOT}")

if (NOT EXISTS "${THINKGEAR_INC_DIR}/thinkgear.h")
    message(FATAL_ERROR "thinkgear.h not found. Expect: ${THINKGEAR_INC_DIR}/thinkgear.h")
endif()

# 允许你在不同目录结构下放置 lib/dll（只要存在其中一种就行）
set(_THINKGEAR_LIB_CANDIDATES
    "${THINKGEAR_ROOT}/libs/x64/thinkgear64.lib"
    "${THINKGEAR_ROOT}/x64/thinkgear64.lib"
    "${THINKGEAR_ROOT}/thinkgear64.lib"
)

set(_THINKGEAR_DLL_CANDIDATES
    "${THINKGEAR_ROOT}/libs/x64/thinkgear64.dll"
    "${THINKGEAR_ROOT}/x64/thinkgear64.dll"
    "${THINKGEAR_ROOT}/thinkgear64.dll"
)

set(THINKGEAR_LIB "")
foreach(p IN LISTS _THINKGEAR_LIB_CANDIDATES)
    if (EXISTS "${p}")
        set(THINKGEAR_LIB "${p}")
        break()
    endif()
endforeach()

set(THINKGEAR_DLL "")
foreach(p IN LISTS _THINKGEAR_DLL_CANDIDATES)
    if (EXISTS "${p}")
        set(THINKGEAR_DLL "${p}")
        break()
    endif()
endforeach()

if (THINKGEAR_LIB STREQUAL "")
    message(FATAL_ERROR "thinkgear64.lib not found. Put it under one of: ${_THINKGEAR_LIB_CANDIDATES}")
endif()

if (THINKGEAR_DLL STREQUAL "")
    message(FATAL_ERROR "thinkgear64.dll not found. Put it under one of: ${_THINKGEAR_DLL_CANDIDATES}")
endif()

# BrainFlow DataHandler 库（DataFilter 的底层实现）
set(_DATAHANDLER_LIB_CANDIDATES
    "${PREPROCESSING_ROOT}/DataHandler.lib"
    "${PREPROCESSING_ROOT}/data_handler.lib"
)

set(_DATAHANDLER_DLL_CANDIDATES
    "${PREPROCESSING_ROOT}/DataHandler.dll"
    "${PREPROCESSING_ROOT}/data_handler.dll"
)

set(DATAHANDLER_LIB "")
foreach(p IN LISTS _DATAHANDLER_LIB_CANDIDATES)
    if (EXISTS "${p}")
        set(DATAHANDLER_LIB "${p}")
        break()
    endif()
endforeach()

set(DATAHANDLER_DLL "")
foreach(p IN LISTS _DATAHANDLER_DLL_CANDIDATES)
    if (EXISTS "${p}")
        set(DATAHANDLER_DLL "${p}")
        break()
    endif()
endforeach()

if (DATAHANDLER_LIB STREQUAL "")
    message(FATAL_ERROR "DataHandler.lib not found. Checked: ${_DATAHANDLER_LIB_CANDIDATES}")
endif()

if (DATAHANDLER_DLL STREQUAL "")
    message(FATAL_ERROR "DataHandler.dll not found. Checked: ${_DATAHANDLER_DLL_CANDIDATES}")
endif()

target_include_directories(QtBCI
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${THINKGEAR_INC_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/preprocessing"
)

# BrainFlow DataFilter 头文件目录（给 preprocessing/data_filter.h 使用）
set(_BRAINFLOW_UTILS_INC_CANDIDATES
    "${CMAKE_CURRENT_SOURCE_DIR}/../brainflow-master/brainflow-master/src/utils/inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../brainflow-master/brainflow-master/src/utils/inc"
)
set(_BRAINFLOW_DATA_HANDLER_INC_CANDIDATES
    "${CMAKE_CURRENT_SOURCE_DIR}/../brainflow-master/brainflow-master/src/data_handler/inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../brainflow-master/brainflow-master/src/data_handler/inc"
)

set(BRAINFLOW_UTILS_INC "")
foreach(p IN LISTS _BRAINFLOW_UTILS_INC_CANDIDATES)
    if (EXISTS "${p}/brainflow_array.h")
        set(BRAINFLOW_UTILS_INC "${p}")
        break()
    endif()
endforeach()

set(BRAINFLOW_DATA_HANDLER_INC "")
foreach(p IN LISTS _BRAINFLOW_DATA_HANDLER_INC_CANDIDATES)
    if (EXISTS "${p}/data_handler.h")
        set(BRAINFLOW_DATA_HANDLER_INC "${p}")
        break()
    endif()
endforeach()

if (BRAINFLOW_UTILS_INC STREQUAL "")
    message(FATAL_ERROR "brainflow_array.h not found. Checked: ${_BRAINFLOW_UTILS_INC_CANDIDATES}")
endif()

if (BRAINFLOW_DATA_HANDLER_INC STREQUAL "")
    message(FATAL_ERROR "data_handler.h not found. Checked: ${_BRAINFLOW_DATA_HANDLER_INC_CANDIDATES}")
endif()

target_include_directories(QtBCI
    PRIVATE
        "${BRAINFLOW_UTILS_INC}"
        "${BRAINFLOW_DATA_HANDLER_INC}"
)

target_link_libraries(QtBCI
    PRIVATE
        "${THINKGEAR_LIB}"
        "${DATAHANDLER_LIB}"
)

add_custom_command(TARGET QtBCI POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${THINKGEAR_DLL}"
        "${DATAHANDLER_DLL}"
        "$<TARGET_FILE_DIR:QtBCI>"
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(QtBCI)
endif()

include(GNUInstallDirs)

install(TARGETS QtBCI
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET QtBCI
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
