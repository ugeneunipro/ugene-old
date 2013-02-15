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

#include "ExternalToolRegistry.h"

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>

#include <U2Core/Log.h>
namespace U2 {

////////////////////////////////////////
//ExternalToolValidation
const QString ExternalToolValidation::DEFAULT_DESCR_KEY = "DEFAULT_DESCR";

////////////////////////////////////////
//ExternalTool
ExternalTool::ExternalTool(QString _name, QString _path) : name(_name), path(_path){
}

ExternalTool::~ExternalTool() {
}

void ExternalTool::setPath(const QString& _path) {
    if (path!=_path) {
        path=_path;
        emit si_pathChanged();
    }
}
void ExternalTool::setValid(bool _isValid){
    isValidTool=_isValid;
}
void ExternalTool::setVersion(const QString& _version) {
    version=_version;
}

ExternalToolValidation ExternalTool::getToolValidation() {
    ExternalToolValidation result(executableFileName, validationArguments, validMessage, errorDescriptions);
    return result; 
}

////////////////////////////////////////
//ExternalToolRegistry
ExternalToolRegistry::~ExternalToolRegistry() {
    qDeleteAll(registry.values());
}

ExternalTool* ExternalToolRegistry::getByName(const QString& id)
{
    return registry.value(id, NULL);
}

bool ExternalToolRegistry::registerEntry(ExternalTool *t){
    if (registry.contains(t->getName())) {
        return false;
    } else {
        registry.insert(t->getName(), t);
        return true;
    }
}

void ExternalToolRegistry::unregisterEntry(const QString &id){
    delete registry.take(id);
}

QList<ExternalTool*> ExternalToolRegistry::getAllEntries() const
{
    return registry.values();
}
QList< QList<ExternalTool*> > ExternalToolRegistry::getAllEntriesSortedByToolKits() const
{
    QList< QList<ExternalTool*> > res;
    QList<ExternalTool*> list=registry.values();
    while(!list.isEmpty()){
        QString name=list.first()->getToolKitName();
        QList<ExternalTool*> toolKitList;
        for(int i=0;i<list.length();i++){
            if(name == list.at(i)->getToolKitName()){
                toolKitList.append(list.takeAt(i));
                i--;
            }
        }
        res.append(toolKitList);
    }
    return res;
}

ExternalToolValidation DefaultExternalToolValidations::pythonValidation(){
    QString pythonExecutable = "python";
    QStringList pythonArgs;
    pythonArgs << "--version";
    QString pmsg = "Python";
    QStrStrMap perrMsgs;
    perrMsgs.insert(ExternalToolValidation::DEFAULT_DESCR_KEY, "Python 2 required for this tool. Please install Python or set your PATH variable if you have it installed.");

    ExternalToolValidation pythonValidation(pythonExecutable, pythonArgs, pmsg, perrMsgs);
    return pythonValidation;
}

ExternalToolValidation DefaultExternalToolValidations::rValidation(){
    QString rExecutable = "rscript";
    QStringList rArgs;
    rArgs << "--version";
    QString rmsg = "R";
    QStrStrMap rerrMsgs;
    rerrMsgs.insert(ExternalToolValidation::DEFAULT_DESCR_KEY, "R Script required for this tool. Please install R Script or set your PATH variable if you have it installed.");

    ExternalToolValidation rValidation(rExecutable, rArgs, rmsg, rerrMsgs);
    return rValidation;
}

}//namespace
