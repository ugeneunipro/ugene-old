#ifndef _U2_MSA_ALIGN_ALG_REGISTRY_H_
#define _U2_MSA_ALIGN_ALG_REGISTRY_H_

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QMutex>
#include <QtCore/QObject>

#include <U2Core/global.h>

namespace U2 {

class MSAAlignTaskFactory;
class MSAAlignGUIExtensionsFactory;
class MSAAlignAlgorithmEnv;


class U2ALGORITHM_EXPORT MSAAlignAlgRegistry : public QObject {
    Q_OBJECT
public:
    MSAAlignAlgRegistry(QObject* pOwn = 0);
    ~MSAAlignAlgRegistry();
    
    bool registerAlgorithm(MSAAlignAlgorithmEnv* env);
    MSAAlignAlgorithmEnv* unregisterAlgorithm(const QString& id);
    MSAAlignAlgorithmEnv* getAlgorithm(const QString& id) const;
    
    QStringList getRegisteredAlgorithmIds() const;
private:
    mutable QMutex mutex;
    QMap<QString, MSAAlignAlgorithmEnv*> algorithms;
    
    Q_DISABLE_COPY(MSAAlignAlgRegistry);
};

class U2ALGORITHM_EXPORT MSAAlignAlgorithmEnv {
public:
    MSAAlignAlgorithmEnv(const QString& id, MSAAlignTaskFactory* tf , MSAAlignGUIExtensionsFactory* guiExt);
    virtual ~MSAAlignAlgorithmEnv();
    
    const QString& getId()  const {return id;}
    
    MSAAlignTaskFactory* getTaskFactory() const {return taskFactory;}
    MSAAlignGUIExtensionsFactory* getGUIExtFactory() const {return guiExtFactory;}

private:
    Q_DISABLE_COPY(MSAAlignAlgorithmEnv);

protected:
    QString id;
    MSAAlignTaskFactory* taskFactory;
    MSAAlignGUIExtensionsFactory* guiExtFactory;
};

} // namespace

#endif
