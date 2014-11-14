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

#include <QApplication>

#include "api/GTRadioButton.h"
#include "api/GTWidget.h"

#include "DocumentProviderSelectorDialogFiller.h"

namespace U2 {

QMap<DocumentProviderSelectorDialogFiller::OpenWith, QString> DocumentProviderSelectorDialogFiller::initMap(){
    QMap<DocumentProviderSelectorDialogFiller::OpenWith, QString> result;
    result.insert(DocumentProviderSelectorDialogFiller::AlignmentEditor,
                  "Open in <b>Alignment editor</b> with the <b>ACE</b> format._radio");
    result.insert(DocumentProviderSelectorDialogFiller::AssemblyBrowser,
                  "Open in <b>Assembly Browser</b> with the <b>ACE file importer</b> format._radio");
    return result;
}
const QMap<DocumentProviderSelectorDialogFiller::OpenWith, QString> DocumentProviderSelectorDialogFiller::openWithMap =
        DocumentProviderSelectorDialogFiller::initMap();

#define GT_CLASS_NAME "DocumentFormatSelectorDialogFiller"
#define GT_METHOD_NAME "commonScenario"
void DocumentProviderSelectorDialogFiller::commonScenario(){
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QRadioButton* radio = GTWidget::findExactWidget<QRadioButton*>(os, openWithMap.value(openWith), dialog);
    GTWidget::click(os, radio);

    GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
