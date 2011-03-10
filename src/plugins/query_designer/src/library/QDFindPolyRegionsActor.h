#ifndef _U2_QD_FIND_POLY_ACTOR_H_
#define _U2_QD_FIND_POLY_ACTOR_H_

#include <U2Lang/QDScheme.h>
#include <U2Lang/QueryDesignerRegistry.h>

#include <U2Core/DNASequence.h>


namespace U2 {

class FindPolyRegionsSettings {
public:
    char ch;
    int percent;
    qint64 minLen;
    qint64 offset;
    FindPolyRegionsSettings() : percent(0.9f), minLen(0), offset(0) {}
};

class FindPolyRegionsTask : public Task {
    Q_OBJECT
public:
    FindPolyRegionsTask(const FindPolyRegionsSettings& settings, const DNASequence& sequence)
        : Task(tr("Find base content task"), TaskFlag_None), settings_(settings), sequence_(sequence) {}
    void run();
    QList<SharedAnnotationData> getResultAsAnnotations() const;
private:
    void find(const char* seq,
        qint64 seqLen,
        char ch,
        int percent,
        qint64 len,
        QVector<U2Region>& result
        );
private:
    FindPolyRegionsSettings settings_;
    DNASequence sequence_;
    QVector<U2Region> resultRegions;
};

class QDFindPolyActor : public QDActor {
    Q_OBJECT
public:
    QDFindPolyActor(QDActorPrototype const* proto);
    int getMinResultLen() const;
    int getMaxResultLen() const;
    QString getText() const;
    Task* getAlgorithmTask(const QVector<U2Region>& location);
    QColor defaultColor() const { return QColor(0xff, 0xc6, 0); }
    virtual bool hasStrand() const { return false; }
private slots:
    void sl_onTaskFinished(Task*);
};

class QDFindPolyActorPrototype : public QDActorPrototype {
public:
    QDFindPolyActorPrototype();
    virtual QDActor* createInstance() const { return new QDFindPolyActor(this); }
};

} //namespace

#endif
