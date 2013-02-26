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

#ifndef _U2_TREE_OPTIONS_WIDGET_H_
#define _U2_TREE_OPTIONS_WIDGET_H_

#include <QtGui/QtGui>
#include <U2View/MSAEditor.h>
#include <U2View/TreeSettingsDialog.h>
#include <U2View/TextSettingsDialog.h>
#include <ov_phyltree/GraphicsBranchItem.h>
#include <U2View/MSAEditorTreeViewer.h>

namespace U2 {
class TreeViewerUI;

class SimilarityStatisticsSettings;

class U2VIEW_EXPORT TreeOptionsWidget : public QWidget
{
    Q_OBJECT
public:
    TreeOptionsWidget(TreeViewer* tree);
    TreeOptionsWidget(MSAEditor* msa);
private slots:
    void sl_onSettingsChanged();
    void sl_onLabelsSettingsChanged();
    void sl_onLayoutChanged(int index);
    void sl_colorButton();
    void sl_branchColorButton();
    void sl_textSettingsChanged();
    void sl_branchSettingsChanged();
    void sl_syncSettingsChanged();
private:
    void createGroups();
    QWidget* createTreesSettings();
    QWidget* createLabelsSettings();
    QWidget* createBranchSettings();
    QWidget* createLabelsFormatSettings();
    QWidget* createButtonSettings();
    QWidget* createTreeSyncSettings();

    void updateButtonColor(QPushButton* button, const QColor& newColor);

    void updateVisualSettings();
    void updateSyncSettings();
    TreeViewerUI* getTreeViewer();

    MSAEditor    *msa;
    TreeViewerUI *treeViewer;

    QComboBox *layoutCombo;

    QSlider *widthSlider;
    QSlider *heightSlider;
    QComboBox *treeViewCombo;

    QCheckBox *showNamesCheck;
    QCheckBox *showDistancesCheck;
    QCheckBox *alignLabelsCheck;

    TreeSettings* settings;
    TreeSettings newSettings;
    TreeLabelsSettings labelsSettings;
    TextSettings textSettings;
    BranchSettings branchSettings;
    TreeSynchronizationSettings syncSettings;

    QPushButton *colorButton;
    QSpinBox *sizeSpinBox;
    QFontComboBox *fontComboBox;
    QToolButton *boldToolButton;
    QToolButton *italicToolButton;
    QToolButton *underlineToolButton;
    QToolButton *overlineToolButton;

    QPushButton *branchColorButton;
    QSpinBox *thicknessSpinBox;

    QComboBox *currentTree;
    QCheckBox *syncSizesCheck;
    QCheckBox *sortSeqByTreeCheck;
    QCheckBox *syncCollapsingCheck;
};
} // namespace

#endif