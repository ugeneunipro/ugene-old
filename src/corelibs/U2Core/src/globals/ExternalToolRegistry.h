#ifndef _U2_EXTERNAL_TOOL_REGISTRY_H
#define _U2_EXTERNAL_TOOL_REGISTRY_H

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QVariant>
#include <QtGui/QIcon>

#include <U2Core/IdRegistry.h>
#include <U2Core/global.h>

namespace U2 {


class U2CORE_EXPORT ExternalTool : public QObject {
    Q_OBJECT
public:
    ExternalTool(QString name, QString path = "");
    ~ExternalTool();

    const QString&      getName()  const { return name; }
    const QString&      getPath()  const { return path; }
    const QIcon&        getIcon()  const { return icon; }
    const QIcon&        getGrayIcon()  const { return grayIcon; }
    const QIcon&        getWarnIcon()  const { return warnIcon; }
    const QString&      getDescription()  const { return description; }
    const QString&      getExecutableFileName()  const { return executableFileName; }
    const QStringList&  getValidationArguments()  const { return validationArguments; }
    const QString&      getValidMessage()  const { return validMessage; }
    const QString&      getVersion()  const { return version; }
    const QRegExp&      getVersionRegExp()  const { return versionRegExp; }
    const QString&      getToolKitName()  const { return toolKitName; }

    void setPath(const QString& _path);
    void setValid(bool _isValid);
    void setVersion(const QString& _version);

    bool isValid() const { return isValidTool; }//may be not needed
signals:
    void si_pathChanged();

protected:
    QString     name;
    QString     path;
    QIcon       icon;
    QIcon       grayIcon;
    QIcon       warnIcon;
    QString     description;
    QString     executableFileName;
    QStringList validationArguments;
    QString     validMessage;
    QString     version;
    QRegExp     versionRegExp;
    bool        isValidTool;
    QString     toolKitName;

}; // ExternalTool


class U2CORE_EXPORT ExternalToolRegistry : public QObject {
    Q_OBJECT
public:
    ~ExternalToolRegistry();

    ExternalTool* getByName(const QString& id);

    bool registerEntry(ExternalTool* t);
    void unregisterEntry(const QString& id);

    QList<ExternalTool*> getAllEntries() const;
    QList< QList<ExternalTool*> > getAllEntriesSortedByToolKits() const;

protected:
    QMap<QString, ExternalTool*>    registry;
    QString                         temporaryDirectory;

}; // ExternalToolRegistry

} //namespace
#endif // U2_EXTERNAL_TOOL_REGISTRY_H
