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

#include "WorkflowEditor.h"
#include "WorkflowViewController.h"
#include "IterationListWidget.h"
#include "WorkflowEditorDelegates.h"

#include "ActorCfgModel.h"

#include <U2Core/Settings.h>
#include <U2Core/Log.h>
#include <U2Lang/WorkflowUtils.h>
#include <U2Lang/MapDatatypeEditor.h>

#include <QtGui/QAction>
#include <QtGui/QHeaderView>


#define MAIN_SPLITTER "main.splitter"
#define TAB_SPLITTER "tab.splitter"

namespace U2 {

WorkflowEditor::WorkflowEditor(WorkflowView *p) 
: QWidget(p), owner(p), custom(NULL), customWidget(NULL), subject(NULL), actor(NULL)
{
    setupUi(this);

    inputPortWidget << table2;
    outputPortWidget << table3;
    inputPortBox->setEnabled(false);
    outputPortBox->setEnabled(false);
    inputPortBox->setVisible(true);
    outputPortBox->setVisible(true);

    caption->setMinimumHeight(nameEdit->sizeHint().height());
    //doc->setMaximumHeight(height()/4);

    iterationList = new IterationListWidget(this);
    iterationBox->layout()->addWidget(iterationList);
    connect(iterationBox, SIGNAL(toggled(bool)), iterationList, SLOT(setVisible(bool)));
    connect(iterationBox, SIGNAL(toggled(bool)), SLOT(sl_resizeSplitter(bool)));
    iterationBox->setChecked(false);
    //iterationBox->setMaximumHeight(height()/4);

    actorModel = new ActorCfgModel(this, iterationList->list());
    table->setModel(actorModel);
    table->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
    
    table->horizontalHeader()->setStretchLastSection(true);
    //table->horizontalHeader()->setStretchLastSection(false);
    //table->horizontalHeader()->setResizeMode(0, QHeaderView::ResizeToContents);
    //table->horizontalHeader()->setResizeMode(1, QHeaderView::Interactive);
    //table->horizontalHeader()->setResizeMode(2, QHeaderView::Stretch);
    
    table->horizontalHeader()->setClickable(false);
    table->verticalHeader()->hide();
    table->verticalHeader()->setDefaultSectionSize(QFontMetrics(QFont()).height() + 6);
    table->setItemDelegate(new SuperDelegate(this));
    //table->setMinimumHeight(height()/2);

    reset();

    doc->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    propDoc->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    doc->installEventFilter(this);
    
    connect(iterationList, SIGNAL(iterationListAboutToChange()), SLOT(finishPropertyEditing()));
    connect(iterationList, SIGNAL(selectionChanged()), SLOT(updateIterationData()));
    connect(iterationList, SIGNAL(listChanged()), SLOT(commitIterations()));
    connect(iterationList, SIGNAL(iteratedChanged()), SLOT(sl_iteratedChanged()));
    connect(iterationList, SIGNAL(selectionChanged()), SIGNAL(iterationSelected()));

    connect(nameEdit, SIGNAL(editingFinished()), SLOT(editingLabelFinished()));

    connect(table->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), SLOT(sl_showPropDoc()));
    connect(table->model(), SIGNAL(dataChanged(QModelIndex, QModelIndex)), SLOT(handleDataChanged(QModelIndex, QModelIndex)));
    
    // FIXME
    //connect(doc, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(sl_contextMenuForDoc(const QPoint &)));
}

void WorkflowEditor::setEditable(bool editable) {
    table->setDisabled(!editable);
    iterationList->setDisabled(!editable);
    foreach(QWidget* w, inputPortWidget) { w->setDisabled(!editable); }
    foreach(QWidget* w, outputPortWidget) { w->setDisabled(!editable); }
}

void WorkflowEditor::sl_resizeSplitter(bool b) {
    QWidget *w = qobject_cast<QWidget*>(sender());
    int ind = splitter->indexOf(w);
    if(ind != -1) {
        if(!b) {
            splitter->setStretchFactor(ind, 0);
            QList<int> sizes = splitter->sizes();
            sizes[ind] = 0;
            splitter->setSizes(sizes);
        } else {
            if(paramBox == w) {
               changeSizes(paramBox, paramHeight);
            } else {
                int h = w->minimumHeight();
                QList<int> sizes = splitter->sizes();
                sizes[ind] = h;
                sizes[splitter->indexOf(propDoc)] -= h;
                splitter->setSizes(sizes);
            }
        }
    }
}

