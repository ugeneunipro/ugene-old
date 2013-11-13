#ifndef _U2_CUSTOM_PATTERN_ANNOTATION_TASK_H_
#define _U2_CUSTOM_PATTERN_ANNOTATION_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AutoAnnotationsSupport.h>

namespace U2 {


typedef QPair<QByteArray,QByteArray> FeaturePattern;

class FeatureStore {
    QList<FeaturePattern> features;
    QString name, path;
public:
    FeatureStore(const QString& storeName, const QString& filePath) : name(storeName), path(filePath) {}
    bool load();
    const QString& getName() const { return name; }
    const QList<FeaturePattern>& getFeatures() const { return features; }
};

typedef QSharedPointer<FeatureStore> SharedFeatureStore;

class CustomPatternAnnotationTask :  public Task
{
    Q_OBJECT
public:
    CustomPatternAnnotationTask(AnnotationTableObject* aobj, const U2EntityRef& entityRef, const SharedFeatureStore& store );

    void prepare();

private:
    AnnotationTableObject* aTableObj;
    U2EntityRef seqRef;
    SharedFeatureStore featureStore;

};


class CustomPatternAutoAnnotationUpdater : public AutoAnnotationsUpdater {
    Q_OBJECT
    SharedFeatureStore featureStore;
public:
    CustomPatternAutoAnnotationUpdater(const SharedFeatureStore& store);
    Task* createAutoAnnotationsUpdateTask(const AutoAnnotationObject* aa);
    bool checkConstraints(const AutoAnnotationConstraints& constraints);
};


} // namespace

#endif // _U2_CUSTOM_PATTERN_ANNOTATION_TASK_H_
