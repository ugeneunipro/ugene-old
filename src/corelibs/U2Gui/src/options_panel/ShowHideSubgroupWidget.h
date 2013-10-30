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

#ifndef _U2_SHOW_HIDE_SUBGROUP_WIDGET_H_
#define _U2_SHOW_HIDE_SUBGROUP_WIDGET_H_

#include <U2Core/global.h>

#include <QtGui/QtGui>


namespace U2 {


class ArrowHeaderWidget;


class U2GUI_EXPORT ShowHideSubgroupWidget : public QWidget
{
    Q_OBJECT
public:
    ShowHideSubgroupWidget(QString id, QString caption, QWidget* innerWidget, bool isOpened);

    bool isSubgroupOpened();

    void showProgress();
    void hideProgress();

    void setPermanentlyOpen(bool isOpened);

signals:
    void si_subgroupStateChanged(QString id);

private:
    ArrowHeaderWidget* arrowHeaderWidget;
    QString subgroupId;
    QWidget* innerWidget;

private slots:
    void updateSubgroupState(bool isSubgroupOpened);
};


class ArrowHeaderWidget : public QWidget
{
    Q_OBJECT

public:
    ArrowHeaderWidget(QString caption, bool isOpened);
    ~ArrowHeaderWidget();

    bool isArrowOpened() { return isOpened; }
    void showProgressWithTimeout();
    void hideProgress();

    void setOpened(bool isOpened);

signals:
    void si_arrowHeaderPressed(bool isSubgroupOpened);

private slots:
    void sl_showProgress();

private:
    bool isOpened;
    QLabel* arrow;

    QLabel* progressMovieLabel;
    QMovie* progressMovie;

    /**
    * Used to provide a small timeout before the progress start
    * Partially prevents blinking of the progress for very quick tasks
    */
    bool canStartProgress;
    static const int TIMEOUT = 300;

    virtual void mousePressEvent(QMouseEvent *);
};


} // namespace

#endif
