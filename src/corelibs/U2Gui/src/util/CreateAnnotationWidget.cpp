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

#include <U2Core/U2SafePoints.h>

#include "CreateAnnotationWidget.h"

namespace U2 {

CreateAnnotationWidget::CreateAnnotationWidget(CreateAnnotationWidgetController::AnnotationWidgetMode layoutMode, QWidget *parent) :
    QWidget(parent),
    saveAnnotationsInnerWidget(NULL),
    annotationParametersInnerWidget(NULL),
    saveAnnotationsWidget(NULL),
    annotationParametersWidget(NULL),
    rbExistingTable(NULL),
    rbCreateNewTable(NULL),
    rbUseAutoTable(NULL),
    cbExistingTable(NULL),
    leNewTablePath(NULL),
    tbBrowseExistingTable(NULL),
    tbBrowseNewTable(NULL),
    lblGroupName(NULL),
    lblAnnotationName(NULL),
    lblLocation(NULL),
    leGroupName(NULL),
    leAnnotationName(NULL),
    leLocation(NULL),
    tbSelectGroupName(NULL),
    tbSelectAnnotationName(NULL),
    tbDoComplement(NULL),
    chbUsePatternNames(NULL)
{
    switch (layoutMode) {
    case CreateAnnotationWidgetController::Normal:
        normalUi.setupUi(this);
        initNormalLayout();
        break;
    case CreateAnnotationWidgetController::OptionsPanel:
        optionsPanelUi.setupUi(this);
        initOptionsPanelLayout();
        break;
    default:
        FAIL("Undefined widget layout mode", );
    }
}

void CreateAnnotationWidget::initNormalLayout() {
    saveAnnotationsInnerWidget = normalUi.saveAnnotationsInnerWidget;
    annotationParametersInnerWidget = normalUi.annotationParametersInnerWidget;

    saveAnnotationsWidget = new ShowHideSubgroupWidget("save_params", tr("Save annotation(s) to"), saveAnnotationsInnerWidget, true);
    annotationParametersWidget = new ShowHideSubgroupWidget("annotparams", tr("Annotation parameters"), annotationParametersInnerWidget, true);

    normalUi.mainLayout->insertWidget(0, saveAnnotationsWidget);
    normalUi.mainLayout->insertWidget(1, annotationParametersWidget);

    rbExistingTable = normalUi.rbExistingTable;
    rbCreateNewTable = normalUi.rbCreateNewTable;
    rbUseAutoTable = normalUi.rbUseAutoTable;

    cbExistingTable = normalUi.cbExistingTable;
    leNewTablePath = normalUi.leNewTablePath;

    tbBrowseExistingTable = normalUi.tbBrowseExistingTable;
    tbBrowseNewTable = normalUi.tbBrowseNewTable;

    lblGroupName = normalUi.lblGroupName;
    lblAnnotationName = normalUi.lblAnnotationName;
    lblLocation = normalUi.lblLocation;

    leGroupName = normalUi.leGroupName;
    leAnnotationName = normalUi.leAnnotationName;
    leLocation = normalUi.locationEdit;

    tbSelectGroupName = normalUi.tbSelectGroupName;
    tbSelectAnnotationName = normalUi.tbSelectAnnotationName;
    tbDoComplement = normalUi.tbDoComplement;

    chbUsePatternNames = normalUi.chbUsePatternNames;

    saveAnnotationsWidget->setPermanentlyOpen(true);
    annotationParametersWidget->setPermanentlyOpen(true);
}

void CreateAnnotationWidget::initOptionsPanelLayout() {
    saveAnnotationsInnerWidget = optionsPanelUi.saveAnnotationsInnerWidget;
    annotationParametersInnerWidget = optionsPanelUi.annotationParametersInnerWidget;

    saveAnnotationsWidget = new ShowHideSubgroupWidget("save_params", tr("Save annotation(s) to"), saveAnnotationsInnerWidget, false);
    annotationParametersWidget = new ShowHideSubgroupWidget("annotparams", tr("Annotation parameters"), annotationParametersInnerWidget, false);

    optionsPanelUi.mainLayout->insertWidget(0, saveAnnotationsWidget);
    optionsPanelUi.mainLayout->insertWidget(1, annotationParametersWidget);

    rbExistingTable = optionsPanelUi.rbExistingTable;
    rbCreateNewTable = optionsPanelUi.rbCreateNewTable;
    rbUseAutoTable = optionsPanelUi.rbUseAutoTable;

    cbExistingTable = optionsPanelUi.cbExistingTable;
    leNewTablePath = optionsPanelUi.leNewTablePath;

    tbBrowseExistingTable = optionsPanelUi.tbBrowseExistingTable;
    tbBrowseNewTable = optionsPanelUi.tbBrowseNewTable;

    lblGroupName = optionsPanelUi.lblGroupName;
    lblAnnotationName = optionsPanelUi.lblAnnotationName;
    lblLocation = optionsPanelUi.lblLocation;

    leGroupName = optionsPanelUi.leGroupName;
    leAnnotationName = optionsPanelUi.leAnnotationName;
    leLocation = optionsPanelUi.locationEdit;

    tbSelectGroupName = optionsPanelUi.tbSelectGroupName;
    tbSelectAnnotationName = optionsPanelUi.tbSelectAnnotationName;
    tbDoComplement = optionsPanelUi.tbDoComplement;

    chbUsePatternNames = optionsPanelUi.chbUsePatternNames;
}


}   // namespace U2
