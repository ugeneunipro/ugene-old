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

#ifndef _U2_CREATE_UINDEX_DLG_H_
#define _U2_CREATE_UINDEX_DLG_H_

#include <ui/ui_CreateFileIndexDialog.h>
#include <QtGui/QDialog>

namespace U2 {

    struct CreateFileIndexDialogModel {
        QStringList inputUrls;
        QString     outputUrl;
        bool        gzippedOutput;
        bool        addToProject;
    }; // CreateFileIndexDialogModel

    class CreateFileIndexDialog : public QDialog, Ui_CreateFileIndexDialog {
        Q_OBJECT
    public:
        CreateFileIndexDialog(QWidget* p, bool enableAddToProjButton);

        CreateFileIndexDialogModel getModel() const;

        virtual void accept();

    private:
        CreateFileIndexDialogModel model;

        private slots:
            void sl_openInputFile();
            void sl_openOutputFile();
    };

} // U2

#endif
