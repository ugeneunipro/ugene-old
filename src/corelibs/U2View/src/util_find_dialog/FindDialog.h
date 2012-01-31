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

#ifndef _U2_FINDDIALOG_H_
#define _U2_FINDDIALOG_H_

#include <U2Core/global.h>
#include <U2Core/U2Region.h>
#include <U2Gui/RegionSelector.h>

#include <ui/ui_FindDialogUI.h>

#include <QtCore/QList>
#include <QtGui/QListWidget>
#include <QtGui/QListWidgetItem>
#include <QtGui/QCloseEvent>
#include <QtCore/QTimer>

namespace U2 {

class ADVSequenceObjectContext;
class U2SequenceObject;
class FindAlgorithmTask;
class Task;
class FRListItem;

class U2VIEW_EXPORT FindDialog : public QDialog, public Ui_FindDialogBase {
    Q_OBJECT
    friend class FindAlgorithmTask;

public:
    static bool runDialog(ADVSequenceObjectContext* ctx);

public slots:
    virtual void reject();

protected:
    bool eventFilter(QObject *obj, QEvent *ev);

private:
    FindDialog(ADVSequenceObjectContext* c);
        
private slots:
    
    //buttons:
    void sl_onSaveAnnotations();
    void sl_onClearList();
    void sl_onFindNext();
    void sl_onFindAll();
    void sl_onClose();

    //line ed
    void sl_onSearchPatternChanged(const QString&);

    // groups
    void sl_onSequenceTypeChanged();
    void sl_onStrandChanged();
    void sl_onAlgorithmChanged();

    //spin box
    void sl_onMatchPercentChanged(int);

    void sl_onTaskFinished(Task*);
    void sl_onTimer();

    //range
    void sl_onRegionChanged(const U2Region& r);
    void sl_onCurrentPosChanged(int);

    void sl_onResultActivated(QListWidgetItem* i, bool setPos = true);
    void sl_currentResultChanged(QListWidgetItem*, QListWidgetItem*);

    void sl_onRemoveOverlaps();

private:
    void connectGUI();
    void updateState();
    void updateStatus();
    void tunePercentBox();
    
    bool checkState(bool forSingleShot);
    bool checkPrevSettings();
    void savePrevSettings();

    void runTask(bool singleShot);

    void importResults();

    int getMaxErr() const;
    U2Region getCompleteSearchRegion() const;

private:
    ADVSequenceObjectContext*   ctx;

    QString prevSearchString;
    int prevAlgorithm; //0 - 100%, 1-mismatch, 2-insdel
    int prevMatch;
    FindAlgorithmTask* task;
    QTimer* timer;
    U2Region initialSelection;

    RegionSelector* rs;
};

}//namespace

#endif
