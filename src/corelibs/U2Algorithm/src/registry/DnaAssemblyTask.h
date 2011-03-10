#ifndef _U2_DNA_ASSEMBLY_TASK_H_
#define _U2_DNA_ASSEMBLY_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/GUrl.h>
#include <U2Core/MAlignment.h>
#include <U2Core/DNASequence.h>
#include <U2Core/UIndex.h>

namespace U2 {

class Document;

class U2ALGORITHM_EXPORT DnaAssemblyToRefTaskSettings {
public:
    DnaAssemblyToRefTaskSettings() 
    {}
    QList<DNASequence> shortReads;
    QList<GUrl> shortReadUrls;
    GUrl refSeqUrl;
    GUrl resultFileName;
    QString indexFileName;
    QString algName;
    bool loadResultDocument;
    void setCustomSettings(const QMap<QString, QVariant>& settings);
    QVariant getCustomValue(const QString& optionName, const QVariant& defaultVal) const;
    void setCustomValue(const QString& optionName, const QVariant& val);
private:
    QMap<QString, QVariant> customSettings;
};

class U2ALGORITHM_EXPORT DnaAssemblyToReferenceTask : public Task {
    Q_OBJECT
public:
    DnaAssemblyToReferenceTask(const DnaAssemblyToRefTaskSettings& settings, TaskFlags flags = TaskFlags_FOSCOE, bool justBuildIndex = false);
    virtual ~DnaAssemblyToReferenceTask() {}
    const MAlignment& getResult() { return result; }
    const GUrl getResultUrl() { return settings.resultFileName; }
    bool isHaveResult() const {return haveResults;}
    bool isJustBuildIndex() const {return justBuildIndex;}
protected:
    const DnaAssemblyToRefTaskSettings& settings;
    MAlignment result;
    bool justBuildIndex;
    bool haveResults;
};

class U2ALGORITHM_EXPORT DnaAssemblyToRefTaskFactory {
public:
    virtual DnaAssemblyToReferenceTask* createTaskInstance(const DnaAssemblyToRefTaskSettings& settings, bool justBuildIndex = false) = 0;
    virtual ~DnaAssemblyToRefTaskFactory() {}
};

#define DNA_ASSEMBLEY_TO_REF_TASK_FACTORY(c) \
public: \
    static const QString taskName; \
class Factory : public DnaAssemblyToRefTaskFactory { \
public: \
    Factory() { } \
    DnaAssemblyToReferenceTask* createTaskInstance(const DnaAssemblyToRefTaskSettings& s, bool justBuildIndex = false) { return new c(s, justBuildIndex); } \
};

} // U2


#endif // _U2_DNA_ASSEMBLEY_TASK_H_
