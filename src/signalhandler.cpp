// Copyright (c) 2015-2017, The Bytecoin developers
//
// This file is part of Bytecoin.
//
// Bytecoin is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Bytecoin is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Bytecoin.  If not, see <http://www.gnu.org/licenses/>.

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