void WorkflowEditor::changeSizes(QWidget *w, int h) {
    int ind = splitter->indexOf(w);
    if(ind == -1) {
        return;
    } else {
        QList<int> sizes = splitter->sizes();
        sizes[ind] = h;
        sizes[splitter->indexOf(propDoc)] -= h/2;
        sizes[splitter->indexOf(doc)] -= h/2;
        splitter->setSizes(sizes);
    }
}

void WorkflowEditor::handleDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight) {
    if (topLeft == bottomRight) {
        commitIterations();
    }
}

void WorkflowEditor::changeScriptMode(bool _mode) {
    if(table->currentIndex().column() == 2) {
        table->clearSelection();
        table->setCurrentIndex(QModelIndex());
    }
    actorModel->changeScriptMode(_mode);
    //table->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    table->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
    table->horizontalHeader()->setStretchLastSection(true);
    if(_mode) {
        int tWidth = table->width();
        table->setColumnWidth(0, tWidth/3 - 2);
        table->setColumnWidth(1, tWidth/3 - 2);
        table->setColumnWidth(2, tWidth/3 - 2);
    }
}

void WorkflowEditor::selectIteration(int id) {
    iterationList->selectIteration(id);
}

Iteration WorkflowEditor::getCurrentIteration() const {
    return iterationList->list().at(iterationList->current());
}

void WorkflowEditor::updateIterationData() {
    if (sender()) {
        finishPropertyEditing();
    }
    actorModel->setIterations(iterationList->list());
    actorModel->selectIteration(iterationList->current());
}

void WorkflowEditor::sl_showDoc(const QString& str) {
    propDoc->setText(str);
}

void WorkflowEditor::sl_showPropDoc() {
    QModelIndex current = table->selectionModel()->currentIndex();
    if (current.isValid()) {
        propDoc->setText(WorkflowUtils::getRichDoc(current.data(DescriptorRole).value<Descriptor>()));
    } else {
        propDoc->setText("");
    }
}

void WorkflowEditor::editingLabelFinished() {
    QString newLabel = nameEdit->text();
    if( !newLabel.isEmpty() && newLabel != actor->getLabel() ) {
        actor->setLabel(newLabel);
        owner->getScene()->setModified(true);
        owner->refreshView();
    }
}

void WorkflowEditor::reset() {
    caption->setText("");
    nameEdit->hide();
    paramBox->setTitle(tr("Parameters"));
    setDescriptor(NULL);
    edit(NULL);
    actor = NULL;
    actorModel->setActor(NULL);
    propDoc->setText("");
    inputPortBox->setEnabled(false);
    outputPortBox->setEnabled(false);
    inputPortBox->setVisible(true);
    outputPortBox->setVisible(true);
    paramBox->setEnabled(false);

    QList<int> sizes = splitter->sizes();
    int splitterHeight = splitter->height();
    int indDoc = splitter->indexOf(doc);
    int indPropDoc = splitter->indexOf(propDoc);
    int ind = splitter->indexOf(inputPortBox);
    splitter->setStretchFactor(ind, 0);
    sizes[ind] = 0;
    ind = splitter->indexOf(outputPortBox);
    splitter->setStretchFactor(ind, 0);
    sizes[ind] = 0;
    ind = splitter->indexOf(paramBox);
    splitter->setStretchFactor(ind, 0);
    sizes[ind] = 0;
    ind = splitter->indexOf(iterationBox);
    splitter->setStretchFactor(ind, 0);
    sizes[ind] = 0;

    sizes[indDoc] = splitterHeight/2;
    splitter->setStretchFactor(indDoc, 1);
    sizes[indPropDoc] = splitterHeight/2;
    splitter->setStretchFactor(indPropDoc, 1);
    splitter->setSizes(sizes);

    paramHeight = 0;
    inputHeight = 0;
    outputHeight = 0;
}

void WorkflowEditor::resetIterations() {
    //disconnect(iterationList, SIGNAL(listChanged()), this, SLOT(commitIterations()));
    iterationList->setList(owner->getScene()->getIterations());
    //connect(iterationList, SIGNAL(listChanged()), SLOT(commitIterations()));
}

void WorkflowEditor::commitIterations() {
    uiLog.trace("committing iterations data");
    owner->getScene()->setIterations(iterationList->list());
}

void WorkflowEditor::sl_iteratedChanged() {
    owner->getScene()->setIterated(true);
    owner->sl_updateUi();
}

void WorkflowEditor::finishPropertyEditing() {
    //table->setCurrentCell(0,0, QItemSelectionModel::NoUpdate);
    table->setCurrentIndex(QModelIndex()/*table->model()->index(0, 0, QModelIndex())*/);
}

