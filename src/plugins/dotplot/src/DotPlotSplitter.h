#ifndef _U2_DOT_PLOT_SPLITTER_H_
#define _U2_DOT_PLOT_SPLITTER_H_


#include <U2View/ADVSplitWidget.h>

#include <QtGui/QIcon>

class QMenu;
class QToolButton;

namespace U2 {

class GObjectView;
class DotPlotWidget;
class ADVSequenceObjectContext;
class HBar;

class DotPlotSplitter : public ADVSplitWidget {
    Q_OBJECT

public:
    DotPlotSplitter(AnnotatedDNAView*);
    ~DotPlotSplitter();

    virtual bool acceptsGObject(GObject* objects) {Q_UNUSED(objects);return false;}
    virtual void updateState(const QVariantMap&){};
    virtual void saveState(QVariantMap&){};

    void addView(DotPlotWidget*);
    void removeView(DotPlotWidget*);
    bool isEmpty() const;
    void buildPopupMenu(QMenu *);

private:
    QToolButton *createToolButton(const QString& iconPath, const QString& toolTip, const char *slot, bool checkable = true);
    QToolButton *createToolButton(const QIcon& ic, const QString& toolTip, const char *slot, bool checkable = true);

    QSplitter *splitter;
    HBar *buttonToolBar;
    QList<DotPlotWidget*> dotPlotList;

    bool locked;
    QToolButton *syncLockButton, *aspectRatioButton, *zoomInButton, *zoomOutButton, *resetZoomingButton, *zoomToButton, *handButton, *selButton;

    void updateButtonState();
    void checkLockButtonState();

private slots:
    void sl_toggleSyncLock(bool);
    void sl_toggleAspectRatio(bool);
    void sl_toggleSel();
    void sl_toggleHand();
    void sl_toggleZoomIn();
    void sl_toggleZoomOut();
    void sl_toggleZoomReset();

    void sl_dotPlotChanged(ADVSequenceObjectContext*, ADVSequenceObjectContext*, float, float, QPointF);
    void sl_dotPlotSelecting();
};

} // namespace

#endif // _U2_DOT_PLOT_SPLITTER_H_
