/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#ifndef _U2_EXPORT_ANNOTATIONS_DIALOG_H_
#define _U2_EXPORT_ANNOTATIONS_DIALOG_H_

#include <QDialog>

#include <U2Core/global.h>

namespace U2 {

namespace Ui {

class ExportAnnotationsDialog;

}

class SaveDocumentController;

class U2GUI_EXPORT ExportAnnotationsDialog : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY( ExportAnnotationsDialog )
public:
                                    ExportAnnotationsDialog( const QString &filename,
                                            QWidget *parent );
                                    ~ExportAnnotationsDialog( );

    QString                         fileFormat( ) const;
    QString                         filePath( ) const;
    bool                            exportSequence( ) const;
    bool                            exportSequenceNames( ) const;
    void                            setExportSequenceVisible( bool value );

    static const QString            CSV_FORMAT_ID;

private:
    void                            initSaveController(const QString &filename);

    QList<QString>                  supportedFormatsExts;
    Ui::ExportAnnotationsDialog *   ui;
    SaveDocumentController *        saveController;
};

} // namespace U2

#endif // _U2_EXPORT_ANNOTATIONS_DIALOG_H_
