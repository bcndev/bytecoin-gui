// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <csignal>

#include "signalhandler.h"
#include "logger.h"

namespace WalletGUI {

SignalHandler& SignalHandler::instance()
{
    static SignalHandler inst;
    return inst;
}

SignalHandler::SignalHandler()
{
    std::signal(SIGINT, SignalHandler::sigHandler);
    std::signal(SIGTERM, SignalHandler::sigHandler);
#ifndef Q_OS_WIN
    std::signal(SIGPIPE, SIG_IGN);
#endif
}

SignalHandler::~SignalHandler()
{}

void SignalHandler::sigHandler(int signal)
{
    WalletLogger::warning(tr("[SignalHandler] Signal received: %1").arg(signal));
    emit SignalHandler::instance().quitSignal();
}

}
