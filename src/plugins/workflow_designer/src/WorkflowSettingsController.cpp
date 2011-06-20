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

#include "WorkflowSettingsController.h"
#include "WorkflowViewController.h"
#include "WorkflowViewItems.h"

#include <U2Lang/WorkflowSettings.h>
#include <U2Core/AppContext.h>
#include <U2Core/Version.h>

#include <QtGui/QMouseEvent>
#include <QtGui/QFileDialog>
#include <QtGui/QColorDialog>

namespace U2 {

WorkflowSettingsPageController::WorkflowSettingsPageController(QObject* p) 
: AppSettingsGUIPageController(tr("Workflow Designer"), WorkflowSettingsPageId, p) {}


AppSettingsGUIPageState* WorkflowSettingsPageController::getSavedState() {
    WorkflowSettingsPageState* state = new WorkflowSettingsPageState();
    state->showGrid = WorkflowSettings::showGrid();
    state->snap2grid = WorkflowSettings::snap2Grid();
    state->lockRun = WorkflowSettings::monitorRun();
    //state->failFast = WorkflowSettings::failFast();
    state->style = WorkflowSettings::defaultStyle();
    state->font = WorkflowSettings::defaultFont();
    state->path = WorkflowSettings::getUserDirectory();
    state->color = WorkflowSettings::getBGColor();
    state->runSchemaInSeparateProcess = WorkflowSettings::runInSeparateProcess();
    state->externalToolCfgDir = WorkflowSettings::getExternalToolDirectory();
    return state;
}

void WorkflowSettingsPageController::saveState(AppSettingsGUIPageState* s) {
    WorkflowSettingsPageState* state = qobject_cast<WorkflowSettingsPageState*>(s);
    WorkflowSettings::setShowGrid(state->showGrid);
    WorkflowSettings::setSnap2Grid(state->snap2grid);
    WorkflowSettings::setMonitorRun(state->lockRun);
    //WorkflowSettings::setFailFast(state->failFast);
    WorkflowSettings::setDefaultStyle(state->style);
    WorkflowSettings::setDefaultFont(state->font);
    WorkflowSettings::setUserDirectory(state->path);
    WorkflowSettings::setBGColor(state->color);
    WorkflowSettings::setRunInSeparateProcess(state->runSchemaInSeparateProcess);
    WorkflowSettings::setExternalToolDirectory(state->externalToolCfgDir);
}

AppSettingsGUIPageWidget* WorkflowSettingsPageController::createWidget(AppSettingsGUIPageState* state) {
    WorkflowSettingsPageWidget* r = new WorkflowSettingsPageWidget(this);
    r->setState(state);
    return r;
}

WorkflowSettingsPageWidget::WorkflowSettingsPageWidget(WorkflowSettingsPageController* ) {
    setupUi(this);
    styleCombo->addItem(U2::WorkflowView::tr("Minimal"), ItemStyles::SIMPLE);
    styleCombo->addItem(U2::WorkflowView::tr("Extended"), ItemStyles::EXTENDED);
    connect(dirButton, SIGNAL(clicked()), SLOT(sl_getDirectory()));
    connect(colorWidget, SIGNAL(clicked()), SLOT(sl_getColor()));
    connect(extToolDirButton, SIGNAL(clicked()), SLOT(sl_getExternalToolCfgDir()));
    colorWidget->installEventFilter(this);
#ifdef RUN_WORKFLOW_IN_THREADS
    runInSeparateProcessBox->setVisible(false);
#else
    runInSeparateProcessBox->setVisible(Version::ugeneVersion().isDevVersion);
#endif // RUN_WORKFLOW_IN_THREADS
}

void WorkflowSettingsPageWidget::sl_getDirectory() {
    QString url = WorkflowSettings::getUserDirectory();

    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setViewMode(QFileDialog::List);
    dialog.setDirectory(url);
    if(dialog.exec() == QDialog::Accepted) {
        QString dir = dialog.selectedFiles().first();
        dirEdit->setText(dir + "/");
    }
}

void WorkflowSettingsPageWidget::sl_getColor() {
    QColor newColor = QColorDialog::getColor(colorWidget->palette().color(colorWidget->backgroundRole()), this);
    QPalette pal = colorWidget->palette();
    pal.setColor(colorWidget->backgroundRole() , newColor);
    colorWidget->setPalette(pal);
}

bool WorkflowSettingsPageWidget::eventFilter(QObject *, QEvent * event)
{
    if(event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *e= static_cast<QMouseEvent*>(event);
        if(e->button() == Qt::LeftButton) {
            sl_getColor();
        }
    }
    return false;
}

void WorkflowSettingsPageWidget::setState(AppSettingsGUIPageState* s) {
    WorkflowSettingsPageState* state = qobject_cast<WorkflowSettingsPageState*>(s);
    gridBox->setChecked(state->showGrid);
    snapBox->setChecked(state->snap2grid);
    lockBox->setChecked(state->lockRun);
    //failBox->setChecked(state->failFast);
    int idx = styleCombo->findData(state->style);
    if (idx < 0) idx = 1; 
    styleCombo->setCurrentIndex(idx);
    fontCombo->setCurrentFont(state->font);
    dirEdit->setText(state->path);
    colorWidget->setAutoFillBackground(true);
    QPalette pal = colorWidget->palette();
    pal.setColor(colorWidget->backgroundRole(), state->color);
    colorWidget->setPalette(pal);
    runInSeparateProcessBox->setChecked(state->runSchemaInSeparateProcess);
    extToolDirEdit->setText(state->externalToolCfgDir);
}

AppSettingsGUIPageState* WorkflowSettingsPageWidget::getState(QString& ) const {
    WorkflowSettingsPageState* state = new WorkflowSettingsPageState();
    state->showGrid = gridBox->isChecked();
    state->snap2grid = snapBox->isChecked();
    state->lockRun = lockBox->isChecked();
    //state->failFast = failBox->isChecked();
    state->style = styleCombo->itemData(styleCombo->currentIndex()).toString();
    state->font = fontCombo->currentFont();
    state->path = dirEdit->text();
    state->color = colorWidget->palette().color(colorWidget->backgroundRole());
    state->runSchemaInSeparateProcess = runInSeparateProcessBox->isChecked();
    state->externalToolCfgDir = extToolDirEdit->text();
    return state;
}

void WorkflowSettingsPageWidget::sl_getExternalToolCfgDir() {
    QString url = WorkflowSettings::getExternalToolDirectory();

    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setViewMode(QFileDialog::List);
    dialog.setDirectory(url);
    if(dialog.exec() == QDialog::Accepted) {
        QString dir = dialog.selectedFiles().first();
        extToolDirEdit->setText(dir + "/");
    }
}

} //namespace
