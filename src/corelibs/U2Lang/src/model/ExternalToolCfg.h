#ifndef ExternalToolCfg_h__
#define ExternalToolCfg_h__

#include <U2Lang/Datatype.h>
#include <U2Lang/ConfigurationEditor.h>

#include <QtCore/qglobal.h>
#include <QtCore/QString>
#include <QtCore/QObject>
#include <QtCore/QMap>


namespace U2 {

#define SEQ_WITH_ANNS QString("Sequence_with_annotations")

class U2LANG_EXPORT DataConfig {
public:
    QString attrName;
    QString type;
    QString format;
    QString description;
};

class U2LANG_EXPORT AttributeConfig {
public:
    QString attrName;
    QString type;
    QString description;
    //PropertyDelegate *delegate;
};

class U2LANG_EXPORT ExternalProcessConfig {
public:
    QList<DataConfig> inputs;
    QList<DataConfig> outputs;
    QList<AttributeConfig> attrs;
    QString cmdLine;
    QString name;
    QString description;
    QString templateDescription;
    QString filePath;
};


class U2LANG_EXPORT ExternalToolCfgRegistry: public QObject {
    Q_OBJECT
public:
    ExternalToolCfgRegistry(QObject *p = NULL): QObject(p) {}
    bool registerExternalTool(ExternalProcessConfig *cfg) {
        if(configs.contains(cfg->name)) {
            return false;
        } else {
            configs.insert(cfg->name, cfg);
            return true;
        }
    }
    ExternalProcessConfig *getConfigByName(const QString& name) const {
        if(configs.contains(name)) {
            return configs.value(name);
        } else {
            return NULL;
        }
    }
    void unregisterConfig(const QString &name) {
        configs.remove(name);
    }
    QList<ExternalProcessConfig*> getConfigs() const {
        return configs.values();
    }

private:
    QMap<QString, ExternalProcessConfig*> configs;

};

}


#endif // ExternalToolCfg_h__
