/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

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

//additional tool validations. Even with other executables 
class U2CORE_EXPORT ExternalToolValidation {
public:
    
    ExternalToolValidation(const QString& _toolRunnerProgram, const QString& _executableFile, const QStringList& _arguments, const QString& _expectedMsg, const QStrStrMap& _possibleErrorsDescr = QStrStrMap())
        :toolRunnerProgram(_toolRunnerProgram)
        ,executableFile(_executableFile)
        ,arguments(_arguments)
        ,expectedMsg(_expectedMsg)
        ,possibleErrorsDescr(_possibleErrorsDescr)
    {}

public:
    QString toolRunnerProgram;
    QString executableFile;
    QStringList arguments;
    QString expectedMsg;
    QStrStrMap possibleErrorsDescr;

    static const QString DEFAULT_DESCR_KEY;
};

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
    const QString&      getToolRunnerProgram()  const { return toolRunnerProgramm; }
    const QString&      getExecutableFileName()  const { return executableFileName; }
    const QStringList&  getValidationArguments()  const { return validationArguments; }
    const QString&      getValidMessage()  const { return validMessage; }
    const QString&      getVersion()  const { return version; }
    const QRegExp&      getVersionRegExp()  const { return versionRegExp; }
    const QString&      getToolKitName()  const { return toolKitName; }
    const QStrStrMap&   getErrorDescriptions()  const { return errorDescriptions; }

    ExternalToolValidation getToolValidation();
    const QList<ExternalToolValidation>& getToolAdditionalValidations() const { return additionalValidators; }

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
    QString     toolRunnerProgramm;
    QString     executableFileName;
    QStringList validationArguments;
    QString     validMessage;
    QString     version;
    QRegExp     versionRegExp;
    bool        isValidTool;
    QString     toolKitName;
    QStrStrMap  errorDescriptions;
    QList<ExternalToolValidation> additionalValidators;

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

class U2CORE_EXPORT DefaultExternalToolValidations{
public:
    static ExternalToolValidation pythonValidation();
    static ExternalToolValidation rValidation();
};

} //namespace
#endif // U2_EXTERNAL_TOOL_REGISTRY_H
