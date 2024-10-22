/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include <U2Core/AppContext.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/U2SafePoints.h>

#include "ScriptingToolRegistry.h"

namespace U2 {

////////////////////////////////////////
//ScriptingTool
ScriptingTool::ScriptingTool(QString _name, QString _path, const QStringList& _runParams) : name(_name), path(_path), runParams(_runParams) {
}

void ScriptingTool::onPathChanged(ExternalTool *tool, const QStringList& runParams) {
    ScriptingToolRegistry* reg = AppContext::getScriptingToolRegistry();
    CHECK(NULL != reg, );

    if (tool->isValid()){
        if (NULL != reg->getByName(tool->getName())){
            reg->unregisterEntry(tool->getName());
        }
        if (!tool->getPath().isEmpty()){
            reg->registerEntry(new ScriptingTool(tool->getName(), tool->getPath(), runParams));
        }
    } else {
        reg->unregisterEntry(tool->getName());
    }
}

////////////////////////////////////////
//ScriptingToolRegistry
ScriptingToolRegistry::~ScriptingToolRegistry() {
    qDeleteAll(registry.values());
}

ScriptingTool* ScriptingToolRegistry::getByName(const QString& id){
    return registry.value(id, NULL);
}

bool ScriptingToolRegistry::registerEntry(ScriptingTool *t){
    if (registry.contains(t->getName())) {
        return false;
    } else {
        registry.insert(t->getName(), t);
        return true;
    }
}

void ScriptingToolRegistry::unregisterEntry(const QString &id){
    delete registry.take(id);
}

QList<ScriptingTool*> ScriptingToolRegistry::getAllEntries() const{
    return registry.values();
}

}//namespace
