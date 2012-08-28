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

#ifndef _U2_MAFFT_SUPPORT_RUN_DIALOG_H
#define _U2_MAFFT_SUPPORT_RUN_DIALOG_H

#include <QtGui/QDialog>
#include <U2Gui/DialogUtils.h>
#include <ui/ui_MAFFTSupportRunDialog.h>
#include "MAFFTSupportTask.h"

namespace U2 {

class MAFFTSupportRunDialog : public QDialog, public Ui_MAFFTSupportRunDialog {
    Q_OBJECT
public:
    MAFFTSupportRunDialog(MAFFTSupportTaskSettings& settings, QWidget* parent);
private slots:
    void sl_align();
private:
    MAFFTSupportTaskSettings&        settings;
};

class MAFFTWithExtFileSpecifySupportRunDialog : public QDialog, public Ui_MAFFTSupportRunDialog {
    Q_OBJECT
public:
    MAFFTWithExtFileSpecifySupportRunDialog(MAFFTSupportTaskSettings& settings, QWidget* parent);
private slots:
    void sl_align();
    void sl_inputPathButtonClicked();
    void sl_outputPathButtonClicked();
private:
    MAFFTSupportTaskSettings&     settings;
    void buildMultipleAlignmentUrl(const GUrl &alnUrl);};

}//namespace
#endif // _U2_MAFFT_SUPPORT_RUN_DIALOG_H
