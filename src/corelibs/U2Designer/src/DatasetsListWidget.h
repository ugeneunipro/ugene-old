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

#ifndef _U2_DATASETS_LIST_WIDGET_H_
#define _U2_DATASETS_LIST_WIDGET_H_

#include "DatasetWidget.h"

#include "ui_DatasetsListWidget.h"

namespace U2 {

class DatasetsListWidget : public QWidget, public Ui::DatasetsListWidget {
    Q_OBJECT
public:
    DatasetsListWidget(QWidget *parent = NULL);

    void appendDataset(const QString &name, DatasetWidget *page);

signals:
    void si_addDataset(const QString &name, U2OpStatus &os);

private slots:
    void sl_deleteDataset(int idx);
    void sl_renameDataset(const QString &newName);
    void sl_newDataset();
};

} // U2

#endif // _U2_DATASETS_LIST_WIDGET_H_
