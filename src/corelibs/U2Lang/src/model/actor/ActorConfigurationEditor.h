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

#ifndef _U2_ACTORCONFIGURATIONEDITOR_H_
#define _U2_ACTORCONFIGURATIONEDITOR_H_

#include <U2Lang/ConfigurationEditor.h>

namespace U2 {
namespace Workflow {

class Actor;

/**
 * base class for Actor's configuration editor
 */
class U2LANG_EXPORT ActorConfigurationEditor : public ConfigurationEditor {
    Q_OBJECT
public:
    ActorConfigurationEditor();
    ActorConfigurationEditor(const ActorConfigurationEditor &other);

    virtual void setConfiguration(Actor *actor);
    virtual ConfigurationEditor * clone();

protected:
    Actor *cfg;
}; // ActorConfigurationEditor

} // Workflow
} // U2

#endif // _U2_ACTORCONFIGURATIONEDITOR_H_
