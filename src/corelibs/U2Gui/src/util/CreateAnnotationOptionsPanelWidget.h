/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#ifndef _U2_CREATE_ANNOTATION_OPTIONS_PANEL_WIDGET_H_
#define _U2_CREATE_ANNOTATION_OPTIONS_PANEL_WIDGET_H_

#include "CreateAnnotationWidget.h"
#include "ui_CreateAnnotationOptionsPanelWidget.h"

namespace U2 {

class ShowHideSubgroupWidget;

class CreateAnnotationOptionsPanelWidget : public CreateAnnotationWidget, private Ui_CreateAnnotationOptionsPanelWidget {
    Q_OBJECT
public:
    CreateAnnotationOptionsPanelWidget(QWidget *parent = NULL);

    void setLocationVisible(bool visible);
    void setAnnotationParametersVisible(bool visible);
    void setAnnotationTypeVisible(bool visible);
    void setAnnotationNameVisible(bool visible);
    void setAutoTableOptionVisible(bool visible);
    void setDescriptionVisible(bool visible);
    void setUsePatternNamesVisible(bool visible);

    void setAnnotationNameEnabled(bool enable);

    void useAminoAnnotationTypes(bool useAmino);

    void focusGroupName();
    void focusAnnotationType();
    void focusAnnotationName();
    void focusLocation();

    void setNewTablePath(const QString &path);
    void setGroupName(const QString &name);
    void setAnnotationType(U2FeatureType type);
    void setAnnotationName(const QString &name);
    void setLocation(const U2Location &location);

    QString getAnnotationTypeString() const;
    QString getGroupName() const;
    QString getAnnotationName() const;
    QString getLocationString() const;
    QString getDescription() const;
    bool isUsePatternNamesChecked() const;

    bool isExistingTablesListEmpty() const;

    void selectNewTableOption();
    void selectAutoTableOption();

    void setExistingTableOptionEnable(bool enable);

    bool isNewTableOptionSelected() const;
    bool isExistingTableOptionSelected() const;
    bool isAutoTableOptionSelected() const;

    void showSelectGroupMenu(QMenu &menu);
    GObjectComboBoxController * createGObjectComboBoxController(const GObjectComboBoxControllerConstraints &constraints);

    void countDescriptionUsage() const;
    void fillSaveDocumentControllerConfig(SaveDocumentControllerConfig &config) const;

private:
    void initLayout();
    void init();
    void connectSignals();

    ShowHideSubgroupWidget *annotationParametersWidget;
};

}   // namespace U2

#endif // _U2_CREATE_ANNOTATION_OPTIONS_PANEL_WIDGET_H_
