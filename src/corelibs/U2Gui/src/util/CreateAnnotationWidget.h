/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#ifndef _U2_CREATE_ANNOTATION_WIDGET_H_
#define _U2_CREATE_ANNOTATION_WIDGET_H_

#include <QWidget>

#include <U2Gui/ShowHideSubgroupWidget.h>

#include "CreateAnnotationWidgetController.h"
#include "ui/ui_CreateAnnotationNormalWidget.h"
#include "ui/ui_CreateAnnotationOptionsPanelWidget.h"

namespace U2 {

class ShowHideSubgroupWidget;

class CreateAnnotationWidget : public QWidget {
    Q_OBJECT
    friend class CreateAnnotationWidgetController;
public:
    CreateAnnotationWidget(CreateAnnotationWidgetController::AnnotationWidgetMode layoutMode, QWidget *parent = NULL);

private:
    void initNormalLayout();
    void initOptionsPanelLayout();

    Ui::CreateAnnotationNormalWidget normalUi;
    Ui::CreateAnnotationOptionsPanelWidget optionsPanelUi;

    QWidget *saveAnnotationsInnerWidget;
    QWidget *annotationParametersInnerWidget;
    ShowHideSubgroupWidget *saveAnnotationsWidget;
    ShowHideSubgroupWidget *annotationParametersWidget;

    QRadioButton *rbExistingTable;
    QRadioButton *rbCreateNewTable;
    QRadioButton *rbUseAutoTable;

    QComboBox *cbExistingTable;
    QLineEdit *leNewTablePath;

    QToolButton *tbBrowseExistingTable;
    QToolButton *tbBrowseNewTable;

    QLabel *lblGroupName;
    QLabel *lblAnnotationName;
    QLabel *lblLocation;

    QLineEdit *leGroupName;
    QLineEdit *leAnnotationName;
    QLineEdit *leLocation;

    QToolButton *tbSelectGroupName;
    QToolButton *tbSelectAnnotationName;
    QToolButton *tbDoComplement;

    QCheckBox *chbUsePatternNames;
};

}   // namespace U2

#endif // _U2_CREATE_ANNOTATION_WIDGET_H_
