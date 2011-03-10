#ifndef _U2_ADV_SEQUENCE_WIDGET_H_
#define _U2_ADV_SEQUENCE_WIDGET_H_

#include <U2Core/global.h>
#include <U2Core/U2Region.h>

#include <QtGui/QAction>
#include <QtGui/QWidget>

namespace U2 {

class AnnotatedDNAView;
class DNASequenceObject;
class DNASequenceSelection;
class ADVSequenceWidgetAction;
class ADVSequenceObjectContext;
class GObject;

class U2VIEW_EXPORT ADVSequenceWidget : public QWidget {
    Q_OBJECT
public:
    ADVSequenceWidget(AnnotatedDNAView* ctx);

    AnnotatedDNAView* getAnnotatedDNAView() const {return ctx;}

    virtual QList<DNASequenceObject*> getSequenceObjects() const;
    virtual QList<ADVSequenceObjectContext*> getSequenceContexts() const {return seqContexts;}

    /** actions associated with this block. Automatically deleted with block*/
    virtual void addADVSequenceWidgetAction(ADVSequenceWidgetAction* action);
    
    ADVSequenceWidgetAction* getADVSequenceWidgetAction(const QString& objName) const;
    
    virtual ADVSequenceObjectContext* getActiveSequenceContext() const = 0;

    virtual void centerPosition(int pos, QWidget* skipView = NULL) = 0;

    virtual void buildPopupMenu(QMenu& m);

    //TODO: replace with GObjectViewObjectHandler
    virtual bool isWidgetOnlyObject(GObject* o) const { Q_UNUSED(o); return false;}

    virtual void updateState(const QVariantMap& m) {Q_UNUSED(m);}
    
    virtual void saveState(QVariantMap& m) {Q_UNUSED(m);}

    virtual U2Region getVisibleRange() const = 0;
    
    virtual void setVisibleRange(const U2Region& r) = 0;

    virtual int getNumBasesVisible() const = 0;

    virtual void setNumBasesVisible(int n) = 0;
    
signals:
    void si_sequenceObjectAdded(DNASequenceObject*);
    void si_sequenceObjectRemoved(DNASequenceObject* );
    
protected:
    AnnotatedDNAView*                   ctx;
    QList<ADVSequenceObjectContext*>    seqContexts;
    QList<ADVSequenceWidgetAction*>     wActions;
};

class U2VIEW_EXPORT ADVSequenceWidgetAction : public QAction {
    Q_OBJECT
public:
    ADVSequenceWidgetAction(const QString& objName, const QString& text) 
        : QAction(text, NULL), addToBar(false), addToMenu(false), seqWidget(NULL) { setObjectName(objName); }

    bool                addToBar;
    bool                addToMenu;
    QString             afterMenu;
    ADVSequenceWidget*  seqWidget;
};


}//namespace

#endif
