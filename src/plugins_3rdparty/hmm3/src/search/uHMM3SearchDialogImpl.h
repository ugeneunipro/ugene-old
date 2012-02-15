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

#ifndef _GB2_UHMM3_SEARCH_DLG_IMPL_H_
#define _GB2_UHMM3_SEARCH_DLG_IMPL_H_

#include <QtGui/QDialog>
#include <QtGui/QButtonGroup>

#include <U2Core/DNASequence.h>
#include <U2Gui/CreateAnnotationWidgetController.h>
#include <U2Core/DNASequenceObject.h>

#include <ui/ui_UHMM3SearchDialog.h>
#include <search/uHMM3SearchTask.h>

namespace U2 {

struct UHMM3SearchDialogModel {
    UHMM3SearchTaskSettings searchSettings;
    QString                 hmmfile;
    DNASequence             sequence;
}; // UHMM3SearchDialogModel

class UHMM3SearchDialogImpl : public QDialog, public Ui_UHMM3SearchDialog {
    Q_OBJECT
public:
    static const QString DOM_E_PLUS_PREFIX;
    static const QString DOM_E_MINUS_PREFIX;
    static const QString HMM_FILES_DIR_ID;
    static const QString ANNOTATIONS_DEFAULT_NAME;
    
public:
    UHMM3SearchDialogImpl( const U2SequenceObject * seqObj, QWidget * p = NULL );
    
private:
    void setModelValues();
    void getModelValues();
    QString checkModel();
    
private slots:
    void sl_cancelButtonClicked();
    void sl_okButtonClicked();
    void sl_useEvalTresholdsButtonChanged( bool checked );
    void sl_useScoreTresholdsButtonChanged( bool checked );
    void sl_useExplicitScoreTresholdButton( bool checked );
    void sl_maxCheckBoxChanged( int state );
    void sl_domESpinBoxChanged( int newVal );
    void sl_queryHmmFileToolButtonClicked();
    void sl_domZCheckBoxChanged( int state );
    
private:
    QButtonGroup                        useScoreTresholdGroup;
    CreateAnnotationWidgetController *  annotationsWidgetController;
    UHMM3SearchDialogModel              model;
    
}; // UHMM3SearchDialogImpl

} // U2

#endif // _GB2_UHMM3_SEARCH_DLG_IMPL_H_
