/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_DOCUMENT_FORMAT_SELECTOR_CONTROLLER_
#define _U2_DOCUMENT_FORMAT_SELECTOR_CONTROLLER_

#include <ui/ui_DocumentFormatSelectorDialog.h>

#include <U2Core/AppContext.h>
#include <U2Core/DocumentUtils.h>

#include <QtGui/QRadioButton>
#include <QtGui/QToolButton>

namespace U2 {

class DocumentFormatSelectorController: public QDialog, public Ui_DocumentFormatSelectorDialog {
	Q_OBJECT

    DocumentFormatSelectorController(const QList<FormatDetectionResult>& results, QWidget *p);

public:
    static int selectResult(const GUrl& url, const QByteArray& rawData, const QList<FormatDetectionResult>& results);
    static QString score2Text(int score);

private:
    int getSelectedFormatIdx() const;

private slots:
    void sl_moreFormatInfo();

private:
    QList<QRadioButton*>    radioButtons;
    QList<QToolButton*>     moreButtons;
    const QList<FormatDetectionResult>& formatDetectionResults;
};

} //namespace

#endif
