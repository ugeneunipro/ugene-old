/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#ifndef _U2_MSA_EDITOR_DATA_LIST_H_
#define _U2_MSA_EDITOR_DATA_LIST_H_


//#include <QtGui/QWidget>
//#include <U2Core/U2Region.h>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QTabWidget>
#include <QtGui/QSplitter>

#include <U2Core/Task.h>
#include <U2Core/MAlignment.h>
#include <U2Core/MAlignmentObject.h>
#include "MSAEditorNameList.h"

namespace U2 
{

class MSADistanceMatrix;
class MAlignmentRow;
class Task;
class MSAWidget;

class UpdatedWidgetSettings {
public:
    UpdatedWidgetSettings() 
        : ma(NULL), ui(NULL), autoUpdate(true) {}
    const MAlignmentObject* ma;
    MSAEditorUI*            ui;
    bool                    autoUpdate;
};
enum DataState {
    DataIsOutdated,
    DataIsValid,
    DataIsBeingUpdated
};
class UpdatedWidgetInterface{
public:
    virtual ~UpdatedWidgetInterface() {}
    virtual void onAlignmentChanged(const MAlignment& maBefore, const MAlignmentModInfo& modInfo) = 0;
    virtual void setSettings(const UpdatedWidgetSettings* settings) = 0;
    virtual QWidget* getWidget() = 0;
    virtual const UpdatedWidgetSettings& getSettings() const = 0;
    virtual void updateWidget() = 0;
};
class SimilarityStatisticsSettings : public UpdatedWidgetSettings {
public:
    SimilarityStatisticsSettings() : usePercents(false), excludeGaps(false){}
    QString                         algoName;// selected algorithm
    bool                            usePercents; 
    bool                            excludeGaps; 
};

class MSAEditorSimilarityColumn : public MSAEditorNameList, public UpdatedWidgetInterface {
    Q_OBJECT
public:
    MSAEditorSimilarityColumn(MSAEditorUI* ui, QScrollBar* nhBar, const SimilarityStatisticsSettings* _settings);
    virtual ~MSAEditorSimilarityColumn();

    void setSettings(const UpdatedWidgetSettings* _settings);
    const UpdatedWidgetSettings& getSettings() const {return curSettings;}
    QWidget* getWidget(){return this;}
    void updateWidget(){updateDistanceMatrix();}

    DataState getState() {return state;}

    void setAlgorithm(MSADistanceMatrix* _algo) {algo = _algo;}
protected:
    QString getTextForRow(int s);
    QString getSeqName(int s);
signals:
    void si_dataStateChanged(DataState newState);
private slots:

    void onAlignmentChanged(const MAlignment& maBefore, const MAlignmentModInfo& modInfo);
    void sl_createMatrixTaskFinished(Task*);
private:
    void sl_buildStaticMenu(GObjectView*, QMenu*) {}
    void sl_buildContextMenu(GObjectView*, QMenu*) {}
    void updateDistanceMatrix();

    MSADistanceMatrix* algo;
    SimilarityStatisticsSettings newSettings;
    SimilarityStatisticsSettings curSettings;

    DataState state;
    bool      autoUpdate;
};


class CreateDistanceMatrixTask : public Task {
    Q_OBJECT
public:
    CreateDistanceMatrixTask(const SimilarityStatisticsSettings& _s);
    
    virtual void prepare();

    MSADistanceMatrix* getResult() const {return resMatrix;}

    QList<Task*> onSubTaskFinished(Task* subTask);
private:
    SimilarityStatisticsSettings   s;
    QString                             resultText;
    MSADistanceMatrix*                  resMatrix;
};



class MSAEditorAlignmentDependentWidget : public QWidget {
    Q_OBJECT
public:
    MSAEditorAlignmentDependentWidget(UpdatedWidgetInterface* _contentWidget);

    void setSettings(const UpdatedWidgetSettings* _settings);
    const DataState& getDataState() const{return state;}
    const UpdatedWidgetSettings* getSettings() const {return settings;} 

private slots:
    void sl_onAlignmentChanged(const MAlignment& maBefore, const MAlignmentModInfo& modInfo);
    void sl_onUpdateButonPressed();
    void sl_onDataStateChanged(DataState newState);
    void sl_onFontChanged(const QFont&);
private:
    void createWidgetUI();
    void createHeaderWidget();

