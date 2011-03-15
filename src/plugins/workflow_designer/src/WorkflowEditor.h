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

#ifndef _U2_WORKFLOW_EDITOR_H_
#define _U2_WORKFLOW_EDITOR_H_

#include <U2Lang/ActorModel.h>
#include <U2Lang/Schema.h>
#include <ui/ui_WorkflowEditorWidget.h>

#include <QtGui/QShortcutEvent>

namespace U2 {
using namespace Workflow;
class WorkflowView;
class IterationListWidget;
class ActorCfgModel;

class WorkflowEditor : public QWidget, Ui_WorkflowEditorWidget
{
    Q_OBJECT
public:

    WorkflowEditor(WorkflowView *parent);

    QVariant saveState() const;
    void restoreState(const QVariant&);

    Iteration getCurrentIteration() const;
    void changeScriptMode(bool _mode);

    void setEditable(bool editable);

    bool eventFilter(QObject* object, QEvent* event);

signals:
    void iterationSelected();

public slots:
    void editActor(Actor*);
    void editPort(Port*);
    void setDescriptor(Descriptor* d, const QString& hint = QString());
    void edit(Configuration* subject);
    void selectIteration(int id);
    void reset();
    void commit();
    void resetIterations();
    void commitIterations();
    void sl_resizeSplitter(bool);
    
protected:

    //void commitData(const QString& name, const QVariant& val);

private slots:
    void finishPropertyEditing();
    void updateIterationData();
    void handleDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight);
    void editingLabelFinished();
    void sl_showPropDoc();
    void sl_changeVisibleInput(bool);
    void sl_changeVisibleOutput(bool);
    void sl_showDoc(const QString&);

private:
    void changeSizes(QWidget *w, int h);

private:
    IterationListWidget* iterationList;
    WorkflowView* owner;
    ConfigurationEditor* custom;
    QWidget* customWidget;
    Configuration* subject;
    Actor* actor;
    friend class SuperDelegate;
    ActorCfgModel* actorModel;
    QList<QWidget *> inputPortWidget;
    QList<QWidget *> outputPortWidget;
    int paramHeight, inputHeight, outputHeight;
};


}//namespace

#endif
