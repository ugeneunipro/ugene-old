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

#include <U2Core/U2SafePoints.h>

#include <U2Lang/URLContainer.h>

#include "DatasetsListWidget.h"
#include "DirectoryItem.h"
#include "FileItem.h"
#include "DatasetWidget.h"
#include "UrlItem.h"

#include "DatasetsController.h"

namespace U2 {

/************************************************************************/
/* Utilities */
/************************************************************************/
class ItemWidgetCreator : public URLContainerVisitor {
public:
    ItemWidgetCreator() : urlItem(NULL) {}

    virtual void visit(FileUrlContainer *url) {
        urlItem = new FileItem(url->getUrl());
    }

    virtual void visit(DirUrlContainer *url) {
        DirectoryItem *dItem = new DirectoryItem(url->getUrl());
        dItem->setIncludeFilter(url->getIncludeFilter());
        dItem->setExcludeFilter(url->getExcludeFilter());
        dItem->setRecursive(url->isRecursive());
        urlItem = dItem;
    }

    UrlItem * getWidget() {
        return urlItem;
    }

private:
    UrlItem *urlItem;
};

class URLContainerUpdateHelper : public UrlItemVisitor {
public:
    URLContainerUpdateHelper(FileUrlContainer *url)
        : fileUrl(url), dirUrl(NULL) {}
    URLContainerUpdateHelper(DirUrlContainer *url)
        : fileUrl(NULL), dirUrl(url) {}

    virtual void visit(DirectoryItem *item) {
        SAFE_POINT(NULL != dirUrl, "NULL directory url", );
        dirUrl->setIncludeFilter(item->getIncludeFilter());
        dirUrl->setExcludeFilter(item->getExcludeFilter());
        dirUrl->setRecursive(item->isRecursive());
    }

    virtual void visit(FileItem * /*item*/) {
    }

private:
    FileUrlContainer *fileUrl;
    DirUrlContainer *dirUrl;
};

class URLContainerUpdater : public URLContainerVisitor {
public:
    URLContainerUpdater(UrlItem *_item)
        : item(_item) {}

    virtual void visit(FileUrlContainer *url) {
        URLContainerUpdateHelper helper(url);
        item->accept(&helper);
    }

