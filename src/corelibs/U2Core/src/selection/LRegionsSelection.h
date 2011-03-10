#ifndef _U2_LREGION_SELECTION_H_
#define _U2_LREGION_SELECTION_H_

#include <U2Core/U2Region.h>
#include <U2Core/SelectionModel.h>

namespace U2 {

class U2CORE_EXPORT LRegionsSelection : public GSelection {
    Q_OBJECT
public:
    LRegionsSelection(GSelectionType type, QObject* p = NULL) : GSelection(type, p) {}

    const QVector<U2Region>& getSelectedRegions() const {return regions;}

    void setSelectedRegions(const QVector<U2Region>& newSelection);

    void addRegion(const U2Region& r);

    void removeRegion(const U2Region& r);

    virtual bool isEmpty() const {return regions.isEmpty();}

    virtual void clear();

signals:
    void si_selectionChanged(LRegionsSelection* thiz, const QVector<U2Region>& added, const QVector<U2Region>& removed);

public:
    QVector<U2Region> regions;
};

}//namespace

#endif
