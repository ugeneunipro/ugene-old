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

#ifndef _GB2_UHMM3_PHMMER_DIALOG_IMPL_H_
#define _GB2_UHMM3_PHMMER_DIALOG_IMPL_H_

#include <U2Core/DNASequenceObject.h>
#include <U2Core/DNASequence.h>
#include <U2Gui/CreateAnnotationWidgetController.h>

#include <phmmer/uhmm3phmmer.h>
#include <ui/ui_UHMM3PhmmerDialog.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QDialog>
#else
#include <QtWidgets/QDialog>
#endif

namespace U2 {

class UHMM3PhmmerDialogModel {
public:
    UHMM3PhmmerSettings phmmerSettings;
    QString             queryfile;
    DNASequence         dbSequence;
}; // UHMM3PhmmerDialogModel

class UHMM3PhmmerDialogImpl : public QDialog, public Ui_UHMM3PhmmerDialog {
    Q_OBJECT
private:
    static const QString    QUERY_FILES_DIR;
    static const QString    DOM_E_PLUS_PREFIX;
    static const QString    DOM_E_MINUS_PREFIX;
    static const QString    ANNOTATIONS_DEFAULT_NAME;
    static const int        ANNOTATIONS_WIDGET_LOCATION = 1;
public:
    UHMM3PhmmerDialogImpl( const U2SequenceObject * seqObj, QWidget * p = NULL );
    
private:
    void setModelValues();
    void getModelValues();
    QString checkModel();
    
private slots:
    void sl_queryToolButtonClicked();
    void sl_cancelButtonClicked();
    void sl_okButtonClicked();
    void sl_useEvalTresholdsButtonChanged( bool checked );
    void sl_useScoreTresholdsButtonChanged( bool checked );
    void sl_domZCheckBoxChanged( int state );
    void sl_maxCheckBoxChanged( int state );
    void sl_domESpinBoxChanged( int newVal );
    
private:
    UHMM3PhmmerDialogModel model;
    CreateAnnotationWidgetController *  annotationsWidgetController;
    
}; // UHMM3PhmmerDialogImpl

} // U2

#endif // _GB2_UHMM3_PHMMER_DIALOG_IMPL_H_
