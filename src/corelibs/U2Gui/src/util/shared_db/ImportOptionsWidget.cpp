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
#include "ui/ui_ImportOptionsWidget.h"

namespace U2 {

ImportOptionsWidget::ImportOptionsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImportOptionsWidget)
{
    ui->setupUi(this);
}

ImportOptionsWidget::~ImportOptionsWidget() {
    delete ui;
}

void ImportOptionsWidget::init(const QString& baseFolder, const ImportToDatabaseOptions& options) {
    ui->leBaseFolder->setText(baseFolder);

    ui->cbRecursively->setChecked(options.processFoldersRecursively);
    ui->cbCreateSubfoldersForFiles->setChecked(options.createSubfolderForEachFile);
    ui->cbImportUnrecognized->setChecked(options.importUnknownAsUdr);
    ui->cbCreateSubfoldersforDocs->setChecked(options.createSubfolderForEachDocument);

    switch (options.multiSequencePolicy) {
        case ImportToDatabaseOptions::SEPARATE:
            ui->rbSeparate->setChecked(true);
            break;
        case ImportToDatabaseOptions::MERGE:
            ui->rbMerge->setChecked(true);
            ui->sbMerge->setValue(options.mergeMultiSequencePolicySeparatorSize);
            break;
        case ImportToDatabaseOptions::MALIGNMENT:
            ui->rbMalignment->setChecked(true);
            break;
    }
}

QString ImportOptionsWidget::getFolder() const {
    return U2DbiUtils::makeFolderCanonical(ui->leBaseFolder->text());
}

ImportToDatabaseOptions ImportOptionsWidget::getOptions() const {
    ImportToDatabaseOptions options;

    options.processFoldersRecursively = ui->cbRecursively->isChecked();
    options.createSubfolderForEachFile = ui->cbCreateSubfoldersForFiles->isChecked();
    options.importUnknownAsUdr = ui->cbImportUnrecognized->isChecked();
    options.createSubfolderForEachDocument = ui->cbCreateSubfoldersforDocs->isChecked();

    if (ui->rbSeparate->isChecked()) {
        options.multiSequencePolicy = ImportToDatabaseOptions::SEPARATE;
    } else if (ui->rbMerge->isChecked()) {
        options.multiSequencePolicy = ImportToDatabaseOptions::MERGE;
        options.mergeMultiSequencePolicySeparatorSize = ui->sbMerge->value();
    } else if (ui->rbMalignment->isChecked()) {
        options.multiSequencePolicy = ImportToDatabaseOptions::MALIGNMENT;
    }

    return options;
}

}   // namespace U2