    virtual void visit(DirUrlContainer *url) {
        URLContainerUpdateHelper helper(url);
        item->accept(&helper);
    }

private:
    UrlItem *item;
};

/************************************************************************/
/* Controller */
/************************************************************************/
DatasetsController::DatasetsController(URLAttribute *_attr)
: QObject(), attr(_attr), datasetsWidget(NULL)
{
    initSets(attr->getDatasets());
    initialize();
}

DatasetsController::DatasetsController(QList<Dataset> &_sets)
: QObject(), attr(NULL), datasetsWidget(NULL)
{
    initSets(_sets);
    initialize();
}

void DatasetsController::initSets(const QList<Dataset> &s) {
    foreach (const Dataset &d, s) {
        sets << new Dataset(d);
    }
}

void DatasetsController::initialize() {
    SAFE_POINT(sets.size() > 0, "0 datasets count", );
    datasetsWidget = new DatasetsListWidget();

    foreach (Dataset *dSet, sets) {
        datasetsWidget->appendDataset(dSet->getName(), createDatasetWidget(dSet));
    }
    connect(datasetsWidget, SIGNAL(si_addDataset(const QString &, U2OpStatus &)),
        SLOT(sl_addDataset(const QString &, U2OpStatus &)));
    connect(datasetsWidget, SIGNAL(si_renameDataset(int, const QString &, U2OpStatus &)),
        SLOT(sl_renameDataset(int, const QString &, U2OpStatus &)));
    connect(datasetsWidget, SIGNAL(si_deleteDataset(int)), SLOT(sl_deleteDataset(int)));
}

DatasetsController::~DatasetsController() {
    datasetsWidget->setParent(NULL);
    delete datasetsWidget;
    qDeleteAll(sets);
    sets.clear();
}

QWidget * DatasetsController::getWigdet() {
    return datasetsWidget;
}

QList<Dataset> DatasetsController::getDatasets() {
    QList<Dataset> result;
    foreach (Dataset *d, sets) {
        result << Dataset(*d);
    }
    return result;
}

void DatasetsController::createItemWidget(URLContainer *url, DatasetWidget *inDataWidget) {
    ItemWidgetCreator wc;
    url->accept(&wc);
    urlMap[wc.getWidget()] = url;
    itemSetMap[wc.getWidget()] = setMap[inDataWidget];
    connect(wc.getWidget(), SIGNAL(si_dataChanged()), SLOT(sl_itemChanged()));
    connect(wc.getWidget(), SIGNAL(si_itemDeleted()), SLOT(sl_itemDeleted()));

    inDataWidget->addUrlItem(wc.getWidget());
}

DatasetWidget * DatasetsController::createDatasetWidget(Dataset *dSet) {
    DatasetWidget *inDataWidget = new DatasetWidget(datasetsWidget);
    setMap[inDataWidget] = dSet;
    connect(inDataWidget, SIGNAL(si_addUrl(const QString &, U2OpStatus &)),
        SLOT(sl_addUrl(const QString &, U2OpStatus &)));
    connect(inDataWidget, SIGNAL(si_replaceUrl(UrlItem *, int)), SLOT(sl_replaceUrl(UrlItem *, int)));

    foreach (URLContainer *url, dSet->getUrls()) {
        createItemWidget(url, inDataWidget);
    }

    return inDataWidget;
}

void DatasetsController::sl_itemChanged() {
    UrlItem *item = dynamic_cast<UrlItem*>(sender());
    SAFE_POINT(NULL != item, "NULL item widget", );

    URLContainer *url = urlMap[item];
    SAFE_POINT(NULL != url, "NULL url container", );

    URLContainerUpdater updater(item);
    url->accept(&updater);
    updateAttribute();
}

void DatasetsController::sl_itemDeleted() {
    UrlItem *item = dynamic_cast<UrlItem*>(sender());
    SAFE_POINT(NULL != item, "NULL item widget", );

    URLContainer *url = urlMap.take(item);
    SAFE_POINT(NULL != url, "NULL url container", );

    Dataset *dSet = itemSetMap.take(item);
    SAFE_POINT(NULL != dSet, "NULL dataset", );

    dSet->removeUrl(url);
    delete url;
    updateAttribute();
}

void DatasetsController::sl_replaceUrl(UrlItem *item, int newPos) {
    URLContainer *url = urlMap.value(item);
    SAFE_POINT(NULL != url, "NULL url container", );

    Dataset *dSet = itemSetMap.value(item);
    SAFE_POINT(NULL != dSet, "NULL dataset", );
    SAFE_POINT(newPos >=0 && newPos < dSet->getUrls().size(),
        "New url position is out of range", );

    int pos = dSet->getUrls().indexOf(url);
    dSet->getUrls().removeAt(pos);
    dSet->getUrls().insert(newPos, url);
    updateAttribute();
}

void DatasetsController::sl_addUrl(const QString &url, U2OpStatus &os) {
    DatasetWidget *inData = dynamic_cast<DatasetWidget*>(sender());
    SAFE_POINT(NULL != inData, "NULL input data widget", );

    URLContainer *urlCont = URLContainerFactory::createUrlContainer(url);
    if (NULL == urlCont) {
        os.setError(tr("This file or directory does not exist: %1").arg(url));
        return;
    }

    setMap[inData]->addUrl(urlCont);
    createItemWidget(urlCont, inData);
    updateAttribute();
}

void DatasetsController::sl_deleteDataset(int dsNum) {
    SAFE_POINT(dsNum < sets.size(), "Datasets: out of range", );

    Dataset *dSet = sets.at(dsNum);
    SAFE_POINT(NULL != dSet, "NULL dataset", );

    sets.removeOne(dSet);

    QList<UrlItem*> items = itemSetMap.keys(dSet);
    foreach (UrlItem *item, items) {
        urlMap.remove(item);
        itemSetMap.remove(item);
    }
    delete dSet;

    // add empty default dataset is the last one is deleted
    if (sets.isEmpty()) {
        sets << new Dataset();
        datasetsWidget->appendDataset(sets.first()->getName(),
            createDatasetWidget(sets.first()));
    }
    updateAttribute();
}

void DatasetsController::checkName(const QString &name, U2OpStatus &os, Dataset *exception) {
    if (name.isEmpty()) {
        os.setError(tr("Dataset name is empty"));
        return;
    }
    foreach (Dataset *dSet, sets) {
        if (dSet != exception && dSet->getName() == name) {
            os.setError(tr("This dataset name already exists"));
            return;
        }
    }
}

void DatasetsController::sl_addDataset(const QString &name, U2OpStatus &os) {
    checkName(name, os);
    CHECK_OP(os, );
    sets << new Dataset(name);
    datasetsWidget->appendDataset(sets.last()->getName(),
        createDatasetWidget(sets.last()));
    updateAttribute();
}

void DatasetsController::sl_renameDataset(int dsNum, const QString &newName, U2OpStatus &os) {    
    SAFE_POINT(dsNum < sets.size(), "Datasets: out of range", );

    Dataset *dSet = sets.at(dsNum);
    SAFE_POINT(NULL != dSet, "NULL dataset", );

    checkName(newName, os, dSet);
    CHECK_OP(os, );

    dSet->setName(newName);
    updateAttribute();
}

void DatasetsController::updateAttribute() {
    if (NULL != attr) {
        attr->updateValue();
    }
    emit si_attributeChanged();
}

} // U2
