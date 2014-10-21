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

#ifndef _U2_PRIMERLIBRARYWIDGET_H_
#define _U2_PRIMERLIBRARYWIDGET_H_

#include <QAbstractItemModel>

#include <U2Core/U2OpStatus.h>

#include <U2Gui/MainWindow.h>

#include "Primer.h"

#include "ui_PrimerLibraryWidget.h"

namespace U2 {

class PrimerLibraryModel : public QAbstractItemModel {
public:
    PrimerLibraryModel(QObject *parent);

    /* QAbstractItemModel */
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent) const;

    void addPrimer(Primer &primer, U2OpStatus &os);
    void removePrimer(const QModelIndex &index, U2OpStatus &os);

private:
    QVariant displayData(const QModelIndex &index) const;

private:
    QList<Primer> primers;
};

class PrimerLibraryWidget : public QWidget, Ui::PrimerLibraryWidget {
    Q_OBJECT
public:
    PrimerLibraryWidget(QWidget *parent);

signals:
    void si_close();

private slots:
    void sl_newPrimer();
    void sl_removePrimers();
    void sl_selectionChanged();

private:
    void checkOp(const U2OpStatus &os);

private:
    PrimerLibraryModel *model;
    QPushButton *removePrimersButton;
};

} // U2

#endif // _U2_PRIMERLIBRARYWIDGET_H_
