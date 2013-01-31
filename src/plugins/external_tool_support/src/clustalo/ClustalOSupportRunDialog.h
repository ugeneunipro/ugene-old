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

#ifndef _U2_CLUSTALO_SUPPORT_RUN_DIALOG_H
#define _U2_CLUSTALO_SUPPORT_RUN_DIALOG_H

#include <QDialog>
#include <U2Gui/DialogUtils.h>
#include <ui/ui_ClustalOSupportRunDialog.h>
#include "ClustalOSupportTask.h"

namespace U2 {

class ClustalOSupportRunDialog : public QDialog, public Ui_ClustalOSupportRunDialog {
    Q_OBJECT
public:
    ClustalOSupportRunDialog(const MAlignment& ma, ClustalOSupportTaskSettings& settings, QWidget* parent);
private slots:
    void sl_align();
private:
    MAlignment                      ma;
    ClustalOSupportTaskSettings&    settings;

};

class ClustalOWithExtFileSpecifySupportRunDialog : public QDialog, public Ui_ClustalOSupportRunDialog {
    Q_OBJECT
public:
    ClustalOWithExtFileSpecifySupportRunDialog(ClustalOSupportTaskSettings& settings, QWidget* parent);
private slots:
    void sl_align();
    void sl_inputPathButtonClicked();
    void sl_outputPathButtonClicked();
private:
    ClustalOSupportTaskSettings&    settings;
    void buildMultipleAlignmentUrl(const GUrl &alnUrl);
};

}//namespace
#endif // _U2_CLUSTALO_SUPPORT_RUN_DIALOG_H
