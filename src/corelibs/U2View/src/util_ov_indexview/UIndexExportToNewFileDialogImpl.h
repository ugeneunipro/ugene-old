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


#ifndef _U2_UINDEX_EXPORT_TO_NEW_DIALOG_IMPL_H_
#define _U2_UINDEX_EXPORT_TO_NEW_DIALOG_IMPL_H_

#include <ui/ui_UIndexExportToNewFileDialog.h>

namespace U2 {

struct UIndexExportToNewFileDialogModel {
    QString exportFilename;
    bool    gzipExported;
};

class UIndexExportToNewFileDialogImpl : public QDialog, public Ui::UIndexExportToNewFileDialog {
    Q_OBJECT
public:
    static const QString SETTINGS_LAST_DIRECTORY;
    
    UIndexExportToNewFileDialogImpl();
    
    UIndexExportToNewFileDialogModel getModel() const;
    
    virtual void accept();

private slots:
    void sl_openExportFile();
    
private:
    UIndexExportToNewFileDialogModel model;
    
}; // UIndexExportToNewFileDialogImpl

} // U2

#endif // _U2_UINDEX_EXPORT_TO_NEW_DIALOG_IMPL_H_
