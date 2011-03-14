#ifndef _U2_AUTO_ANNOTATIONS_SUPPORT_H_
#define _U2_AUTO_ANNOTATIONS_SUPPORT_H_

#include <QtCore/QSet>
#include <U2Core/Task.h>

namespace U2 {

class AnnotationTableObject;
class DNASequenceObject;
class DNAAlphabet;
class StateLock;

// This object represents in-memory AnnotationTableObject.
// Auto-annotations are used to represent temporary algorithm results,
// such as for example restriction sites or ORFS.
// Auto-annotations are controlled by AnnotatedDNAView.

class AutoAnnotationsUpdater;
class AutoAnnotationsSupport;

class U2CORE_EXPORT AutoAnnotationObject : public QObject 
{
    Q_OBJECT
public:
    AutoAnnotationObject(DNASequenceObject* obj);
    ~AutoAnnotationObject();
    AnnotationTableObject* getAnnotationObject() const { return aobj; }
    DNASequenceObject* getSeqObject() const {return dnaObj; }
    void setGroupEnabled(const QString& groupName, bool enabled);
    void lock();
    void unlock();
    void update();
    void update(AutoAnnotationsUpdater* updater);
public slots:
    void updateGroup(const QString& groupName);
private:
    DNASequenceObject* dnaObj;
    AnnotationTableObject*  aobj;
    AutoAnnotationsSupport* aaSupport;
    QSet<QString> enabledGroups;
    StateLock* stateLock;
};

#define AUTO_ANNOTATION_SETTINGS "auto-annotations/"

struct AutoAnnotationConstraints {
    AutoAnnotationConstraints() : alphabet(NULL) {}
    DNAAlphabet* alphabet;
};

class U2CORE_EXPORT AutoAnnotationsUpdater : public QObject {
    Q_OBJECT
private:
    QString groupName;
    QString name;
    bool checkedByDefault;
public:
    AutoAnnotationsUpdater(const QString& nm, const QString& gName);
    virtual ~AutoAnnotationsUpdater();
    const QString& getGroupName() { return groupName; }
    const QString& getName() { return name; }
    bool isCheckedByDefault() { return checkedByDefault; }
    virtual bool checkConstraints(const AutoAnnotationConstraints& constraints) = 0;   
    virtual Task* createAutoAnnotationsUpdateTask(const AutoAnnotationObject* aa) = 0;
};

class U2CORE_EXPORT AutoAnnotationsSupport : public QObject
{
    Q_OBJECT 
public:
    ~AutoAnnotationsSupport();
    void registerAutoAnnotationsUpdater(AutoAnnotationsUpdater* updater);
    QList<AutoAnnotationsUpdater*> getAutoAnnotationUpdaters();
    AutoAnnotationsUpdater* findUpdaterByGroupName(const QString& groupName);
    AutoAnnotationsUpdater* findUpdaterByName(const QString& name);
    void updateAnnotationsByGroup(const QString& groupName);
signals:
    void si_updateAutoAnnotationsGroupRequired(const QString& groupName);
private:
    QList<AutoAnnotationsUpdater*> aaUpdaters;
};

class U2CORE_EXPORT AutoAnnotationsUpdateTask : public Task {
public:
    AutoAnnotationsUpdateTask(AutoAnnotationObject* aaObj, Task* subtask); 
    virtual void prepare();
    virtual ~AutoAnnotationsUpdateTask(); 
protected:
    AutoAnnotationObject* aa;
    Task* subtask;
};


} // namespace

#endif
