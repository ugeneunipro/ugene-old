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

#ifndef _GB2_UHMM3_BUILD_DLG_IMPL_H_
#define _GB2_UHMM3_BUILD_DLG_IMPL_H_

#include <QtGui/QDialog>

#include "uHMM3BuildTask.h"
#include <ui/ui_UHMM3BuildDialog.h>

namespace U2 {

struct UHMM3BuildDialogModel {
    UHMM3BuildTaskSettings  buildSettings;
    
    /* one of this is used */
    QString                 inputFile;
    MAlignment              alignment;
    bool                    alignmentUsing;
    
}; // UHMM3BuildDialogModel

class UHMM3BuildDialogImpl : public QDialog, public Ui_UHMM3BuildDialog {
Q_OBJECT
public:
    static const QString MA_FILES_DIR_ID;
    static const QString HMM_FILES_DIR_ID;
    
public:
    UHMM3BuildDialogImpl( const MAlignment & ma, QWidget * p = NULL );
    
private:
    void setModelValues();
    void getModelValues();
    QString checkModel(); /* return error or empty string */
    void setSignalsAndSlots();
    void initialize();
    
private slots:
    void sl_maOpenFileButtonClicked();
    void sl_outHmmFileButtonClicked();
    void sl_buildButtonClicked();
    void sl_cancelButtonClicked();
    void sl_fastMCRadioButtonChanged( bool checked );
    void sl_wblosumRSWRadioButtonChanged( bool checked );
    void sl_eentESWRadioButtonChanged( bool checked );
    void sl_eclustESWRadioButtonChanged( bool changed );
    void sl_esetESWRadioButtonChanged( bool checked );
    
private:
    UHMM3BuildDialogModel   model;
    
}; // UHMM3BuildDialogImpl

} // U2

#endif // _GB2_UHMM3_BUILD_DLG_IMPL_H_
