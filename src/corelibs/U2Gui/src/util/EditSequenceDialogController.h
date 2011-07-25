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

#ifndef _U2_EDIT_SEQUENCE_CONTROLLER_H_
#define _U2_EDIT_SEQUENCE_CONTROLLER_H_

#include <U2Core/U2Region.h>
#include <U2Core/U2AnnotationUtils.h>
#include <U2Core/GUrl.h>
#include <U2Gui/SeqPasterWidgetController.h>
#include <U2Misc/DialogUtils.h>

#include <QtGui/QDialog>

class Ui_EditSequenceDialog;

namespace U2{

enum EditSequenceDialogMode {
    EditSequenceMode_Replace,
    EditSequenceMode_Insert
};


struct U2GUI_EXPORT EditSequencDialogConfig {
    EditSequenceDialogMode mode;
    U2Region source;
    DNAAlphabet* alphabet;
    QByteArray initialText;
};

class U2GUI_EXPORT EditSequenceDialogController : public QDialog {
    Q_OBJECT
public:
    EditSequenceDialogController(EditSequencDialogConfig cfg, QWidget *p = NULL);
    ~EditSequenceDialogController();
    virtual void accept();

    DNASequence getNewSequence(){return w->getSequence();};
    GUrl getDocumentPath(); 
    int getPosToInsert(){return pos;};
    U2AnnotationUtils::AnnotationStrategyForResize getAnnotationStrategy();
    bool mergeAnnotations();
    DocumentFormatId getDocumentFormatId();
private slots:
    void sl_browseButtonClicked();
    void sl_indexChanged(int);
    void sl_mergeAnnotationsToggled(bool);

private:
    void addSeqpasterWidget();
    bool modifyCurrentDocument();


    QString filter;
    int pos;
    SeqPasterWidgetController *w;
    EditSequencDialogConfig config;
    Ui_EditSequenceDialog* ui;
};

}//ns

#endif

