#ifndef _U2_ADV_SPLIT_WIDGET_H_
#define _U2_ADV_SPLIT_WIDGET_H_

#include <U2Core/global.h>
#include <QtGui/QSplitter>

namespace U2 {
    

class AnnotatedDNAView;
class GObject;

class U2VIEW_EXPORT ADVSplitWidget : public QWidget {
    Q_OBJECT
public:
    ADVSplitWidget(AnnotatedDNAView* view) : dnaView(view) { }
    AnnotatedDNAView* getAnnotatedDNAView() const {return dnaView;}
    virtual bool acceptsGObject(GObject* objects)  = 0;
    virtual void updateState(const QVariantMap& m) = 0;
    virtual void saveState(QVariantMap& m) = 0;
protected:
    AnnotatedDNAView* dnaView;
};


}//namespace

#endif //_U2_ADV_SPLIT_WIDGET_H_
