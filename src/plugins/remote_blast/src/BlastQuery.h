#ifndef _U2_BLAST_QUERY_H_
#define _U2_BLAST_QUERY_H_

#include "RemoteBLASTTask.h"
#include "RemoteBLASTConsts.h"

#include <U2Lang/QDScheme.h>
#include <U2Lang/QueryDesignerRegistry.h>


namespace U2 {

class QDCDDActor : public QDActor {
    Q_OBJECT
public:
    QDCDDActor(QDActorPrototype const* a);
    int getMinResultLen() const;
    int getMaxResultLen() const;
    QString getText() const;
    Task* getAlgorithmTask(const QVector<U2Region>& location);
    QColor defaultColor() const { return QColor(0x6A, 0x94, 0xd4); }
    virtual bool hasStrand() const { return false; }
private slots:
    void sl_onAlgorithmTaskFinished();
private:
    RemoteBLASTTaskSettings settings;
    QMap<RemoteBLASTTask*, int> offsetMap;
};

class QDCDDActorPrototype : public QDActorPrototype {
public:
    QDCDDActorPrototype();
    virtual QDActor* createInstance() const { return new QDCDDActor(this); }
    virtual QIcon getIcon() const { return QIcon(":remote_blast/images/remote_db_request.png"); }
};

}//namespace

#endif
