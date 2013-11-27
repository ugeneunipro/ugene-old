/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#ifndef _U2_TREE_OPTIONS_WIDGET_H_
#define _U2_TREE_OPTIONS_WIDGET_H_

#include <QtGui/QtGui>
#include <U2View/MSAEditor.h>
#include <U2View/TreeSettingsDialog.h>
#include <U2View/TextSettingsDialog.h>
#include <ov_phyltree/GraphicsBranchItem.h>
#include <U2View/MSAEditorTreeViewer.h>
#include "ui/ui_TreeOptionsWidget.h"


namespace U2 {

class TreeViewerUI;
class SimilarityStatisticsSettings;
class ShowHideSubgroupWidget;

struct TreeOpWidgetViewSettings {
    TreeOpWidgetViewSettings() 
        : openGeneralGroup(true), openLabelsGroup(true), openBranchesGroup(true), showFontSettings(false), showPenSettings(false){}
    
    bool openGeneralGroup;
    bool openLabelsGroup;
    bool openBranchesGroup;

    bool showFontSettings;
    bool showPenSettings;
};

class U2VIEW_EXPORT TreeOptionsWidget : public QWidget, private Ui_TreeOptionWidget
{
    Q_OBJECT
public:
    TreeOptionsWidget(TreeViewer* tree, const TreeOpWidgetViewSettings& _viewSettings);
    TreeOptionsWidget(MSAEditor* msa, const TreeOpWidgetViewSettings& _viewSettings);
    ~TreeOptionsWidget();

    const TreeOpWidgetViewSettings& getViewSettings();
signals:
    void saveViewSettings(const TreeOpWidgetViewSettings&);
private slots:
    void sl_onGeneralSettingsChanged();
    void sl_onLabelsSettingsChanged();
    void sl_onLayoutChanged(int index);
    void sl_labelsColorButton();
    void sl_branchesColorButton();
    void sl_textSettingsChanged();
    void sl_branchSettingsChanged();

    void sl_onLblLinkActivated(const QString& link);
    void sl_onSettingsChanged();
private:
    void createGroups();
    void createGeneralSettingsWidgets();
    void updateGeneralSettingsWidgets();
    void updateLabelsSettingsWidgets();
    void updateBranchSettings();
    void updateAllWidgets();
    void connectSlots();

    void updateButtonColor(QPushButton* button, const QColor& newColor);
    void updateShowFontOpLabel(QString newText);
    void updateShowPenOpLabel(QString newText);

    TreeViewerUI* getTreeViewer();

    MSAEditor    *msa;
    TreeViewerUI *treeViewer;

    TreeOpWidgetViewSettings viewSettings;

    TreeSettings* settings;
    TreeSettings treeSettings;
    TreeLabelsSettings labelsSettings;
    TextSettings textSettings;
    BranchSettings branchSettings;

    ShowHideSubgroupWidget* generalOpGroup;
    ShowHideSubgroupWidget* labelsOpGroup;
    ShowHideSubgroupWidget* branchesOpGroup;

    bool showFontSettings;
    bool showPenSettings;

    QWidget* contentWidget;

    static const QString COLOR_BOX_STYLE;
};

class U2VIEW_EXPORT AddTreeWidget : public QWidget
{
    Q_OBJECT
public:
    AddTreeWidget(MSAEditor* msa);
    ~AddTreeWidget(){}

private slots:
    void sl_onOpenTreeTriggered();
    void sl_onBuildTreeTriggered();
private:
    MSAEditor*   editor;

    QPushButton* openTreeButton;
    QPushButton* buildTreeButton;
    QLabel*      addTreeHint;
};

} // namespace

#endif
