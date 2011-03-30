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

#ifndef _U2_ORFDIALOG_H_
#define _U2_ORFDIALOG_H_

#include "ui/ui_ORFDialogUI.h"

#include <U2Core/global.h>
#include <U2Core/U2Region.h>

#include <QtCore/QList>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QCloseEvent>
#include <QtCore/QTimer>

namespace U2 {

class ADVSequenceObjectContext;
class DNASequenceObject;
class ORFFindTask;
class Task;

class ORFDialog : public QDialog, public Ui_ORFDialogBase {
    Q_OBJECT

public:
    ORFDialog(ADVSequenceObjectContext* ctx);

public slots:
    virtual void reject();
    virtual void accept();

protected:
    bool eventFilter(QObject *obj, QEvent *ev);

private slots:
    
    //buttons:
    void sl_onSaveAnnotations();
    void sl_onClearList();
    void sl_onFindAll();
    void sl_onRangeToSelection();
    void sl_onRangeToPanView();
    void sl_onRangeToSequence();

    void sl_onTaskFinished(Task*);
    void sl_onTimer();

    void sl_onResultActivated(QTreeWidgetItem* i, int col);
    void sl_translationChanged();


private:
    void connectGUI();
    void updateState();
    void updateStatus();
    void tunePercentBox();
    void initSettings();
    void saveSettings();
    
    void runTask();

    void importResults();

    U2Region getCompleteSearchRegion() const;

private:
    ADVSequenceObjectContext* ctx;
    DNASequenceObject* sequence;

    U2Region panViewSelection;
    ORFFindTask* task;
    QTimer* timer;
    U2Region initialSelection;
};

}//namespace

#endif
