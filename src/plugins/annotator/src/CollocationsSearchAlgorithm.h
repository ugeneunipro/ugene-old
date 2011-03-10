#ifndef _U2_COLLOCATION_SEARCH_ALGORITHM_H_
#define _U2_COLLOCATION_SEARCH_ALGORITHM_H_

#include <U2Core/Task.h>
#include <U2Core/U2Region.h>

namespace U2 {

class CollocationsAlgorithmListener {
public:
    virtual ~CollocationsAlgorithmListener(){};
    virtual void onResult(const U2Region& r) = 0;
};

class CollocationsAlgorithmItem {
public:
    CollocationsAlgorithmItem(){}
    CollocationsAlgorithmItem(const QString& _name) : name(_name){}

    QString         name;
    QVector<U2Region> regions;
};

class CollocationsAlgorithmSettings;

class CollocationsAlgorithm {
public:
    enum SearchType {NormalSearch, PartialSearch};
    static void find(const QList<CollocationsAlgorithmItem>& items, TaskStateInfo& si,
        CollocationsAlgorithmListener* l, const CollocationsAlgorithmSettings& cfg);
private:
    static void findN(const QList<CollocationsAlgorithmItem>& items, TaskStateInfo& si, 
                    CollocationsAlgorithmListener* l, const U2Region& searchRegion, qint64 distance);
    static void findP(const QList<CollocationsAlgorithmItem>& items, TaskStateInfo& si,
                    CollocationsAlgorithmListener* l, const U2Region& searchRegion, qint64 distance);

};

class CollocationsAlgorithmSettings {
public:
    CollocationsAlgorithmSettings() : distance(-1), st(CollocationsAlgorithm::NormalSearch) {}
    U2Region searchRegion;
    int distance;
    CollocationsAlgorithm::SearchType st;
};


} //namespace U2
#endif


