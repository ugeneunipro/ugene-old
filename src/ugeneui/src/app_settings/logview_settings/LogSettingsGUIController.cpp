#include "LogSettingsGUIController.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>

#include <QtGui/QHeaderView>
#include <QtGui/QToolButton>
#include <QtGui/QColorDialog>

namespace U2 {

LogSettingsPageState::LogSettingsPageState()
{
}

LogSettingsPageController::LogSettingsPageController(LogSettingsHolder* h, QObject* p) 
: AppSettingsGUIPageController(tr("Logging"), APP_SETTINGS_GUI_LOG, p), target(h)
{
}


AppSettingsGUIPageState* LogSettingsPageController::getSavedState() {
    LogSettingsPageState* state = new LogSettingsPageState();
    state->settings = target->getSettings();
    return state;
}

void LogSettingsPageController::saveState(AppSettingsGUIPageState* s) {
    LogSettingsPageState* state = qobject_cast<LogSettingsPageState*>(s);
    target->setSettings(state->settings);
}

AppSettingsGUIPageWidget* LogSettingsPageController::createWidget(AppSettingsGUIPageState* data) {
    LogSettingsPageWidget* w = new LogSettingsPageWidget();
    w->setState(data);
    return w;
}

//////////////////////////////////////////////////////////////////////////
// widget

LogSettingsPageWidget::LogSettingsPageWidget() {
    setupUi( this );
    tableWidget->verticalHeader()->setVisible(false);
    connect(tableWidget, SIGNAL(currentCellChanged(int, int, int, int)), SLOT(sl_currentCellChanged(int, int, int, int)));
}

void LogSettingsPageWidget::setState(AppSettingsGUIPageState* s) {
    LogSettingsPageState* state = qobject_cast<LogSettingsPageState*>(s);
    LogSettings& settings = state->settings;
    
    tableWidget->clearContents();
    
    tableWidget->setRowCount(1 + settings.getLoggerSettings().size());
    
    // set header
    tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Category")));
    for (int i=0; i< LogLevel_NumLevels; i++) {
        QString catName = LogCategories::getLocalizedLevelName((LogLevel) i);
        tableWidget->setHorizontalHeaderItem(i + 1, new QTableWidgetItem(catName) );
    }

    // create global line
    QTableWidgetItem* item00 = new QTableWidgetItem(tr("<<all>>"));
    tableWidget->setItem(0, 0, item00);
    for (int i=0; i<LogLevel_NumLevels; i++) {
        QTableWidgetItem* levelItem = new QTableWidgetItem();
        tableWidget->setItem(0, i+1, levelItem);

        LogSettingsTopLineWidget* tw = new LogSettingsTopLineWidget(this, settings.levelColors[i], LogLevel(i));
        QHBoxLayout* l = new QHBoxLayout();

        int marginLeft = 6; //TODO: align with usual setCheckState boxes
        l->setContentsMargins(marginLeft, 1, 10, 1);
        l->setSpacing(0);
        
        tw->cb = new QCheckBox(tw);
        tw->cb->setTristate(true);        
        connect(tw->cb, SIGNAL(stateChanged (int)), SLOT(sl_levelStateChanged(int)));
        l->addWidget(tw->cb);

        QLabel* label = new QLabel(tw);
        updateColorLabel(label, settings.levelColors[i]);
        connect(label, SIGNAL(linkActivated(const QString&)), SLOT(sl_changeColor(const QString&)));
        l->addWidget(label);

        tw->setLayout(l);
        tableWidget->setCellWidget(0, i+1, tw);
    }

    // create category lines
    int row=1;
    int nEqual[LogLevel_NumLevels];
    for (int i=0; i<LogLevel_NumLevels; i++) {
        nEqual[i] = 0;
    }

    QList<QString> categoryNames = settings.getLoggerSettings().keys();
    qSort(categoryNames);    
    foreach(const QString& categoryName, categoryNames) {
        const LoggerSettings& cs = settings.getLoggerSettings(categoryName);
        QTableWidgetItem* nameItem = new QTableWidgetItem(cs.categoryName);        
        tableWidget->setItem(row, 0, nameItem);
        for (int i=0; i<LogLevel_NumLevels; i++) {
            QTableWidgetItem* catItem = new QTableWidgetItem();
            catItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            catItem->setCheckState(cs.activeLevelFlag[i] ? Qt::Checked : Qt::Unchecked);
            nEqual[i] += cs.activeLevelFlag[i] ? 1 : 0;
            tableWidget->setItem(row, i+1, catItem);
        }
        row++;
    }
    
    for (int i=0; i<LogLevel_NumLevels; i++) {
        LogSettingsTopLineWidget* tw = qobject_cast<LogSettingsTopLineWidget*>(tableWidget->cellWidget(0, i+1));
        tw->cb->setCheckState(nEqual[i] == 0 ? Qt::Unchecked : nEqual[i] == settings.getLoggerSettings().size() ? Qt::Checked : Qt::PartiallyChecked);
    }    

    tableWidget->resizeRowsToContents();
    connect(tableWidget, SIGNAL(itemChanged(QTableWidgetItem *)), this, SLOT(sl_catItemStateChanged(QTableWidgetItem *)));

    showCategoryCB->setChecked(settings.showCategory);
    showDateCB->setChecked(settings.showDate);
    showLevelCB->setChecked(settings.showLevel);
    colorCB->setChecked(settings.enableColor);
    dateFormatEdit->setText(settings.logPattern);
}

AppSettingsGUIPageState* LogSettingsPageWidget::getState(QString& err) const {
    Q_UNUSED(err);
    LogSettingsPageState* state = new LogSettingsPageState();
    LogSettings& settings = state->settings;
    
    //process global settings
    for (int i = 0; i < LogLevel_NumLevels; i++) {
        QWidget* w = tableWidget->cellWidget(0, i+1);
        LogSettingsTopLineWidget* tw = qobject_cast<LogSettingsTopLineWidget*>(w);
        settings.levelColors[i] = tw->color;
        settings.activeLevelGlobalFlag[i] = tw->cb->checkState() != Qt::Unchecked;
    }

    //process per-category settings
    for (int row=1 ; row < tableWidget->rowCount(); row++) {
        LoggerSettings logCat;
        logCat.categoryName  = tableWidget->item(row, 0)->text();
        for (int i = 0; i < LogLevel_NumLevels; i++) {
            logCat.activeLevelFlag[i]= tableWidget->item(row, i+1)->checkState() == Qt::Checked;
        }
        settings.addCategory(logCat);
    }

    settings.showCategory = showCategoryCB->isChecked();
    settings.showDate = showDateCB->isChecked();
    settings.showLevel = showLevelCB->isChecked();
    settings.enableColor = colorCB->isChecked();
    settings.logPattern = dateFormatEdit->text();
    
    return state;
}

void LogSettingsPageWidget::sl_currentCellChanged ( int currentRow, int currentColumn, int previousRow, int previousColumn) {
    Q_UNUSED(currentRow); Q_UNUSED(currentColumn); Q_UNUSED(previousRow); Q_UNUSED(previousColumn);
    tableWidget->setCurrentCell(0, 0, QItemSelectionModel::NoUpdate);
}

void LogSettingsPageWidget::sl_levelStateChanged(int state) {
    if (state == Qt::PartiallyChecked) {
        return;
    }
    disconnect(tableWidget, SIGNAL(itemChanged(QTableWidgetItem *)), this, SLOT(sl_catItemStateChanged(QTableWidgetItem *)));

    QCheckBox* cb = qobject_cast<QCheckBox*>(sender());
    int column = 1 + qobject_cast<LogSettingsTopLineWidget*>(cb->parent())->level;
    assert(column >= 1 && column <=LogLevel_NumLevels);    
    
    for (int row = 1; row < tableWidget->rowCount(); row++) {
        QTableWidgetItem* catItem = tableWidget->item(row, column);
        catItem->setCheckState(cb->checkState());
    }
    
    connect(tableWidget, SIGNAL(itemChanged(QTableWidgetItem *)), this, SLOT(sl_catItemStateChanged(QTableWidgetItem *)));
}

void LogSettingsPageWidget::sl_catItemStateChanged(QTableWidgetItem *item) {
    if (tableWidget->row(item) == 0) {
        return;
    }

    int column = tableWidget->column(item);    
    assert(column >= 1 && column <=LogLevel_NumLevels);
    
    LogSettingsTopLineWidget* mainItem = qobject_cast<LogSettingsTopLineWidget*>(tableWidget->cellWidget(0, column));

    int nEqual = 0;
    for (int row = 1; row < tableWidget->rowCount(); row++) {
        QTableWidgetItem* catItem = tableWidget->item(row, column);
        assert(catItem!=NULL);        
        nEqual+=catItem->checkState() == item->checkState() ? 1 : 0;
    }
    if (nEqual == tableWidget->rowCount() - 1) {
        mainItem->cb->setCheckState(item->checkState());
    } else {
        mainItem->cb->setCheckState(Qt::PartiallyChecked);
    }
}

void LogSettingsPageWidget::sl_changeColor(const QString& v) {
    Q_UNUSED(v);
    QLabel* label = qobject_cast<QLabel*>(sender());
    LogSettingsTopLineWidget* tw = qobject_cast<LogSettingsTopLineWidget*>(label->parent());
    QColor color = QColorDialog::getColor(tw->color, this);
    if (color.isValid()) {
        tw->color = color.name();
        updateColorLabel(label, tw->color);
    }
}

void LogSettingsPageWidget::updateColorLabel(QLabel* l, const QString& color) {
    QString style = "{color: "+color+"; text-decoration: none;};";
    l->setText("<style> a "+style+" a:visited "+style+" a:hover "+style+"</style>"
                + "<a href=\".\">" + tr("Sample text") + "</a>");
}

} //namespace
