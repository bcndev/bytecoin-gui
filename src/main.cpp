// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QApplication>
#include <cstdio>
#include "mainwindow.h"
#include "application.h"

int main(int argc, char *argv[])
{
    WalletGUI::WalletApplication app(argc, argv);
    try
    {
        if (!app.init())
            return 0;
        return app.exec();
    }
    catch (const std::exception& _error)
    {
        fprintf(stderr, "[Main] Unhandled exception: %s\n", _error.what());
        return 0;
    }
}
