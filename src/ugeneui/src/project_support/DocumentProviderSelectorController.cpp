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

#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QToolButton>

#include <U2Core/AppContext.h>
#include <U2Core/DocumentImport.h>
#include <U2Core/L10n.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/HelpButton.h>
#include <U2Gui/ObjectViewModel.h>
#include <U2Gui/QObjectScopedPointer.h>

#include <U2View/AssemblyBrowserFactory.h>
#include <U2View/MSAEditorFactory.h>

#include "DocumentProviderSelectorController.h"

namespace U2 {

int DocumentProviderSelectorController::selectResult(const GUrl& url, const QList<FormatDetectionResult> &results) {
    SAFE_POINT(!results.isEmpty(), "Results list is empty!", -1);
    if (results.size() == 1) {
        return 0;
    }

    QObjectScopedPointer<DocumentProviderSelectorController> d = new DocumentProviderSelectorController(results, QApplication::activeModalWidget());
    d->gbFormats->setTitle(tr("Open '%1' as").arg(url.fileName()));
    d->buttonBox->button(QDialogButtonBox::Cancel)->setAutoDefault(false);
    d->buttonBox->button(QDialogButtonBox::Cancel)->setDefault(false);
    d->buttonBox->button(QDialogButtonBox::Ok)->setAutoDefault(true);
    d->buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);

    for (int i = 0; i < results.size(); i++) {
        const FormatDetectionResult &result = results[i];
        QList<GObjectType> supportedTypes;
        QString text;
        if (result.format != NULL) {
            supportedTypes = result.format->getSupportedObjectTypes().toList();
        } else if (result.importer != NULL) {
            supportedTypes = result.importer->getSupportedObjectTypes().toList();
        } else {
            assert(0);
            continue;
        }

        if (supportedTypes.isEmpty()) {
            continue;
        }
        text = tr("%1 in the %2").arg(getTypeName(supportedTypes.first())).arg(getViewName(supportedTypes.first()));

        QRadioButton *rbFormat = new QRadioButton(text);
        rbFormat->setObjectName(text + "_radio");
        rbFormat->setChecked(i == 0);
        d->formatsRadioButtons << rbFormat;

        d->formatsLayout->addWidget(rbFormat);
    }
    d->adjustSize();

    CHECK(!d->formatsRadioButtons.isEmpty(), -1);
    d->formatsRadioButtons[0]->setFocus();

    const int rc = d->exec();
    CHECK(!d.isNull(), -1);

    if (rc == QDialog::Rejected) {
        return -1;
    }

    return d->getSelectedFormatIdx();
}

DocumentProviderSelectorController::DocumentProviderSelectorController(const QList<FormatDetectionResult> &formatDetectionResults, QWidget *parent) :
    QDialog(parent),
    formatDetectionResults(formatDetectionResults)
{
    setupUi(this);
    new HelpButton(this, buttonBox, "16122301");
}

int DocumentProviderSelectorController::getSelectedFormatIdx() const {
    for (int i = 0; i < formatsRadioButtons.size(); i++) {
        if (formatsRadioButtons[i]->isChecked()) {
            return i;
        }
    }
    return 0;
}

QString DocumentProviderSelectorController::getViewName(const GObjectType &objectType) {
    GObjectViewFactoryRegistry *objectViewFactoriesRegistry = AppContext::getObjectViewFactoryRegistry();
    SAFE_POINT(NULL != objectViewFactoriesRegistry, L10N::nullPointerError("Object View Factories Registry"), "");

    QString id;
    if (GObjectTypes::ASSEMBLY == objectType) {
        id = AssemblyBrowserFactory::ID;
    } else if (GObjectTypes::MULTIPLE_ALIGNMENT == objectType) {
        id = MSAEditorFactory::ID;
    }

    GObjectViewFactory *factory = objectViewFactoriesRegistry->getFactoryById(id);
    SAFE_POINT(NULL != factory, L10N::nullPointerError("GObject View Factory"), "");
    return factory->getName();
}

QString DocumentProviderSelectorController::getTypeName(const GObjectType &objectType) {
    if (GObjectTypes::ASSEMBLY == objectType) {
        return "Short reads assembly";
    } else if (GObjectTypes::MULTIPLE_ALIGNMENT == objectType) {
        return "Multiple sequence alignment";
    }
    FAIL("An unexpected type", "");
}

}   // namespace U2
