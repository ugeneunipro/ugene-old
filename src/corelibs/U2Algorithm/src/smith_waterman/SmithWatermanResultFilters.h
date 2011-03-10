#ifndef _U2_SMITH_WATERMAN_RESULT_FILTERS_H_
#define _U2_SMITH_WATERMAN_RESULT_FILTERS_H_

#include <QList>
#include <U2Core/U2Region.h>
#include <U2Algorithm/SmithWatermanResult.h>


namespace U2 {

class U2ALGORITHM_EXPORT SmithWatermanResultFilter {
public:
    virtual bool applyFilter(QList<SmithWatermanResult>* lst) = 0;
    virtual SmithWatermanResultFilter* clone() const = 0;
    virtual ~SmithWatermanResultFilter() {};
    
    virtual QString getId() const = 0;
};

class U2ALGORITHM_EXPORT SWRF_EmptyFilter: public SmithWatermanResultFilter {
private:
    static const QString ID;
    
public:
    virtual bool applyFilter(QList<SmithWatermanResult>* lst);
    virtual SmithWatermanResultFilter* clone() const;
    
    virtual QString getId() const;
    
private:
    bool needErase(const SmithWatermanResult& currItem,
                   const SmithWatermanResult& someItem) const;
};

class U2ALGORITHM_EXPORT SWRF_WithoutIntersect: public SmithWatermanResultFilter {
private:
    static const QString ID;

public:
    virtual bool applyFilter(QList<SmithWatermanResult>* lst);
    virtual SmithWatermanResultFilter* clone() const;
    
    virtual QString getId() const;
    
private:
    bool needErase(const SmithWatermanResult& currItem,
                   const SmithWatermanResult& someItem) const;

};

} // namesapce

#endif
