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
    return state;
}

} //namespace
