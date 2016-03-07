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

#ifndef _SEND_SELECTION_DIALOG_H_
#define _SEND_SELECTION_DIALOG_H_

#include "ui_RemoteBLASTDialog.h"

#include <U2Core/DNASequenceObject.h>
#include <U2Gui/CreateAnnotationWidgetController.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QMessageBox>
#else
#include <QtWidgets/QMessageBox>
#endif

#include "RemoteBLASTTask.h"
#include "RemoteBLASTConsts.h"


namespace U2 {

class CreateAnnotationWidgetController;
class AnnotationTableObject;

class SendSelectionDialog: public QDialog, Ui_RemoteBLASTDialog {
    Q_OBJECT
public:
    SendSelectionDialog( const U2SequenceObject* dnaso, bool _isAminoSeq, QWidget *p = NULL );
    QString getGroupName() const;
    const QString &getAnnotationDescription() const;
    AnnotationTableObject * getAnnotationObject() const;
    const CreateAnnotationModel *getModel() const;
    QString getUrl() const;
private slots:
    void sl_scriptSelected( int index );
    void sl_megablastChecked(int state);
    void sl_serviceChanged(int);
    void sl_OK();
    void sl_Cancel();

private:
    void setupDataBaseList();
    void setupAlphabet();
    void setUpSettings();
    void saveSettings();
    void alignComboBoxes();

public:
    int retries;
    QString db;
    QString requestParameters;
    bool translateToAmino;
    int filterResults;
    bool useEval;
    RemoteBLASTTaskSettings cfg;

private:
    bool isAminoSeq;
    CreateAnnotationWidgetController * ca_c;
    bool extImported;
};

}

#endif
