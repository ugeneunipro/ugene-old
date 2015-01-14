/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#ifndef _U2_FOLDER_NANE_DIALOG_H_
#define _U2_FOLDER_NANE_DIALOG_H_

#include <QtCore/qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QDialog>
#else
#include <QtWidgets/QDialog>
#endif

#include "ui_FolderNameDialog.h"

class QPushButton;

namespace U2 {

class FolderNameDialog : public QDialog, public Ui::FolderNameDialog {
    Q_OBJECT
public:
    FolderNameDialog(const QString &name, QWidget *parent);

    QString getResult() const;

private slots:
    void sl_textChanged(const QString &text);

private:
    QPushButton *okButton;
};

} // U2

#endif // _U2_FOLDER_NANE_DIALOG_H_
