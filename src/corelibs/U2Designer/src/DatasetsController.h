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

#ifndef _U2_DATASETS_CONTROLLER_H_
#define _U2_DATASETS_CONTROLLER_H_

#include <U2Core/U2OpStatus.h>

#include <U2Lang/URLAttribute.h>

namespace U2 {

class DatasetsListWidget;
class DatasetWidget;
class UrlItem;

class U2DESIGNER_EXPORT DatasetsController : public QObject {
    Q_OBJECT
public:
    DatasetsController(URLAttribute *attr);
    ~DatasetsController();

    QWidget * getWigdet();

signals:
    void si_attributeChanged();

private:
    URLAttribute *attr;
    QMap<UrlItem*, URLContainer*> urlMap;
    QMap<UrlItem*, Dataset*> itemSetMap;
    QMap<DatasetWidget*, Dataset*> setMap;

    DatasetsListWidget *datasetsWidget;

private slots:
    void sl_itemChanged();
    void sl_replaceUrl(UrlItem *item, int newPos);
    void sl_itemDeleted();
    void sl_renameDataset(const QString &newName, U2OpStatus &os);

    void sl_datasetDeleted();

    void sl_addUrl(const QString &url, U2OpStatus &os);
    void sl_addDataset(const QString &name, U2OpStatus &os);

private:
    DatasetWidget * createDatasetWidget(Dataset &dSet);
    void createItemWidget(URLContainer *url, DatasetWidget *inDataWidget);
    void checkName(const QString &name, U2OpStatus &os);
    void updateAttribute();
};

} // U2

#endif // _U2_DATASETS_CONTROLLER_H_
