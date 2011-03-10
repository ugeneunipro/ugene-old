#ifndef _U2_DOT_PLOT_CLASSES_H_
#define _U2_DOT_PLOT_CLASSES_H_

#include <U2Algorithm/RepeatFinderSettings.h>

#include <QtGui/QMessageBox>
#include <QtGui/QPainter>
#include <QtCore/QMutex>

namespace U2 {

// mini map on the DotPlotWidget
class DotPlotMiniMap {
public:
    DotPlotMiniMap (int bigMapW, int bigMapH, float ratio);
    void draw(QPainter &p, int shiftX, int shiftY, const QPointF &zoom) const;

    QRectF getBoundary() const;
    QPointF fromMiniMap(const QPointF &p, const QPointF &zoom) const;

private:
    int x, y, w, h;
    float ratio;
};

// error messages and dialogs
class DotPlotDialogs: QObject {
    Q_OBJECT
public:
    enum Errors {ErrorOpen, ErrorNames, NoErrors};

    static void taskRunning();
    static int saveDotPlot();
    static void fileOpenError(const QString &filename);
    static void filesOpenError();
    static int loadDifferent();
    static void loadWrongFormat();
    static void wrongAlphabetTypes();
    static void tooManyResults();
};

struct DotPlotResults {
    DotPlotResults(): x(0), y(0), len(0){};
    DotPlotResults(int _x, int _y, int _len):x(_x), y(_y), len(_len){};

    int x, y, len;
};

// Listener which collect results from an algorithm
class DotPlotResultsListener : public RFResultsListener {
    friend class DotPlotWidget;

public:
    DotPlotResultsListener();
    ~DotPlotResultsListener();

    void setTask(Task *);

    virtual void onResult(const RFResult& r);
    virtual void onResults(const QVector<RFResult>& v);

private:
    QList<DotPlotResults> *dotPlotList;
    QMutex mutex;

    bool stateOk;

    static const int maxResults = 8*1024*1024;
    Task *rfTask;
};

} // namespace

#endif // _U2_DOT_PLOT_CLASSES_H_
