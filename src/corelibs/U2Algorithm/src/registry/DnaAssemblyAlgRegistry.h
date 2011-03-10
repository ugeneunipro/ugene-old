#ifndef _U2_DNA_ASSEMBLY_ALG_REGISTRY_H_
#define _U2_DNA_ASSEMBLY_ALG_REGISTRY_H_

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QMutex>
#include <QtCore/QObject>

#include <U2Core/global.h>

namespace U2 {

class DnaAssemblyToRefTaskFactory;
class DnaAssemblyGUIExtensionsFactory;
class DnaAssemblyAlgorithmEnv;


class U2ALGORITHM_EXPORT DnaAssemblyAlgRegistry : public QObject {
    Q_OBJECT
public:
    DnaAssemblyAlgRegistry(QObject* pOwn = 0);
    ~DnaAssemblyAlgRegistry();
    
    bool registerAlgorithm(DnaAssemblyAlgorithmEnv* env);
    DnaAssemblyAlgorithmEnv* unregisterAlgorithm(const QString& id);
    DnaAssemblyAlgorithmEnv* getAlgorithm(const QString& id) const;
    
    QStringList getRegisteredAlgorithmIds() const;
    QStringList getRegisteredAlgorithmsWithIndexFileSupport() const;
private:
    mutable QMutex mutex;
    QMap<QString, DnaAssemblyAlgorithmEnv*> algorithms;
    
    Q_DISABLE_COPY(DnaAssemblyAlgRegistry);
};

class U2ALGORITHM_EXPORT DnaAssemblyAlgorithmEnv {
public:
    DnaAssemblyAlgorithmEnv(const QString& id, DnaAssemblyToRefTaskFactory* tf , DnaAssemblyGUIExtensionsFactory* guiExt, bool supportsIndexFiles);
    virtual ~DnaAssemblyAlgorithmEnv();
    
    const QString& getId()  const {return id;}
    bool isIndexFilesSupported() const {return supportsIndexFiles;}
    
    DnaAssemblyToRefTaskFactory* getTaskFactory() const {return taskFactory;}
    DnaAssemblyGUIExtensionsFactory* getGUIExtFactory() const {return guiExtFactory;}

private:
    Q_DISABLE_COPY(DnaAssemblyAlgorithmEnv);

protected:
    QString id;
    bool supportsIndexFiles;
    DnaAssemblyToRefTaskFactory* taskFactory;
    DnaAssemblyGUIExtensionsFactory* guiExtFactory;
};

} // namespace

#endif
