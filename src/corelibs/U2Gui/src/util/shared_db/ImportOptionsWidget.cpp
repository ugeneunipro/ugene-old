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

#include "ImportOptionsWidget.h"

namespace U2 {

ImportOptionsWidget::ImportOptionsWidget(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);
}

void ImportOptionsWidget::init(const QString& baseFolder, const ImportToDatabaseOptions& options) {
    leBaseFolder->setText(baseFolder);

    cbRecursively->setChecked(options.processFoldersRecursively);
    cbCreateSubfoledsForFiles->setChecked(options.createSubfolderForEachFile);
    cbImportUnrecognized->setChecked(options.importUnknownAsUdr);
    cbCreateSubfoldersforDocs->setChecked(options.createSubfolderForEachDocument);

    switch (options.multiSequencePolicy) {
        case ImportToDatabaseOptions::SEPARATE:
            rbSeparate->setChecked(true);
            break;
        case ImportToDatabaseOptions::MERGE:
            rbMerge->setChecked(true);
            sbMerge->setValue(options.mergeMultiSequencePolicySeparatorSize);
            break;
        case ImportToDatabaseOptions::MALIGNEMNT:
            rbMalignment->setChecked(true);
            break;
    }
}

QString ImportOptionsWidget::getFolder() const {
    return U2DbiUtils::makeFolderCanonical(leBaseFolder->text());
}

ImportToDatabaseOptions ImportOptionsWidget::getOptions() const {
    ImportToDatabaseOptions options;

    options.processFoldersRecursively = cbRecursively->isChecked();
    options.createSubfolderForEachFile = cbCreateSubfoledsForFiles->isChecked();
    options.importUnknownAsUdr = cbImportUnrecognized->isChecked();
    options.createSubfolderForEachDocument = cbCreateSubfoldersforDocs->isChecked();

    if (rbSeparate->isChecked()) {
        options.multiSequencePolicy = ImportToDatabaseOptions::SEPARATE;
    } else if (rbMerge->isChecked()) {
        options.multiSequencePolicy = ImportToDatabaseOptions::MERGE;
        options.mergeMultiSequencePolicySeparatorSize = sbMerge->value();
    } else if (rbMalignment->isChecked()) {
        options.multiSequencePolicy = ImportToDatabaseOptions::MALIGNEMNT;
    }

    return options;
}

}   // namespace U2
