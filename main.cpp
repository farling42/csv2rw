/*
RWImporter
Copyright (C) 2017 Martin Smith

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include "errordialog.h"

static QtMessageHandler orig_handler;

static void message_handler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    switch (type)
    {
    case QtWarningMsg:
    case QtCriticalMsg:
        // Display the issue to the user
        ErrorDialog::theInstance()->addMessage(message);
        return;

    case QtInfoMsg:
    case QtFatalMsg:
    case QtDebugMsg:
        // Use standard error handler
        break;
    }
    orig_handler(type, context, message);
}


int main(int argc, char *argv[])
{
    orig_handler = qInstallMessageHandler(message_handler);

    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("Amusing Time");
    QCoreApplication::setOrganizationDomain("amusingtime.uk");
    QCoreApplication::setApplicationName("RWImporter");

    MainWindow w;
    w.show();

    return a.exec();
}
