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

#ifndef _U2_DATASETS_CONTROLLER_H_
#define _U2_DATASETS_CONTROLLER_H_

#include <U2Core/U2OpStatus.h>

#include <U2Lang/URLAttribute.h>

namespace U2 {

class DatasetsListWidget;
class URLListWidget;
class UrlItem;

/************************************************************************/
/* DatasetsController */
/************************************************************************/
class U2DESIGNER_EXPORT DatasetsController : public QObject {
    Q_OBJECT
public:
    DatasetsController(URLAttribute *attr);
    DatasetsController(QList<Dataset> &sets);
    ~DatasetsController();

    QWidget * getWigdet();
    QList<Dataset> getDatasets();
    void updateAttribute();

    void renameDataset(int dsNum, const QString &newName, U2OpStatus &os);
    void deleteDataset(int dsNum);
    void addDataset(const QString &name, U2OpStatus &os);

signals:
    void si_attributeChanged();

private:
    URLAttribute *attr;
    QList<Dataset*> sets;

    DatasetsListWidget *datasetsWidget;

private:
    void initSets(const QList<Dataset> &sets);
    void initialize();
    URLListWidget * createDatasetWidget(Dataset *dSet);
    void checkName(const QString &name, U2OpStatus &os, Dataset *exception = NULL);
};

/************************************************************************/
/* URLListController */
/************************************************************************/
class URLListController : public QObject {
public:
    URLListController(DatasetsController *parent, Dataset *set);

    URLListWidget * getWidget();
    void addUrl(const QString &url, U2OpStatus &os);
    void replaceUrl(int pos, int newPos);
    void deleteUrl(int pos);
    void changedUrl(UrlItem *item);

private:
    URLListWidget *widget;
    DatasetsController *controller;
    Dataset *set;
    QMap<UrlItem*, URLContainer*> urlMap;

private:
    void addItemWidget(URLContainer *url);
    void createWidget();
    URLContainer * getUrl(int pos);
};

} // U2

#endif // _U2_DATASETS_CONTROLLER_H_
