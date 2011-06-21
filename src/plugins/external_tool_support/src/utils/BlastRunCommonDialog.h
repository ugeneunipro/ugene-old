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

#ifndef _U2_BLAST_RUN_COMMON_DIALOG_H
#define _U2_BLAST_RUN_COMMON_DIALOG_H

#include <QDialog>
#include <U2Core/DNASequenceObject.h>
#include <U2Misc/DialogUtils.h>
#include <ui/ui_BlastAllSupportDialog.h>
#include <U2Gui/CreateAnnotationWidgetController.h>
#include "BlastTaskSettings.h"

namespace U2 {

class BlastRunCommonDialog : public QDialog, public Ui_BlastAllSupportDialog {
    Q_OBJECT
public:
    BlastRunCommonDialog(BlastTaskSettings& settings, QWidget* parent);
protected slots:
    virtual void sl_runQuery() = 0;
    virtual void sl_lineEditChanged() = 0;

    void sl_onBrowseDatabasePath();//=

    void sl_onProgNameChange(int i);//=
    void sl_onMatchScoresChanged(int i);//=
    void sl_onMatrixChanged(int i);//=
    void sl_megablastChecked();//=
    void sl_restoreDefault();//=
protected:
    void getSettings(BlastTaskSettings& settings);

    BlastTaskSettings&                  settings;
    bool                                needRestoreDefault;
    CreateAnnotationWidgetController*   ca_c;
};
}//namespace
#endif // _U2_BLAST_RUN_COMMON_DIALOG_H
