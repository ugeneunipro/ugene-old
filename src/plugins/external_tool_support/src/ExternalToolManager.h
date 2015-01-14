/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#ifndef _U2_EXTERNAL_TOOL_VALIDATION_MANAGER_H_
#define _U2_EXTERNAL_TOOL_VALIDATION_MANAGER_H_

#include <QtCore/QObject>
#include <QtCore/QEventLoop>

#include <U2Core/global.h>
#include <U2Core/ExternalToolRegistry.h>

namespace U2 {

class ExternalToolsValidateTask;

/**
  * Manager can sort an external tools list by their dependencies,
  * run external tools validation tasks, validate tools in
  * the approaching moment (on the startup, on the workflow validation)
  **/
class ExternalToolManagerImpl : public ExternalToolManager {
    Q_OBJECT
public:
    ExternalToolManagerImpl();
    virtual ~ExternalToolManagerImpl();

    virtual void start();
    virtual void stop();

    virtual void check(const QString& toolName, const QString& toolPath, ExternalToolValidationListener* listener);
    virtual void check(const QStringList& toolNames, const QStrStrMap& toolPaths, ExternalToolValidationListener* listener);

    virtual void validate(const QString& toolName, ExternalToolValidationListener* listener = NULL);
    virtual void validate(const QString& toolName, const QString& path, ExternalToolValidationListener* listener = NULL);
    virtual void validate(const QStringList& toolNames, ExternalToolValidationListener* listener = NULL);
    virtual void validate(const QStringList& toolNames, const QStrStrMap& toolPaths, ExternalToolValidationListener* listener = NULL);

    virtual bool isValid(const QString& toolName) const;
    virtual ExternalToolState getToolState(const QString& toolName) const;

signals:
    void si_validationComplete(const QStringList& toolNames, QObject* receiver = NULL, const char* slot = NULL);

private slots:
    void sl_checkTaskStateChanged();
    void sl_validationTaskStateChanged();
    void sl_searchTaskStateChanged();
    void sl_toolValidationStatusChanged(bool isValid);
    void sl_pluginsLoaded();

private:
    void innerStart();
    void checkStartupTasksState();
    QString addTool(ExternalTool* tool);
    bool dependenciesAreOk(const QString& toolName);
    void validateTools(const QStrStrMap& toolPaths = QStrStrMap(), ExternalToolValidationListener* listener = NULL);
    void searchTools();
    void setToolPath(const QString& toolName, const QString& toolPath);
    void setToolValid(const QString& toolName, bool isValid);

    ExternalToolRegistry* etRegistry;
    QList<QString> validateList;
    QList<QString> searchList;
    QStrStrMap dependencies;    // master - vassal
    QMap<QString, ExternalToolState> toolStates;
    QMap<ExternalToolsValidateTask*, ExternalToolValidationListener*> listeners;
    bool startupChecks;

    static const int MAX_PARALLEL_SUBTASKS = 5;
};

}   //namespace

#endif // _U2_EXTERNAL_TOOL_VALIDATION_MANAGER_H_
