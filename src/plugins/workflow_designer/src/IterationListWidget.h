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

#ifndef _U2_WORKFLOW_ITERATION_LIST_H_
#define _U2_WORKFLOW_ITERATION_LIST_H_

#include <U2Lang/Schema.h>
//#include <ui/ui_WorkflowEditorWidget.h>
#include <QtGui/QListView>

namespace U2 {
using namespace Workflow;
class WorkflowView;

class IterationListWidget : public QWidget
{
    Q_OBJECT
public:

    IterationListWidget(QWidget *parent);

    QList<Iteration>& list() const;
    void setList(const QList<Iteration>& lst);
    int current() const;
    QItemSelection selection() const;
    void selectIteration(int id);
    bool expandList(const ActorId& id, const QString& key, const QVariant& val);

signals:
    void iterationListAboutToChange();
    void selectionChanged();
    void listChanged();
    void iteratedChanged();
    
private slots:
    void sl_addIteration();
    void sl_removeIteration();
    void sl_cloneIteration();
    void sl_selectIterations();
    void sl_iterationSelected();
    void sl_updateActions();
    
private:
    void setupIterationUI(QWidget*);
    void updateIterationState();

    QListView* iterationList;
    QAction * cloneAction;
    QAction * removeAction;
};


}//namespace

#endif
