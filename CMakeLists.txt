cmake_minimum_required(VERSION 3.19)
project(QtBCI LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets SerialPort PrintSupport NetWork)

qt_standard_project_setup()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# BrainFlow 头文件与 DataHandler 预编译库：与 CMakeLists.txt 同目录下的 openbci/（便于仅推送 QtBCI 子目录到 GitHub）
set(OPENBCI_VENDOR_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/openbci")

qt_add_executable(QtBCI
    MANUAL_FINALIZATION
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    chartmodedialog.h chartmodedialog.cpp chartmodedialog.ui
    serialconfigdialog.h serialconfigdialog.cpp serialconfigdialog.ui
    saveconfigdialog.h saveconfigdialog.cpp saveconfigdialog.ui

    core/pictureandalgbuffer.h core/pictureandalgbuffer.cpp
    QCustomPlot/qcustomplot.cpp QCustomPlot/qcustomplot.h
    core/logwbuffer.h core/logwbuffer.cpp
    device/serialport.h device/serialport.cpp
    device/serial_eeg/eeg_frame_assembler.h device/serial_eeg/eeg_frame_assembler.cpp
    device/serial_eeg/eeg_payload_parser.h device/serial_eeg/eeg_payload_parser.cpp
    device/serial_eeg/raw_to_uv_processor.h device/serial_eeg/raw_to_uv_processor.cpp
    core/csvlogworker.h core/csvlogworker.cpp
    core/acquisitionengine.h core/acquisitionengine.cpp
    core/algorithmengine.h core/algorithmengine.cpp
    preprocessing/data_filter.h preprocessing/data_filter.cpp
    preprocessing/eeg_preprocess_pipeline.h preprocessing/eeg_preprocess_pipeline.cpp
    core/psdfeatureextractor.h core/psdfeatureextractor.cpp
    core/fftfeatureextractor.h core/fftfeatureextractor.cpp
    Net/networkstreamsettings.h Net/networkstreamsettings.cpp
    Net/udptelemetryhub.h Net/udptelemetryhub.cpp
    Net/networkconfigdialog.h Net/networkconfigdialog.cpp
    Net/networkconfigdialog.ui
)

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

set(PREPROCESSING_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/preprocessing")

# BrainFlow DataHandler 预编译库（优先工程内 openbci/lib/...，其次 preprocessing/）
set(_DATAHANDLER_LIB_CANDIDATES
    "${OPENBCI_VENDOR_ROOT}/lib/mingw_64/DataHandler.lib"
    "${OPENBCI_VENDOR_ROOT}/lib/DataHandler.lib"
    "${PREPROCESSING_ROOT}/DataHandler.lib"
    "${PREPROCESSING_ROOT}/data_handler.lib"
)

set(_DATAHANDLER_DLL_CANDIDATES
    "${OPENBCI_VENDOR_ROOT}/lib/mingw_64/DataHandler.dll"
    "${OPENBCI_VENDOR_ROOT}/lib/DataHandler.dll"
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
    message(FATAL_ERROR "DataHandler.lib not found. Place under openbci/lib/... or preprocessing/. Checked: ${_DATAHANDLER_LIB_CANDIDATES}")
endif()

if (DATAHANDLER_DLL STREQUAL "")
    message(FATAL_ERROR "DataHandler.dll not found. Checked: ${_DATAHANDLER_DLL_CANDIDATES}")
endif()

target_include_directories(QtBCI
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/preprocessing"
)

# BrainFlow 头文件：优先工程根目录下 openbci/brainflow_headers/，兼容旧 brainflow-master 路径
set(_BRAINFLOW_UTILS_INC_CANDIDATES
    "${OPENBCI_VENDOR_ROOT}/brainflow_headers/utils_inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/../brainflow-master/brainflow-master/src/utils/inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../brainflow-master/brainflow-master/src/utils/inc"
)
set(_BRAINFLOW_DATA_HANDLER_INC_CANDIDATES
    "${OPENBCI_VENDOR_ROOT}/brainflow_headers/data_handler_inc"
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
    message(FATAL_ERROR "brainflow_array.h not found. Populate ${OPENBCI_VENDOR_ROOT}/brainflow_headers/utils_inc or install brainflow-master alongside. Checked: ${_BRAINFLOW_UTILS_INC_CANDIDATES}")
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
        "${DATAHANDLER_LIB}"
)

add_custom_command(TARGET QtBCI POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
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
