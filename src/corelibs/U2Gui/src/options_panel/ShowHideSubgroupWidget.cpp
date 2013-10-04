/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "ShowHideSubgroupWidget.h"

#include <U2Core/U2SafePoints.h>


namespace U2 {

ShowHideSubgroupWidget::ShowHideSubgroupWidget(
    QString _id, QString caption, QWidget* _innerWidget, bool isOpened)
    : subgroupId(_id),
      innerWidget(_innerWidget)
{
    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 10, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->setAlignment(Qt::AlignTop);

    innerWidget->setContentsMargins(17, 5, 5, 5);

    arrowHeaderWidget = new ArrowHeaderWidget(caption, isOpened);
    connect(arrowHeaderWidget, SIGNAL(si_arrowHeaderPressed(bool)),
        this, SLOT(updateSubgroupState(bool)));
    updateSubgroupState(isOpened);

    mainLayout->addWidget(arrowHeaderWidget);
    mainLayout->addWidget(innerWidget);

    setLayout(mainLayout);
}


void ShowHideSubgroupWidget::updateSubgroupState(bool isSubgroupOpened)
{
    if (isSubgroupOpened)
    {
        innerWidget->show();
    }
    else
    {
        innerWidget->hide();
    }

    emit si_subgroupStateChanged(subgroupId);
}


bool ShowHideSubgroupWidget::isSubgroupOpened()
{
    SAFE_POINT(0 != arrowHeaderWidget, "The arrow header widget hasn't been created, but it is used.", false);

    return arrowHeaderWidget->isArrowOpened();
}


void ShowHideSubgroupWidget::showProgress()
{
    arrowHeaderWidget->showProgressWithTimeout();
}


void ShowHideSubgroupWidget::hideProgress()
{
    arrowHeaderWidget->hideProgress();
}


ArrowHeaderWidget::ArrowHeaderWidget(QString caption, bool _isOpened)
    : isOpened(_isOpened)
{
    QHBoxLayout* arrowHeaderLayout = new QHBoxLayout();
    arrowHeaderLayout->setContentsMargins(0, 0, 0, 0);
    arrowHeaderLayout->setMargin(0);
    arrowHeaderLayout->setAlignment(Qt::AlignTop);

    arrow = new QLabel();
    if (isOpened) {
        arrow->setPixmap(QPixmap(":core/images/arrow_down.png"));
    } else {
        arrow->setPixmap(QPixmap(":core/images/arrow_right.png"));
    }

    arrow->setMaximumSize(10, 10);

    QLabel* captionLabel = new QLabel(caption);
    captionLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    progressMovieLabel = new QLabel();
    progressMovie = new QMovie(":/core/images/progress.gif");
    progressMovieLabel->setMovie(progressMovie);

    if (progressMovie->isValid()){
        progressMovie->start();
        progressMovie->setPaused(true);
    }

    arrowHeaderLayout->addWidget(arrow);
    arrowHeaderLayout->addWidget(captionLabel);
    arrowHeaderLayout->addWidget(progressMovieLabel);

    progressMovieLabel->hide();
    canStartProgress = false;

    setLayout(arrowHeaderLayout);
}


ArrowHeaderWidget::~ArrowHeaderWidget()
{
    delete progressMovie;
}

void ArrowHeaderWidget::showProgressWithTimeout()
{
    QTimer* timeoutToStartProgress = new QTimer(this);
    connect(timeoutToStartProgress, SIGNAL(timeout()), SLOT(sl_showProgress()));
    timeoutToStartProgress->start(TIMEOUT);
    canStartProgress = true;
}


void ArrowHeaderWidget::sl_showProgress()
{
    if (canStartProgress) {
        progressMovie->setPaused(false);
        progressMovieLabel->show();
    }
}


void ArrowHeaderWidget::hideProgress()
{
    canStartProgress = false;
    progressMovieLabel->hide();
    progressMovie->setPaused(true);
}


void ArrowHeaderWidget::mousePressEvent(QMouseEvent * /* event */)
{
    if (isOpened) {
        arrow->setPixmap(QPixmap(":core/images/arrow_right.png"));
        isOpened = false;
    } else {
        arrow->setPixmap(QPixmap(":core/images/arrow_down.png"));
        isOpened = true;
    }

    emit si_arrowHeaderPressed(isOpened);
}

} // namespace
