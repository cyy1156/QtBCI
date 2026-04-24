include("D:/360MoveData/Users/ckgxnn/Desktop/QtBCI/QtBCI/build/Desktop_Qt_6_10_2_MinGW_64_bit-Release/.qt/QtDeploySupport.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/QtBCI-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase;qtserialport")

qt6_deploy_runtime_dependencies(
    EXECUTABLE "D:/360MoveData/Users/ckgxnn/Desktop/QtBCI/QtBCI/build/Desktop_Qt_6_10_2_MinGW_64_bit-Release/QtBCI.exe"
    GENERATE_QT_CONF
)