    MSAWidget*                   headerWidget;
    QLabel                       statusBar;
    QLabel                       nameWidget;
    QPushButton                  updateButton;
    UpdatedWidgetInterface*      contentWidget;
    const UpdatedWidgetSettings* settings;
    DataState                    state;
    QWidget*                     updateBar;

    bool                         automaticUpdating;

    static const QString DataIsOutdatedMessage;
    static const QString DataIsValidMessage;
    static const QString DataIsBeingUpdatedMessage;
};
class UpdatedTabWidget : public QTabWidget {
    Q_OBJECT
public:
    UpdatedTabWidget(QWidget* parent);
    virtual ~UpdatedTabWidget() {}
    int addTab(QWidget *page, const QString &label);
    QWidget* getCurrentWidget() {return currentWidget();}
protected slots:
    void sl_refreshTriggered();
    void sl_refreshAllTriggered();
    void sl_closeOtherTabsTriggered();
    void sl_closeAllTabsTriggered();
    void sl_closeTabTriggered();
    void sl_addHSplitterTriggered();
    void sl_addVSplitterTriggered();
    void sl_onTabCloseRequested(int index);
signals:
    void si_onTabCloseRequested(QWidget*);
    void si_addSplitterTriggered(Qt::Orientation splitterOrientation, QWidget* page, const QString &label);
    void si_widgetSelected(UpdatedTabWidget*);
protected:
    void buildMenu();
    bool eventFilter(QObject *target, QEvent *event);
    void updateActionsState();

    QMenu*              tabsMenu;
    QPoint              menuPos;
    QAction*            refreshAction;
    QAction*            refreshAllAction;
    QAction*            closeOtherTabs;
    QAction*            closeAllTabs;
    QAction*            closeTab;
    QAction*            addHorizontalSplitter;
    QAction*            addVerticalSplitter;
};
class TabWidgetArea : public QWidget{
    Q_OBJECT
public:
    TabWidgetArea(QWidget* parent);
    virtual ~TabWidgetArea();

    virtual void addTab(QWidget *page, const QString &label);
    virtual void deleteTab(QWidget *page);

    UpdatedTabWidget* getCurrentTabWidget() {return currentWidget;}
    QWidget*          getCurrentWidget() {return currentWidget->getCurrentWidget();}
protected:
    void paintEvent(QPaintEvent *);
    virtual UpdatedTabWidget* createTabWidget();
    void initialize();

    QList<UpdatedTabWidget*>            tabWidgets;

private slots:
    void sl_onWidgetSelected(UpdatedTabWidget*);
    void sl_addSplitter(Qt::Orientation splitterOrientation, QWidget* page, const QString &label);
    void sl_onTabCloseRequested(QWidget* page);
signals:
    void si_tabsCountChanged(int curTabsNumber);
private:
    int                                 tabsCount;
    QMap<QWidget*, UpdatedTabWidget*>   widgetLocations;
    QList<QSplitter*>                   splitters;
    UpdatedTabWidget*                   currentWidget;
    QLayout*                            currentLayout;
};
class MSAEditorUpdatedTabWidget : public UpdatedTabWidget {
    Q_OBJECT
public:
    MSAEditorUpdatedTabWidget(MSAEditor* _msa, QWidget* parent);
    ~MSAEditorUpdatedTabWidget(){}

    void addExistingTree();
public slots:
    void sl_addTabTriggered();
private:
    MSAEditor* msa;
    QPushButton* addTabButton;
};

class MSAEditorTabWidgetArea : public TabWidgetArea{
    Q_OBJECT
public:
    MSAEditorTabWidgetArea(MSAEditor* msa, QWidget* parent);
    ~MSAEditorTabWidgetArea(){}

    virtual void deleteTab(QWidget *page);
protected:
    virtual UpdatedTabWidget* createTabWidget();
private:
    QPushButton* addTabButton;
    MSAEditor*   msa;
};


} //namespace


#endif
