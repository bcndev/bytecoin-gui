#-------------------------------------------------
#
# Project created by QtCreator 2017-10-02T15:01:03
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = bytecoin-gui
TEMPLATE = app

!win32: QMAKE_CXXFLAGS += -std=c++14 -Wall -Wextra -pedantic
macx: QMAKE_CXXFLAGS += -mmacosx-version-min=10.11
macx: QMAKE_CFLAGS += -mmacosx-version-min=10.11
macx: ICON = images/bytecoin.icns

#QMAKE_CXXFLAGS += -fno-omit-frame-pointer -fsanitize=address,undefined
#LIBS += -lasan -lubsan

CONFIG += c++14 strict_c++

# copy walletd adjacent to bytecoin-gui binary on all 3 platforms
win32 {
WALLETD_BY_SRC_PATH = $$shell_path($$clean_path("$$PWD/../../bytecoin/bin/walletd.exe"))
BYTECOIND_BY_SRC_PATH = $$shell_path($$clean_path("$$PWD/../../bytecoin/bin/bytecoind.exe"))
Debug:BY_DST_PATH = $$shell_path($$clean_path("$$OUT_PWD/debug"))
Release:BY_DST_PATH = $$shell_path($$clean_path("$$OUT_PWD/release"))
copywalletd.commands = $(COPY_FILE) $${WALLETD_BY_SRC_PATH} $${BY_DST_PATH}
copybytecoind.commands = $(COPY_FILE) $${BYTECOIND_BY_SRC_PATH} $${BY_DST_PATH}
}else:macx {
copywalletd.commands += $(COPY_FILE) $$PWD/../../bytecoin/bin/walletd $$OUT_PWD/bytecoin-gui.app/Contents/MacOS
copybytecoind.commands += $(COPY_FILE) $$PWD/../../bytecoin/bin/bytecoind $$OUT_PWD/bytecoin-gui.app/Contents/MacOS
}else {
copywalletd.commands += $(COPY_FILE) $$PWD/../../bytecoin/bin/walletd $$OUT_PWD
copybytecoind.commands += $(COPY_FILE) $$PWD/../../bytecoin/bin/bytecoind $$OUT_PWD
}
first.depends = $(first) copywalletd copybytecoind
export(first.depends)
export(copywalletd.commands)
export(copybytecoind.commands)
QMAKE_EXTRA_TARGETS += first copywalletd copybytecoind

SOURCES += main.cpp\
    mainwindow.cpp \
    signalhandler.cpp \
    overviewframe.cpp \
    aboutdialog.cpp \
    JsonRpc/JsonRpcClient.cpp \
    JsonRpc/JsonRpcNotification.cpp \
    JsonRpc/JsonRpcObject.cpp \
    JsonRpc/JsonRpcObjectFactory.cpp \
    JsonRpc/JsonRpcRequest.cpp \
    JsonRpc/JsonRpcResponse.cpp \
    application.cpp \
    logger.cpp \
    okbutton.cpp \
    statusbar.cpp \
    tableview.cpp \
    windoweditemmodel.cpp \
    walletmodel.cpp \
    sendframe.cpp \
    transferframe.cpp \
    resizablescrollarea.cpp \
    common.cpp \
    connectionoptionsframe.cpp \
    optionsdialog.cpp \
    settings.cpp \
    Miner/Miner.cpp \
    Miner/StratumClient.cpp \
    Miner/Worker.cpp \
    MinerDelegate.cpp \
    MinerModel.cpp \
    MiningFrame.cpp \
    MiningManager.cpp \
    PoolHeaderView.cpp \
    QuestionDialog.cpp \
    connectselectiondialog.cpp \
    walletd.cpp \
    rpcapi.cpp \
    progressbar.cpp \
    addressbookframe.cpp \
    addressbookmodel.cpp \
    addressbooksortedmodel.cpp \
    newaddressdialog.cpp \
    addressbookdelegate.cpp \
    addressbookmanager.cpp \
    balanceoverviewframe.cpp \
    miningoverviewframe.cpp \
    crashdialog.cpp \
    changepassworddialog.cpp \
    sendconfirmationdialog.cpp \
    addressbookdialog.cpp \
    popup.cpp \
    logframe.cpp \
    askpassworddialog.cpp

HEADERS  += mainwindow.h \
    signalhandler.h \
    overviewframe.h \
    aboutdialog.h \
    JsonRpc/JsonRpcClient.h \
    JsonRpc/JsonRpcNotification.h \
    JsonRpc/JsonRpcObject.h \
    JsonRpc/JsonRpcObjectFactory.h \
    JsonRpc/JsonRpcRequest.h \
    JsonRpc/JsonRpcResponse.h \
    application.h \
    logger.h \
    okbutton.h \
    statusbar.h \
    tableview.h \
    windoweditemmodel.h \
    walletmodel.h \
    sendframe.h \
    transferframe.h \
    resizablescrollarea.h \
    common.h \
    connectionoptionsframe.h \
    optionsdialog.h \
    settings.h \
    Miner/Miner.h \
    Miner/StratumClient.h \
    Miner/Worker.h \
    MinerDelegate.h \
    MinerModel.h \
    MiningFrame.h \
    MiningManager.h \
    PoolHeaderView.h \
    QuestionDialog.h \
    IMinerWorker.h \
    IMiningManager.h \
    IPoolClient.h \
    IPoolMiner.h \
    connectselectiondialog.h \
    walletd.h \
    rpcapi.h \
    progressbar.h \
    addressbookframe.h \
    addressbookmodel.h \
    addressbooksortedmodel.h \
    newaddressdialog.h \
    addressbookdelegate.h \
    addressbookmanager.h \
    balanceoverviewframe.h \
    miningoverviewframe.h \
    crashdialog.h \
    changepassworddialog.h \
    sendconfirmationdialog.h \
    addressbookdialog.h \
    popup.h \
    logframe.h \
    askpassworddialog.h

FORMS    += mainwindow.ui \
    overviewframe.ui \
    aboutdialog.ui \
    sendframe.ui \
    transferframe.ui \
    connectionoptionsframe.ui \
    optionsdialog.ui \
    MiningFrame.ui \
    QuestionDialog.ui \
    connectselectiondialog.ui \
    addressbookframe.ui \
    newaddressdialog.ui \
    balanceoverviewframe.ui \
    miningoverviewframe.ui \
    crashdialog.ui \
    changepassworddialog.ui \
    addressbookdialog.ui \
    logframe.ui \
    askpassworddialog.ui

RESOURCES += \
    resources.qrc \


unix|win32: LIBS += -L$$PWD/../../bytecoin/libs/ -lbytecoin-crypto

INCLUDEPATH += $$PWD/../../bytecoin/src
DEPENDPATH += $$PWD/../../bytecoin/src

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../bytecoin/libs/bytecoin-crypto.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/../../bytecoin/libs/libbytecoin-crypto.a

# to add necessary dependencies,
# 1. delete built bytecoin-gui.app to delete old dependencies (dylibs and frameworks)
# 2. build
# 3. run /Users/user/Qt/5.9.2/clang_64/bin/macdeployqt bytecoin-gui.app
# P.S. in 3, change path accroding to your Qt installation location
