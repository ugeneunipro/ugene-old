/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_EXPORT_ANNOTATIONS_2_CSV_DIALOG_H_
#define _U2_EXPORT_ANNOTATIONS_2_CSV_DIALOG_H_

#include <QtGui/QDialog>
#include <ui/ui_ExportAnnotationsDialog.h>

namespace U2 {

class ExportAnnotationsDialog : public QDialog, public Ui::ExportAnnotationsDialog {
    Q_OBJECT
    Q_DISABLE_COPY(ExportAnnotationsDialog)
public:
    static const QString CSV_FORMAT_ID;
    
public:
    explicit ExportAnnotationsDialog(const QString & filename, QWidget *parent);
    
    QString filePath()const;
    bool exportSequence()const;
    void setExportSequenceVisible(bool value);
    
    QString fileFormat()const;

protected:
    virtual void changeEvent(QEvent *e);

private slots:
    void sl_onChooseFileButtonClicked();
    void sl_onFormatChanged(const QString &);

private:
    QList<QString> supportedFormatsExts;
};

} // namespace U2

#endif 
