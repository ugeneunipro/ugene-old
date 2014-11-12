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

#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QToolButton>

#include <U2Core/AppContext.h>
#include <U2Core/DocumentImport.h>
#include <U2Core/L10n.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/ObjectViewModel.h>

#include <U2View/AssemblyBrowserFactory.h>
#include <U2View/MSAEditorFactory.h>

#include "DocumentProviderSelectorController.h"

namespace U2 {

int DocumentProviderSelectorController::selectResult(const GUrl& url, const QList<FormatDetectionResult> &results) {
    SAFE_POINT(!results.isEmpty(), "Results list is empty!", -1);
    if (results.size() == 1) {
        return 0;
    }

    DocumentProviderSelectorController d(results, QApplication::activeModalWidget());
    d.gbFormats->setTitle(tr("Open '%1' with").arg(url.fileName()));
    d.buttonBox->button(QDialogButtonBox::Cancel)->setAutoDefault(false);
    d.buttonBox->button(QDialogButtonBox::Ok)->setAutoDefault(true);
    d.buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);

    QVBoxLayout *formatsLayout = qobject_cast<QVBoxLayout *>(d.gbFormats->layout());
    for (int i = 0; i < results.size(); i++) {
        const FormatDetectionResult &result = results[i];
        QString text;
        if (result.format != NULL) {
            QList<GObjectType> supportedTypes = result.format->getSupportedObjectTypes().toList();
            if (supportedTypes.isEmpty()) {
                continue;
            }
            text = tr("Open in <b>%1</b> with the <b>%2</b> format.").arg(getViewName(supportedTypes.first())).arg(result.format->getFormatName());
        } else if (result.importer != NULL) {
            QList<GObjectType> supportedTypes = result.importer->getSupportedObjectTypes().toList();
            if (supportedTypes.isEmpty()) {
                continue;
            }
            text = tr("Open in <b>%1</b> with the <b>%2</b> format.").arg(getViewName(supportedTypes.first())).arg(result.importer->getImporterName());
        } else {
            assert(0);
            continue;
        }

        QHBoxLayout *blockLayout = new QHBoxLayout();

        QRadioButton *rbFormat = new QRadioButton();
        rbFormat->setChecked(i == 0);
        d.formatsRadioButtons << rbFormat;

        QLabel *lblDescription = new QLabel(text);
        lblDescription->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        lblDescription->setSizePolicy(QSizePolicy::Expanding, lblDescription->sizePolicy().verticalPolicy());

        QToolButton* tbMore = new QToolButton();
        tbMore->setText(tr("more.."));
        tbMore->setEnabled(!result.getFormatDescriptionText().isEmpty());

        d.moreInfoButtons << tbMore;
        QObject::connect(tbMore, SIGNAL(clicked()), &d, SLOT(sl_moreFormatInfo()));

        blockLayout->addWidget(rbFormat);
        blockLayout->addWidget(lblDescription);
        blockLayout->addWidget(tbMore);
        formatsLayout->addLayout(blockLayout);
    }

    int rc = d.exec();
    if (rc == QDialog::Rejected) {
        return -1;
    }

    return d.getSelectedFormatIdx();
}

void DocumentProviderSelectorController::sl_moreFormatInfo() {
    QToolButton* tb = qobject_cast<QToolButton*>(sender());
    SAFE_POINT(tb != NULL, "Failed to derive selected format info!", );
    int idx = moreInfoButtons.indexOf(tb);
    const FormatDetectionResult& dr = formatDetectionResults[idx];
    QMessageBox::information(this, tr("Format details for '%1' format").arg(dr.getFormatOrImporterName()), dr.getFormatDescriptionText());
}

DocumentProviderSelectorController::DocumentProviderSelectorController(const QList<FormatDetectionResult> &formatDetectionResults, QWidget *parent) :
    QDialog(parent),
    formatDetectionResults(formatDetectionResults)
{
    setupUi(this);
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

}   // namespace U2