void WorkflowEditor::commit() {
    finishPropertyEditing();
    //commitIterations();
}

void WorkflowEditor::editActor(Actor* a) {
    reset();
    actor = a;
    if (a) {
        caption->setText(tr("Element name:"));
        nameEdit->setText(a->getLabel());
        nameEdit->show();
        setDescriptor(a->getProto(), tr("To configure the parameters of the element go to \"Parameters\" area below."));
        edit(a);

        paramHeight = table->rowHeight(0) * (table->model()->rowCount() + 3);
        paramBox->setTitle(tr("Parameters"));
        if(paramBox->isChecked()) {
            changeSizes(paramBox, paramHeight);
        }

        if(!a->getInputPorts().isEmpty()) {
            inputPortBox->setVisible(true);
            inputHeight = 0;
            foreach(Port *p, a->getInputPorts()) {
                BusPortEditor* ed = new BusPortEditor(qobject_cast<IntegralBusPort*>(p));
                ed->setParent(p);
                p->setEditor(ed);
                QWidget *w = ed->getWidget();
                inputHeight += ed->getOptimalHeight(); 
                inputLayout->addWidget(w);
                w->setVisible(inputPortBox->isChecked());
                inputPortBox->setEnabled(true);
                inputPortBox->setVisible(true);
                
                connect(ed, SIGNAL(si_showDoc(const QString&)), SLOT(sl_showDoc(const QString&)));
                inputPortWidget << w;
            }
            connect(inputPortBox, SIGNAL(toggled(bool)), SLOT(sl_changeVisibleInput(bool)));

            if(inputPortBox->isChecked()) {
                changeSizes(inputPortBox, inputHeight);
            }
        } else {
            inputPortBox->setEnabled(false);
            inputPortBox->setVisible(false);
            inputPortBox->resize(0,0);
        }

        if(!a->getOutputPorts().isEmpty()) {
            outputPortBox->setVisible(true);
            outputHeight = 0;
            foreach(Port *p, a->getOutputPorts()) {
                BusPortEditor* ed = new BusPortEditor(qobject_cast<IntegralBusPort*>(p));
                ed->setParent(p);
                p->setEditor(ed);
                QWidget *w = ed->getWidget();
                outputHeight += ed->getOptimalHeight();
                outputLayout->addWidget(w);
                w->setVisible(outputPortBox->isChecked());
                outputPortBox->setEnabled(true);
                outputPortBox->setVisible(true);
                
                connect(ed, SIGNAL(si_showDoc(const QString&)), SLOT(sl_showDoc(const QString&)));
                outputPortWidget << w;
            }
            connect(outputPortBox, SIGNAL(toggled(bool)), SLOT(sl_changeVisibleOutput(bool)));
            if(outputPortBox->isChecked()) {
                changeSizes(outputPortBox, outputHeight);
            }
        } else {
            outputPortBox->setEnabled(false);
            outputPortBox->setVisible(false);
            outputPortBox->resize(0,0);
        }
    }
}

void WorkflowEditor::sl_changeVisibleInput(bool isChecked) {
    if(inputPortWidget.isEmpty()) {
        return;
    }
    foreach(QWidget *w, inputPortWidget)  {
        w->setVisible(isChecked);
    }
    if(!isChecked) {
        int ind = splitter->indexOf(inputPortBox);
        QList<int> sizes = splitter->sizes();
        splitter->setStretchFactor(ind, 0);
        //splitterSizes[0] += splitterSizes[ind];
        sizes[ind] = 0;
        splitter->setSizes(sizes);
    } else {
        changeSizes(inputPortBox, inputHeight);
    }
}

void WorkflowEditor::sl_changeVisibleOutput(bool isChecked) {
    if(outputPortWidget.isEmpty()) {
        return;
    }
    foreach(QWidget *w, outputPortWidget)  {
        w->setVisible(isChecked);
    }
    if(!isChecked) {
        int ind = splitter->indexOf(outputPortBox);
        QList<int> sizes = splitter->sizes();
        splitter->setStretchFactor(ind, 0);
        //splitterSizes[0] += splitterSizes[ind];
        sizes[ind] = 0;
        splitter->setSizes(sizes);
    } else {
        changeSizes(outputPortBox, outputHeight);
    }
}

