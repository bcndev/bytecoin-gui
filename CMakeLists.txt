# to build on mac you should have added a path to your qt directory in CMAKE_PREFIX_PATH environment variable
# for ex.: export CMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH:/Users/testnet/Qt/5.9.2/clang_64/lib/cmake"

cmake_minimum_required(VERSION 3.0.2)
project(armor-gui)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++14")
set(CONFIG "${CONFIG} c++14 strict_c++")

if(APPLE)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.11")
endif()

file(GLOB SOURCES
    src/*.cpp
    src/JsonRpc/*.cpp
    src/Miner/*.cpp
)

include_directories(${CMAKE_BINARY_DIR} src )
include_directories(../armor/include)
include_directories(../armor/src)
link_directories(../armor/libs)
add_executable(armor-gui ${SOURCES} src/resources.qrc)
target_link_libraries(armor-gui armor-crypto)
qt5_use_modules(armor-gui Core Network Gui Widgets)
