#include "SmithWatermanResultFilters.h"
#include <QtAlgorithms>

namespace U2 {

/***************************************
 * SWRF_EmptyFilter
 ***************************************/
const QString SWRF_EmptyFilter::ID = "none";

// Without any filtering
bool SWRF_EmptyFilter::applyFilter(QList<SmithWatermanResult>* /*lst*/) {    
    return true;
}

SmithWatermanResultFilter* SWRF_EmptyFilter::clone() const {
    return new SWRF_EmptyFilter(*this);
}

bool SWRF_EmptyFilter::needErase(
                        const SmithWatermanResult& /*currItem*/,
                        const SmithWatermanResult& /*someItem*/) const {    
    return false;
}

QString SWRF_EmptyFilter::getId() const {
    return ID;
}

/***************************************
* SWRF_WithoutIntersect
***************************************/
const QString SWRF_WithoutIntersect::ID = "filter-intersections";

//greater score goes first
static bool revScoreComparator(const SmithWatermanResult &s0, const SmithWatermanResult &s1) {
    bool res = false;
    if (s0.score < s1.score) {
        res = false;
    } else if (s0.score == s1.score) {
        U2Region r0 = s0.region;
        U2Region r1 = s1.region;
        res = r0 < r1;
    } else {
        res = true;
    }
    return res;
}
// Filtering all intersects result
bool SWRF_WithoutIntersect::applyFilter(QList<SmithWatermanResult>* lst) {
    QList<SmithWatermanResult>& results = *lst;

    qSort(results.begin(), results.end(), revScoreComparator);
    
    int i = 0;
    int size = results.size();
    while (i < size) {
        int j = i + 1;
        while (j < size) {
            const SmithWatermanResult& currItem = results[i];
            const SmithWatermanResult& someItem = results[j];
            if (needErase(currItem, someItem)) {
                results.removeAt(j);
                size--;
                continue;
            }
            j++;
        }
        i++;
    }
    return true;
}

SmithWatermanResultFilter* SWRF_WithoutIntersect::clone() const {
    return new SWRF_WithoutIntersect(*this);
}

bool SWRF_WithoutIntersect::needErase(
                        const SmithWatermanResult& currItem,
                        const SmithWatermanResult& someItem) const {
    U2Region currRegion = currItem.region;
    U2Region someRegion = someItem.region;
    if (currRegion.intersects(someRegion) && 
        currItem.strand == someItem.strand) {
        return true;
    }
    return false;
}

QString SWRF_WithoutIntersect::getId() const {
    return ID;
}

} // namesapce U2
