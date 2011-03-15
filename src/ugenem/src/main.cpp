/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <QtGui/QApplication>
#include <QtGui/QMessageBox>

#include "SendReportDialog.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString message;
    if(argc > 1) {
        message = QString::fromUtf8(QByteArray::fromBase64(argv[1]));
    } else {
        message = "";
    }
    SendReportDialog dlg(message);
    dlg.setWindowIcon(QIcon(":ugenem/images/crash_icon.png"));
    dlg.exec();
    return 0;
}