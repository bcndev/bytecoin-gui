# to build on mac you should have added a path to your qt directory in CMAKE_PREFIX_PATH environment variable
# for ex.: export CMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH:/Users/testnet/Qt/5.9.2/clang_64/lib/cmake"

cmake_minimum_required(VERSION 3.0.2)
project(bytecoin-gui)
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

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp 
    src/signalhandler.cpp 
    src/overviewframe.cpp 
    src/aboutdialog.cpp 
    src/JsonRpc/JsonRpcClient.cpp 
    src/JsonRpc/JsonRpcNotification.cpp 
    src/JsonRpc/JsonRpcObject.cpp 
    src/JsonRpc/JsonRpcObjectFactory.cpp 
    src/JsonRpc/JsonRpcRequest.cpp 
    src/JsonRpc/JsonRpcResponse.cpp 
    src/application.cpp 
    src/logger.cpp 
    src/okbutton.cpp 
    src/statusbar.cpp 
    src/windoweditemmodel.cpp 
    src/walletmodel.cpp 
    src/sendframe.cpp 
    src/transferframe.cpp 
    src/resizablescrollarea.cpp 
    src/common.cpp 
    src/connectionoptionsframe.cpp 
    src/settings.cpp 
    src/Miner/Miner.cpp 
    src/Miner/StratumClient.cpp 
    src/Miner/Worker.cpp 
    src/MinerDelegate.cpp 
    src/MinerModel.cpp 
    src/MiningFrame.cpp 
    src/MiningManager.cpp 
    src/connectselectiondialog.cpp 
    src/walletd.cpp 
    src/rpcapi.cpp 
    src/progressbar.cpp 
    src/addressbookframe.cpp 
    src/addressbookmodel.cpp 
    src/addressbooksortedmodel.cpp 
    src/newaddressdialog.cpp 
    src/addressbookdelegate.cpp 
    src/addressbookmanager.cpp 
    src/balanceoverviewframe.cpp 
    src/miningoverviewframe.cpp 
    src/crashdialog.cpp 
    src/changepassworddialog.cpp
    src/sendconfirmationdialog.cpp
    src/addressbookdialog.cpp
    src/popup.cpp
    src/logframe.cpp
    src/askpassworddialog.cpp
    src/importkeydialog.cpp
    src/questiondialog.cpp
    src/PoolTreeView.cpp
    src/createproofdialog.cpp
    src/checkproofdialog.cpp
    src/walletdparamsdialog.cpp
    src/exportkeydialog.cpp
    src/filedownloader.cpp
    src/mnemonicdialog.cpp
    src/version.cpp
)

include_directories(${CMAKE_BINARY_DIR} src )
include_directories(../bytecoin/include)
include_directories(../bytecoin/src)
link_directories(../bytecoin/libs)
add_executable(bytecoin-gui ${SOURCES} src/resources.qrc)
target_link_libraries(bytecoin-gui bytecoin-crypto)
qt5_use_modules(bytecoin-gui Core Network Gui Widgets)
