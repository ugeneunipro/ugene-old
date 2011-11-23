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

#ifndef _U2_ED_SEARCH_DIALOG_H_
#define _U2_ED_SEARCH_DIALOG_H_

#include "ExpertDiscoveryData.h"
#include "ExpertDiscoveryTask.h"

#include <U2Core/global.h>
#include <U2Core/U2Region.h>
#include <U2Gui/RegionSelector.h>

#include <ui/ui_ExpertDiscoverySearchDialog.h>

#include <QtCore/QList>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QCloseEvent>
#include <QtCore/QTimer>

namespace U2 {

class ADVSequenceObjectContext;
class U2SequenceObject;
class Task;

class ExpertDiscoverySearchDialogController : public QDialog, public Ui_ExpertDiscoverySearchDialog {
    Q_OBJECT
public:
    ExpertDiscoverySearchDialogController(ADVSequenceObjectContext* ctx, ExpertDiscoveryData& data, QWidget *p);
    ~ExpertDiscoverySearchDialogController();

public slots:
    virtual void reject();

protected:
    bool eventFilter(QObject *obj, QEvent *ev);

private slots:
    
    //buttons:
//    void sl_selectModelFile();
    void sl_onSaveAnnotations();
    void sl_onClearList();
    void sl_onSearch();
    void sl_onClose();

    // groups
    void sl_onTaskFinished();
    void sl_onTimer();

    void sl_onResultActivated(QTreeWidgetItem* i, int col);
    
private:
    void connectGUI();
    void updateState();
    void updateStatus();
    //void updateModel(const SiteconModel& m);
    
    bool checkPrevSettings();
    void savePrevSettings();

    void runTask();

    void importResults();

private:
    ADVSequenceObjectContext*   ctx;
    U2Region                     initialSelection;
    
    ExpertDiscoveryData&       edData;

    ExpertDiscoverySearchTask* task;
    QTimer* timer;
    RegionSelector* rs;
};

}//namespace

#endif