void WorkflowEditor::editPort(Port* p) {
    reset();
    if (p) {
        //caption->setText(formatPortCaption(p));
        QString portDoc = tr("<b>%1 \"%2\"</b> of task \"%3\":<br>%4<br><br>%5")
            .arg(p->isOutput() ? tr("Output port") : tr("Input port"))
            .arg(p->getDisplayName())
            .arg(p->owner()->getLabel())
            .arg(p->getDocumentation())
            .arg(tr("You can observe data slots of the port and configure connections if any in the \"Parameters\" widget suited below."));
        doc->setText(portDoc);

        inputPortBox->setEnabled(false);
        outputPortBox->setEnabled(false);
        inputPortBox->setVisible(false);
        outputPortBox->setVisible(false);

        BusPortEditor* ed = new BusPortEditor(qobject_cast<IntegralBusPort*>(p));
        ed->setParent(p);
        p->setEditor(ed);
        paramHeight = ed->getOptimalHeight();

        edit(p);
        if(paramBox->isChecked()) {
            changeSizes(paramBox, paramHeight);
        }

        if(p->isInput()) {
            paramBox->setTitle(tr("Input data"));
        } else {
            paramBox->setTitle(tr("Output data"));
        }
    }
}

void WorkflowEditor::setDescriptor(Descriptor* d, const QString& hint) {
    QString text = d ? WorkflowUtils::getRichDoc(*d) + "<br><br>" + hint : hint;
    if (text.isEmpty()) {
        text = tr("Select an element to inspect.");
    }
    doc->setText(text);
}

void WorkflowEditor::edit(Configuration* cfg) {
    paramBox->setEnabled(true);
    disconnect(paramBox, SIGNAL(toggled(bool)), tableSplitter, SLOT(setVisible(bool)));

    if (customWidget) {
        custom->commit();
        customWidget->hide();
        paramBox->layout()->removeWidget(customWidget);
        customWidget->disconnect();
        delete customWidget;
    }
    foreach(QWidget *w, inputPortWidget) {
        w->hide();
        inputLayout->removeWidget(w);
        //w = NULL;
        w->disconnect();
    }
    inputPortWidget.clear();

    foreach(QWidget *w, outputPortWidget) {
        w->hide();
        inputLayout->removeWidget(w);
        //w = NULL;
        w->disconnect();
    }
    outputPortWidget.clear();

    subject = cfg;
    custom = cfg ? cfg->getEditor() : NULL;
    customWidget = custom ? custom->getWidget() : NULL;

    if(customWidget) {
        connect(paramBox, SIGNAL(toggled(bool)), customWidget, SLOT(setVisible(bool)));
        connect(paramBox, SIGNAL(toggled(bool)), SLOT(sl_resizeSplitter(bool)));
    }
    
    //int h = 0;
    if (subject && !customWidget) {
        assert(actor);
        actorModel->setActor(actor);
        updateIterationData();
        tableSplitter->setVisible(paramBox->isChecked());
        /*if(paramBox->isChecked()) {
            h = table->sizeHint().height();
            
        }*/
        connect(paramBox, SIGNAL(toggled(bool)), tableSplitter, SLOT(setVisible(bool)));
        connect(paramBox, SIGNAL(toggled(bool)), SLOT(sl_resizeSplitter(bool)));
    } else {
        tableSplitter->hide();
        if (customWidget) {
            paramBox->layout()->addWidget(customWidget);
            customWidget->setVisible(paramBox->isChecked());
            /*if(paramBox->isChecked()) {
                h = customWidget->minimumSizeHint().height();
            }*/
        }
    }
}

QVariant WorkflowEditor::saveState() const {
    QVariantMap m;
    m.insert(MAIN_SPLITTER, splitter->saveState());
    m.insert(TAB_SPLITTER, tableSplitter->saveState());
    return m;
}

void WorkflowEditor::restoreState(const QVariant& v) {
    QVariantMap m = v.toMap();
    splitter->restoreState(m.value(MAIN_SPLITTER).toByteArray());
    tableSplitter->restoreState(m.value(TAB_SPLITTER).toByteArray());
}

bool WorkflowEditor::eventFilter(QObject* object, QEvent* event) {
    if (event->type() == QEvent::Shortcut || 
        event->type() == QEvent::ShortcutOverride)
    {
        if (object == doc)
        {
            event->accept();
            return true;
        }
    }
    return false;
}

void WorkflowEditor::setIterated(bool iterated) {
    iterationBox->setVisible(iterated);
}

void WorkflowEditor::sl_linkActivated(const QString& url) {
    const QString& id = WorkflowUtils::getParamIdFromHref(url);
    
    QModelIndex modelIndex = actorModel->modelIndexById(id);
    QModelIndex prev = table->selectionModel()->currentIndex();
    if (modelIndex==prev) {
        table->selectionModel()->reset();
    }
    table->setCurrentIndex(modelIndex);
}

}//namespace
