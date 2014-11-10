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

#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Gui/U2WidgetStateStorage.h>

#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/ADVSequenceWidget.h>
#include <U2View/AnnotatedDNAView.h>

#include "InSilicoPcrOptionPanelWidget.h"
#include "InSilicoPcrProductsTable.h"

#include "PcrOptionsPanelSavableTab.h"

static const QString PCR_PRODUCTS_TABLE_NAME = "productsTable";

typedef QPair<U2::ADVSequenceObjectContext *, QList<U2::InSilicoPcrProduct> > AdvContextPcrProductPair;

Q_DECLARE_METATYPE(AdvContextPcrProductPair)

namespace U2 {

PcrOptionsPanelSavableTab::PcrOptionsPanelSavableTab(QWidget *wrappedWidget, MWMDIWindow *contextWindow)
    : U2SavableWidget(wrappedWidget, contextWindow), originalWrappedWidget(qobject_cast<InSilicoPcrOptionPanelWidget *>(wrappedWidget))
{
    SAFE_POINT(NULL != originalWrappedWidget, "Invalid input widget", );
}

PcrOptionsPanelSavableTab::~PcrOptionsPanelSavableTab() {
    U2WidgetStateStorage::saveWidgetState(*this);
    widgetStateSaved = true;
}

QVariant PcrOptionsPanelSavableTab::getChildValue(const QString &childId) const {
    InSilicoPcrProductsTable *productTable = qobject_cast<InSilicoPcrProductsTable *>(getChildWidgetById(childId));

    if (NULL != productTable) {
        const QList<InSilicoPcrProduct> &products = qobject_cast<InSilicoPcrProductsTable *>(productTable)->getAllProducts();
        AdvContextPcrProductPair data(productTable->getCurrentSequenceContext(), products);
        return QVariant::fromValue<AdvContextPcrProductPair>(data);
    } else {
        return U2SavableWidget::getChildValue(childId);
    }
}

void PcrOptionsPanelSavableTab::setChildValue(const QString &childId, const QVariant &value) {
    InSilicoPcrProductsTable *productTable = qobject_cast<InSilicoPcrProductsTable *>(getChildWidgetById(childId));

    if (NULL != productTable) {
        originalWrappedWidget->setResultTableShown(true);

        const AdvContextPcrProductPair data = value.value<AdvContextPcrProductPair>();
        productTable->showProducts(data.second, data.first);
    } else {
        return U2SavableWidget::setChildValue(childId, value);
    }
}

bool PcrOptionsPanelSavableTab::childValueIsAcceptable(const QString &childId, const QVariant &value) const {
    if (PCR_PRODUCTS_TABLE_NAME == childId) {
        const AdvContextPcrProductPair data = value.value<AdvContextPcrProductPair>();

        AnnotatedDNAView *dnaView = originalWrappedWidget->getDnaView();
        SAFE_POINT(NULL != dnaView, "Invalid sequence view detected", false);

        return dnaView->getSequenceContexts().contains(data.first);
    } else {
        return U2SavableWidget::childValueIsAcceptable(childId, value);
    }
}

} // namespace U2
