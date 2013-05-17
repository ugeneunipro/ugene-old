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
    datasetsWidget = new DatasetsListWidget(this);

    foreach (Dataset *dSet, sets) {
        datasetsWidget->appendPage(dSet->getName(), createDatasetWidget(dSet));
    }
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

URLListWidget * DatasetsController::createDatasetWidget(Dataset *dSet) {
    URLListController *ctrl = new URLListController(this, dSet);
    return ctrl->getWidget();
}

void DatasetsController::deleteDataset(int dsNum) {
    SAFE_POINT(dsNum < sets.size(), "Datasets: out of range", );

    Dataset *dSet = sets.at(dsNum);
    SAFE_POINT(NULL != dSet, "NULL dataset", );

    sets.removeOne(dSet);

    delete dSet;

    // add empty default dataset is the last one is deleted
    if (sets.isEmpty()) {
        sets << new Dataset();
        datasetsWidget->appendPage(sets.first()->getName(),
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

void DatasetsController::addDataset(const QString &name, U2OpStatus &os) {
    checkName(name, os);
    CHECK_OP(os, );
    sets << new Dataset(name);
    datasetsWidget->appendPage(sets.last()->getName(),
        createDatasetWidget(sets.last()));
    updateAttribute();
}

void DatasetsController::renameDataset(int dsNum, const QString &newName, U2OpStatus &os) {
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

/************************************************************************/
/* URLListController */
/************************************************************************/
URLListController::URLListController(DatasetsController *parent, Dataset *_set)
: QObject(parent), widget(NULL), controller(parent), set(_set)
{

}

URLListWidget * URLListController::getWidget() {
    if (NULL == widget) {
        createWidget();
    }
    return widget;
}

void URLListController::createWidget() {
    widget = new URLListWidget(this);

    foreach (URLContainer *url, set->getUrls()) {
        addItemWidget(url);
    }
}

void URLListController::addItemWidget(URLContainer *url) {
    SAFE_POINT(NULL != widget, "NULL url list widget", );
    ItemWidgetCreator wc;
    url->accept(&wc);
    urlMap[wc.getWidget()] = url;

    widget->addUrlItem(wc.getWidget());
}

void URLListController::changedUrl(UrlItem *item) {
    URLContainer *url = urlMap[item];
    SAFE_POINT(NULL != url, "NULL url container", );

    URLContainerUpdater updater(item);
    url->accept(&updater);
    controller->updateAttribute();
}

void URLListController::replaceUrl(int pos, int newPos) {
    URLContainer *url = getUrl(pos);
    CHECK(NULL != url, );

    SAFE_POINT(newPos >=0 && newPos < set->getUrls().size(),
        "New url position is out of range", );

    set->getUrls().removeAt(pos);
    set->getUrls().insert(newPos, url);
    controller->updateAttribute();
}

void URLListController::addUrl(const QString &url, U2OpStatus &os) {
    URLContainer *urlCont = URLContainerFactory::createUrlContainer(url);
    if (NULL == urlCont) {
        os.setError(tr("This file or directory does not exist: %1").arg(url));
        return;
    }

    set->addUrl(urlCont);
    addItemWidget(urlCont);
    controller->updateAttribute();
}

void URLListController::deleteUrl(int pos) {
    URLContainer *url = getUrl(pos);
    CHECK(NULL != url, );

    set->removeUrl(url);
    delete url;
    controller->updateAttribute();
}

URLContainer * URLListController::getUrl(int pos) {
    SAFE_POINT(pos < set->getUrls().size(), "Urls: out of range", NULL);
    URLContainer *url = set->getUrls().at(pos);
    SAFE_POINT(NULL != url, "NULL url container", NULL);
    return url;
}

} // U2
