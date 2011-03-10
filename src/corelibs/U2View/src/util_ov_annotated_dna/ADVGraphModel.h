#ifndef _U2_AVD_GRAPH_MODEL_H_
#define _U2_AVD_GRAPH_MODEL_H_

#include <U2Core/global.h>
#include <U2Core/U2Region.h>

#include <QtCore/QVector>
#include <QtGui/QPixmap>

namespace U2 {

class DNASequenceObject;
class GSequenceGraphData;
class GSequenceGraphWindowData;
class GSequenceGraphView;

//BUG:402: refactor to remove cross references in structures!

class U2VIEW_EXPORT GSequenceGraphAlgorithm {
public:
    virtual ~GSequenceGraphAlgorithm(){}
    virtual void calculate(QVector<float>& res, DNASequenceObject* o, const U2Region& r, const GSequenceGraphWindowData* d) = 0;
};

class U2VIEW_EXPORT GSequenceGraphWindowData {
public:
    GSequenceGraphWindowData() :  step(0), window(0){}
    GSequenceGraphWindowData(int _step, int w) :  step(_step), window(w){}

    virtual ~GSequenceGraphWindowData(){};

    int step;
    int window;
};

class U2VIEW_EXPORT GSequenceGraphMinMaxCutOffData {
public:
    GSequenceGraphMinMaxCutOffData() : min(0.0), max(0.0), enableCuttoff(false) {}
    GSequenceGraphMinMaxCutOffData(double _min, double _max, bool _enableCuttoff) : min(_min), max(_max), enableCuttoff(_enableCuttoff){}

    virtual ~GSequenceGraphMinMaxCutOffData(){};

    double min;
    double max;
    bool enableCuttoff;
};

struct PairVector {
    QVector<float>  firstPoints;  //max if use both
    QVector<float>  secondPoints;
    bool useIntervals;
};


class U2VIEW_EXPORT GSequenceGraphDrawer : public QObject{
    Q_OBJECT
public:
    GSequenceGraphDrawer(GSequenceGraphView* v, const GSequenceGraphWindowData& wd);
    virtual ~GSequenceGraphDrawer();

    virtual void draw(QPainter& p, GSequenceGraphData* d, const QRect& rect);

    virtual void showSettingsDialog();
    
    const GSequenceGraphWindowData& getWindowData() {return wdata;}
    const GSequenceGraphMinMaxCutOffData& getCutOffData() {return commdata;}

protected:
    void calculatePoints(GSequenceGraphData* d, PairVector& points, float& min, float& max, int numPoints);
    
    // calculates points (> visual area size) and fits the result into visual size
    void calculateWithFit(GSequenceGraphData* d, PairVector& points, int alignedStart, int alignedEnd);
    
    // calculates points (< visual area size) and expands points to fill all visual area size
    void calculateWithExpand(GSequenceGraphData* d, PairVector& points, int alignedStart, int alignedEnd);

protected:
    GSequenceGraphView*             view;
    QFont*                          defFont;

    GSequenceGraphWindowData        wdata;
    GSequenceGraphMinMaxCutOffData  commdata;
};


class U2VIEW_EXPORT GSequenceGraphData {
public:
    GSequenceGraphData(const QString& _graphName);
    virtual ~GSequenceGraphData();

    QString                     graphName;
    GSequenceGraphAlgorithm*    ga;

    int                         cachedFrom, cachedLen, cachedW, cachedS;
    int                         alignedFC, alignedLC;
    PairVector                  cachedData;
};


class U2VIEW_EXPORT GSequenceGraphUtils {
public:
    static int getNumSteps(const U2Region& range, int w, int s);

    static void fitToScreen(const QVector<float>& data, int dataStartBase, int dataEndBase,
                            QVector<float>& results,  int resultStartBase, int resultEndBase,
                            int screenWidth, float unknownVal);

    static float calculateAverage(const QVector<float>& data, float start, float range);
    static void calculateMinMax(const QVector<float>& data, float& min, float& max);

};

} // namespace

#endif
