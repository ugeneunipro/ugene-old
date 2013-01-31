/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#ifndef _U2_EXPORT_READS_DIALOG_H__
#define _U2_EXPORT_READS_DIALOG_H__

#include <U2Core/global.h>

#include <ui/ui_ExportReadsDialog.h>
#include <QtGui/QDialog>

namespace U2 {

struct ExportReadsDialogModel {
    QString filepath;
    DocumentFormatId format;
    bool addToProject;
};

class ExportReadsDialog : public QDialog, Ui_ExportReadsDialog {
    Q_OBJECT
public:
    ExportReadsDialog(QWidget * p, const QList<DocumentFormatId> & formats);
    
    virtual void accept();
    
    ExportReadsDialogModel getModel() const;

private slots:
    void sl_selectFile();
    
private:
    QString filter;
    
}; // ExportReadsDialog

} // U2

#endif // _U2_EXPORT_READS_DIALOG_H__
