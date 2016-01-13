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

#ifndef _U2_DOCUMENT_PROVIDER_SELECTOR_CONTROLLER_H_
#define _U2_DOCUMENT_PROVIDER_SELECTOR_CONTROLLER_H_

#include <U2Core/DocumentUtils.h>

#include "ui/ui_DocumentProviderSelectorDialog.h"

class QRadioButton;
class QToolButton;

namespace U2 {

class DocumentProviderSelectorController : public QDialog, private Ui_DocumentProviderSelectorDialog {
    Q_OBJECT
public:
    static int selectResult(const GUrl& url, const QList<FormatDetectionResult>& results);

private:
    DocumentProviderSelectorController(const QList<FormatDetectionResult>& results, QWidget *parent);
    int getSelectedFormatIdx() const;
    static QString getViewName(const GObjectType &objectType);
    static QString getTypeName(const GObjectType &objectType);

    QList<QRadioButton *> formatsRadioButtons;
    const QList<FormatDetectionResult> formatDetectionResults;
};

}   // namespace U2

#endif // _U2_DOCUMENT_PROVIDER_SELECTOR_CONTROLLER_H_
