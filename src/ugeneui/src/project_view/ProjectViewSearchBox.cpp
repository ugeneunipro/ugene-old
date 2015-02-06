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
#include <QMovie>
#include <QPicture>
#include <QStyle>

#include "ProjectViewSearchBox.h"

static const QString LABEL_STYLE_SHEET = "border: 0px; padding: 0px;";

namespace U2 {

ProjectViewSearchBox::ProjectViewSearchBox(QWidget *p)
    : QLineEdit(p), progressLabel(new QLabel(this)), progressMovie(new QMovie(":/core/images/progress.gif", QByteArray(), progressLabel)),
    searchIconLabel(new QLabel(this)), firstShow(true)
{
    setObjectName("nameFilterEdit");

    progressLabel->setStyleSheet(LABEL_STYLE_SHEET);
    progressLabel->setMovie(progressMovie);
    progressMovie->start();

    searchIconLabel->setStyleSheet(LABEL_STYLE_SHEET);
    searchIconLabel->setPixmap(QPixmap(":/core/images/zoom_whole.png"));

    const int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    const QSize progressLabelSize = progressLabel->sizeHint();
    const QSize iconLabelSize = searchIconLabel->sizeHint();
    const QSize minimumWidgetSize = minimumSizeHint();

    setStyleSheet(QString("QLineEdit {padding-right: %1px; padding-left: %2px}").arg(progressLabelSize.width() + frameWidth + 1)
        .arg(iconLabelSize.width() + frameWidth + 1));
    setMinimumSize(qMax(minimumWidgetSize.width(), iconLabelSize.width() + progressLabelSize.width() + frameWidth * 2 + 2),
        qMax(minimumWidgetSize.height(), progressLabelSize.height() + frameWidth * 2 + 2));

    setPlaceholderText(tr("Search..."));
}

void ProjectViewSearchBox::sl_filteringStarted() {
    progressLabel->setVisible(true);
    progressMovie->start();
    updateProgressLabelPosition();
}

void ProjectViewSearchBox::sl_filteringFinished() {
    progressMovie->stop();
    progressLabel->setVisible(false);
}

void ProjectViewSearchBox::paintEvent(QPaintEvent *event) {
    if (firstShow) {
        firstShow = false;
        sl_filteringFinished();
    }
    QLineEdit::paintEvent(event);
}

void ProjectViewSearchBox::updateProgressLabelPosition() {
    const QSize progressLabelSize = progressLabel->sizeHint();
    const QSize iconLabelSize = searchIconLabel->sizeHint();
    const int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    const QRect widgetRect = rect();

    progressLabel->move(widgetRect.right() - 2 * frameWidth - progressLabelSize.width(),
        (widgetRect.bottom() - progressLabelSize.height() + 1) / 2);
    searchIconLabel->move(widgetRect.left() + 2 * frameWidth, (widgetRect.bottom() - iconLabelSize.height() + 1) / 2);
}

void ProjectViewSearchBox::resizeEvent(QResizeEvent *event) {
    updateProgressLabelPosition();
    QLineEdit::resizeEvent(event);
}

} // namespace U2
