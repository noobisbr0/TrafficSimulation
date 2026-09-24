cmake_minimum_required(VERSION 3.16)

project(TrafficSimulation VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Включаем автоматическую обработку мета-объектов Qt (сигналы, слоты, UI-файлы)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Поиск библиотек Qt
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets)

# -------------------------------------------------------------
# 1. БЭКЕНД: Библиотека логики симуляции (TrafficCore)
# -------------------------------------------------------------
# Ищет все заголовочные и исходные файлы в папках ядра и общих типов
file(GLOB_RECURSE CORE_SOURCES
    "src/common/*.h"
    "src/common/*.cpp"
    "src/core/*.h"
    "src/core/*.cpp"
)

add_library(TrafficCore STATIC
    ${CORE_SOURCES}
)

target_include_directories(TrafficCore PUBLIC
    src/
    src/common/
    src/core/
)

# Для бэкенда достаточно только QtCore (или вообще чистый C++, если не нужны сигналы/слоты)
target_link_libraries(TrafficCore PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
)

# -------------------------------------------------------------
# 2. ФРОНТЕНД: Приложение с графическим интерфейсом
# -------------------------------------------------------------
file(GLOB_RECURSE UI_SOURCES
    "src/ui/*.h"
    "src/ui/*.cpp"
    "src/ui/*.ui"
)

# Файл ресурсов (если есть)
set(RESOURCES "")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources/resources.qrc")
    set(RESOURCES "resources/resources.qrc")
endif()

add_executable(${PROJECT_NAME}
    src/main.cpp
    ${UI_SOURCES}
    ${RESOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    src/ui/
)

# Подключаем к GUI-приложению бэкенд и QtWidgets
target_link_libraries(${PROJECT_NAME} PRIVATE
    TrafficCore
    Qt${QT_VERSION_MAJOR}::Widgets
)