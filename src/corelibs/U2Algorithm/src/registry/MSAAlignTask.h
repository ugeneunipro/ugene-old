#ifndef _U2_MSA_ALIGN_TASK_H_
#define _U2_MSA_ALIGN_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/GUrl.h>
#include <U2Core/MAlignment.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/DNASequence.h>
#include <U2Core/UIndex.h>

namespace U2 {

class Document;

class U2ALGORITHM_EXPORT MSAAlignTaskSettings {
public:
    MSAAlignTaskSettings() {}
    GUrl resultFileName;
    QString algName;
    bool loadResultDocument;
    void setCustomSettings(const QMap<QString, QVariant>& settings);
    QVariant getCustomValue(const QString& optionName, const QVariant& defaultVal) const;
    void setCustomValue(const QString& optionName, const QVariant& val);
private:
    QMap<QString, QVariant> customSettings;
};

class U2ALGORITHM_EXPORT MSAAlignTask : public Task {
    Q_OBJECT
public:
    MSAAlignTask(MAlignmentObject* obj, const MSAAlignTaskSettings& settings, TaskFlags flags = TaskFlags_FOSCOE);
    virtual ~MSAAlignTask() {}
protected:
    const MSAAlignTaskSettings& settings;
    MAlignmentObject* obj;
};

class U2ALGORITHM_EXPORT MSAAlignTaskFactory {
public:
    virtual MSAAlignTask* createTaskInstance(MAlignmentObject* obj, const MSAAlignTaskSettings& settings) = 0;
    virtual ~MSAAlignTaskFactory() {}
};

#define MSA_ALIGN_TASK_FACTORY(c) \
public: \
    static const QString taskName; \
class Factory : public MSAAlignTaskFactory { \
public: \
    Factory() { } \
    MSAAlignTask* createTaskInstance(MAlignmentObject* obj, const MSAAlignTaskSettings& s) { return new c(obj, s); } \
};

} // U2


#endif // _U2_MSA_ALIGN_TASK_H_
