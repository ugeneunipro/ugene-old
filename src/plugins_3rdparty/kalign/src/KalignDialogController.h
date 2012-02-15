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

#ifndef _U2_KALIGN_ALIGN_DIALOG_CONTROLLER_H_
#define _U2_KALIGN_ALIGN_DIALOG_CONTROLLER_H_

#include <QtGui/QDialog>

#include <U2Core/GAutoDeleteList.h>
#include <U2Gui/DialogUtils.h>

#include <ui/ui_KalignDialog.h>
#include "KalignTask.h"

namespace U2 {

class KalignDialogController : public QDialog, public Ui_KalignDialog {
    Q_OBJECT

public:
    KalignDialogController(QWidget* w, const MAlignment& ma, KalignTaskSettings& settings, bool translateEnabled);
    bool translateToAmino();

public slots:
    void accept();

private:
    void setupUiExt(); 
    MAlignment                          ma;
    KalignTaskSettings&                 settings;
};

}//namespace
#endif
