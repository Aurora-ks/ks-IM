cmake_minimum_required(VERSION 3.28)
project(Client)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include_directories(include)
include_directories(proto)

find_package(spdlog CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

# 指定qt安装目录
set(CMAKE_PREFIX_PATH "D:/Qt/6.7.3/mingw_64")
set(ElaDll ${CMAKE_CURRENT_SOURCE_DIR}/lib/ElaWidgetTools.dll)

find_package(
        Qt6 COMPONENTS
        Core
        Gui
        Widgets
        Sql
        Network
        WebSockets
        REQUIRED)

add_executable(Client
        resources.qrc
        proto/protocol.pb.cc
        main.cpp
        LoginWindow.cpp
        include/LoginWindow.h
        RegisterWindow.cpp
        logger.cpp
        setting.cpp
        net.cpp
        MainWindow.cpp
        include/MainWindow.h
        user.cpp
        UserPage.cpp
        SettingPage.cpp
        RelationPage.cpp
        include/RelationPage.h
        RelationNotifyWidget.cpp
        include/RelationNotifyWidget.h
        model/FriendListModel.cpp
        include/FriendListModel.h
        UserInfoCard.cpp
        include/UserInfoCard.h
        model/FriendTreeViewItem.cpp
        include/FriendTreeViewItem.h
        SessionPage.cpp
        include/SessionPage.h
        model/SessionListModel.cpp
        include/SessionListModel.h
        model/MessageListModel.cpp
        include/MessageListModel.h
        include/ws.h
        ws.cpp
)

target_link_libraries(Client
        PRIVATE spdlog::spdlog
        PRIVATE Qt6::Core
        PRIVATE Qt6::Gui
        PRIVATE Qt6::Widgets
        PRIVATE Qt6::Sql
        PRIVATE Qt6::Network
        PRIVATE Qt6::WebSockets
        PRIVATE protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite
        PRIVATE ${ElaDll}
)

if (WIN32)
    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif ()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif ()
    foreach (QT_LIB Core Gui Widgets Sql Network WebSockets)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
endif ()

# 复制Dll
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${ElaDll}
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        COMMENT "Copying DLL file to executable directory"
)
# 复制 SQL 驱动插件
set(SQL_DRIVER_PLUGIN_DIR "${QT_INSTALL_PATH}/plugins/sqldrivers")
if (EXISTS "${SQL_DRIVER_PLUGIN_DIR}/qsqlite.dll")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/sqldrivers/")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${SQL_DRIVER_PLUGIN_DIR}/qsqlite${DEBUG_SUFFIX}.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/sqldrivers/")
endif ()