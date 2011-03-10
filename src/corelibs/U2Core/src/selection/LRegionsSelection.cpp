#include "LRegionsSelection.h"

namespace U2 {

static QVector<U2Region> emptyLRegions;
void LRegionsSelection::clear() {
    QVector<U2Region> tmpRemoved = regions;
    regions.clear();
    if (!tmpRemoved.isEmpty()) {
        emit si_selectionChanged(this, emptyLRegions, tmpRemoved);
    }
}

void LRegionsSelection::addRegion(const U2Region& r) {
    assert(r.length > 0);
    if (regions.contains(r)) {
        return;
    }
    regions.append(r);
    QVector<U2Region> tmpAdded; tmpAdded.append(r);
    emit si_selectionChanged(this, tmpAdded, emptyLRegions);
}

void LRegionsSelection::removeRegion(const U2Region& r) {
    int idx = regions.indexOf(r);
    if (idx == -1) {
        return;
    }
    regions.remove(idx);
    QVector<U2Region> tmpRemoved; tmpRemoved.append(r);
    emit si_selectionChanged(this, emptyLRegions, tmpRemoved);
}

void LRegionsSelection::setSelectedRegions(const QVector<U2Region>& newSelection) {
    QVector<U2Region> tmpRemoved = regions;
    regions = newSelection;
    emit si_selectionChanged(this, newSelection, tmpRemoved);    
}

} //namespace
