#ifndef _U2_SW_QUERY_H_
#define _U2_SW_QUERY_H_

#include <U2Lang/QDScheme.h>
#include <U2Lang/QueryDesignerRegistry.h>
#include <U2Algorithm/SmithWatermanSettings.h>
#include <U2Algorithm/SmithWatermanTaskFactory.h>
#include <U2Designer/DelegateEditors.h>

namespace U2 {

class Attribute;
    
class SWAlgoEditor : public ComboBoxDelegate {
    Q_OBJECT
public:
    SWAlgoEditor(Attribute* algAttr) : ComboBoxDelegate(QVariantMap()), algAttr(algAttr) {}
public slots:
    void populate();
private:
    Attribute* algAttr;
};

class QDSWActor : public QDActor {
    Q_OBJECT
public:
    QDSWActor(QDActorPrototype const* proto);
    int getMinResultLen() const;
    int getMaxResultLen() const;
    QString getText() const;
    Task* getAlgorithmTask(const QVector<U2Region>& location);
    QColor defaultColor() const { return QColor(0xff,0xf8,0); }
private slots:
    void sl_onAlgorithmTaskFinished(Task*);
private:
    SmithWatermanSettings settings;
    QString transId, mtrx;
    SmithWatermanTaskFactory* algo;
    QMap<Task*, SmithWatermanReportCallbackImpl*> callbacks;
};

class SWQDActorFactory : public QDActorPrototype {
public:
    SWQDActorFactory();
    QIcon getIcon() const { return QIcon(":core/images/sw.png"); }
    virtual QDActor* createInstance() const { return new QDSWActor(this); }
};

}//namespace

#endif
