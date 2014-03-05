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

#ifndef _U2_SAVE_DOCUMENT_GROUP_CONTROLLER_H_
#define _U2_SAVE_DOCUMENT_GROUP_CONTROLLER_H_

#include <U2Core/DocumentModel.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QLineEdit>
#include <QtGui/QComboBox>
#include <QtGui/QToolButton>
#else
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QToolButton>
#endif

namespace U2{


class DocumentFormatComboboxController;

class SaveDocumentGroupControllerConfig {
public:
    DocumentFormatConstraints   dfc;                // format constraints applied for formats combo
    QToolButton*                fileDialogButton;   // a button to open save file dialog
    QString                     saveTitle;          // a title for save file dialog
    QString                     defaultFileName;    // filename set by default
    QComboBox*                  formatCombo;        // combo widget to list formats
    QLineEdit*                  fileNameEdit;       // edit for file name
    DocumentFormatId            defaultFormatId;    // format selected by default
    QWidget*                    parentWidget;       // parent widget for file dialog
    QString                     objectName;         // custom object name
};

/* Controls 'save document' fields :
    - combo with document format selection
    - edit field with document name
*/
class U2GUI_EXPORT SaveDocumentGroupController : public QObject {
    Q_OBJECT
public:
    SaveDocumentGroupController(const SaveDocumentGroupControllerConfig& conf, QObject* parent);
    
    QString getSaveFileName() const {return conf.fileNameEdit->text();}
    
    void setSelectedFormatId(DocumentFormatId id);

    DocumentFormatId getFormatIdToSave() const;
    
    DocumentFormat* getFormatToSave() const;
    
    QString getDefaultFileName() const {return conf.defaultFileName;}
    
private slots:
    void sl_fileNameChanged(const QString& v);
    void sl_saveButtonClicked();
    void sl_formatChanged(const QString& newFormat);

private:
    DocumentFormatComboboxController* comboController;
    SaveDocumentGroupControllerConfig conf;
};

}//namespace

#endif

