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

#include <QDesktopServices>
#include <QFile>
#include <QScrollBar>

#include <U2Core/Version.h>

#include "StatisticalReportController.h"

namespace U2 {

StatisticalReportController::StatisticalReportController(const QString &newHtmlFilepath) : QDialog() {
    setupUi(this);

    lblStat->setText(tr("<b>Optional:</b> Help make UGENE better by automatically sending anonymous usage statistics."));

    Version v = Version::appVersion();
    setWindowTitle(tr("Welcome to UGENE %1.%2").arg(v.major).arg(v.minor));
    QFile file(newHtmlFilepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        newsTextBrowser->setText(tr("Error loading release new from file"));
        return;
    }

    QString htmlText;
    while (!file.atEnd()) {
        htmlText += file.readLine();
    }

    newsTextBrowser->setText(htmlText);
    connect(newsTextBrowser, SIGNAL(anchorClicked(const QUrl &)), SLOT(sl_onAnchorClicked(const QUrl &)));
}

bool StatisticalReportController::isInfoSharingAccepted() const {
    return chkStat->isChecked();
}

void StatisticalReportController::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);
    QScrollBar *vBar = newsTextBrowser->verticalScrollBar();
    if(vBar->maximum() == vBar->value()){
        return;
    }
    //adjust QTextEditor size
    while(vBar->maximum() != vBar->value()){
        newsTextBrowser->setMinimumHeight(newsTextBrowser->size().height() + 1);
    }
    newsTextBrowser->setMinimumHeight(newsTextBrowser->size().height() + 10);
}

void StatisticalReportController::sl_onAnchorClicked(const QUrl &url) {
    QDesktopServices::openUrl(url);
}

}
