/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#ifndef _U2_CONVERT_ACE_TO_SQLITE_DIALOG_H_
#define _U2_CONVERT_ACE_TO_SQLITE_DIALOG_H_

#include <qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QDialog>
#else
#include <QtWidgets/QDialog>
#endif

#include <U2Core/GUrl.h>

#include "ui/ui_ConvertAceToSqliteDialog.h"

namespace U2 {

class ConvertAceToSqliteDialog : public QDialog, Ui_ConvertAceToSqliteDialog {
    Q_OBJECT
public:
    ConvertAceToSqliteDialog(const GUrl &src);

    const GUrl &getDestinationUrl() const { return destUrl; }
    bool getAddToProject() const { return addToProject; }
    void hideAddToProjectOption(bool hide = true) { cbAddToProject->setVisible(!hide); }

public slots:
    void accept();

private slots:
    void sl_selectFileClicked();

private:
    void initialize(const GUrl &src);
    bool isDestValid();

    GUrl destUrl;
    bool addToProject;
};

}   // namespace

#endif // _U2_CONVERT_ACE_TO_SQLITE_DIALOG_H_
