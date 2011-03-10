#ifndef _U2_UHMM3_QDACTOR_H_
#define _U2_UHMM3_QDACTOR_H_

#include "U2Lang/QDScheme.h"
#include <U2Lang/QueryDesignerRegistry.h>


namespace U2 {

class UHMM3SWSearchTask;

class UHMM3QDActor : public QDActor {
    Q_OBJECT
public:
    UHMM3QDActor(QDActorPrototype const* proto);
    int getMinResultLen() const;
    int getMaxResultLen() const;
    QString getText() const;
    Task* getAlgorithmTask(const QVector<U2Region>& location);
    QColor defaultColor() const { return QColor(0x66,0xa3,0xd2); }
    virtual bool hasStrand() const { return false; }
private slots:
    void sl_onTaskFinished(Task*);
private:
    QMap<UHMM3SWSearchTask*, qint64> offsets;
};

class UHMM3QDActorPrototype : public QDActorPrototype {
public:
    UHMM3QDActorPrototype();
    QDActor* createInstance() const { return new UHMM3QDActor(this); }
};

} //namespace

#endif
