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

#ifndef _U2_ACE_IMPORT_DIALOG_H_
#define _U2_ACE_IMPORT_DIALOG_H_

#include <U2Formats/AceImporter.h>

#include "ui_AceImportDialog.h"

namespace U2 {

class SaveDocumentController;

class AceImportDialog : public ImportDialog, public Ui_AceImportDialog {
    Q_OBJECT
public:
    AceImportDialog(const QVariantMap& settings);

protected:
    bool isValid();
    void applySettings();
    void initSaveController();

    SaveDocumentController *saveController;

    static const QString EXTENSION;
};

}   // namespace U2

#endif // _U2_ACE_IMPORT_DIALOG_H_
