#include "GHints.h"

#include "StateLockableDataModel.h"

namespace U2 {

void ModTrackHints::setMap(const QVariantMap& _map) {
    if (map == _map) {
        return;
    }
    map = _map;
    setModified();
}

void ModTrackHints::set(const QString& key, const QVariant& val) {
    if (get(key) == val) {
        return;
    }
    map[key] = val;
    setModified();
}

int ModTrackHints::remove(const QString& key) {
    int r = map.remove(key);
    if (r!=0) {
        setModified();
    }
    return r;
}

void ModTrackHints::setModified() {
    StateLockableTreeItem* modItem = p;
    while (modItem->getParentStateLockItem()!=NULL && topParentMode) {
        modItem = modItem->getParentStateLockItem();
    }
    if (!modItem->isStateLocked()) { //TODO:
        modItem->setModified(true);
    }
}

}//namespace

