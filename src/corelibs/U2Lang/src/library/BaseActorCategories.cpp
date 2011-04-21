/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include <U2Lang/WorkflowEnv.h>

#include "LocalDomain.h"
#include "BaseActorCategories.h"

namespace U2 {
namespace Workflow {

const Descriptor BaseActorCategories::CATEGORY_DATASRC() {
    return Descriptor("a", tr("Data sources"), "");
}

const Descriptor BaseActorCategories::CATEGORY_DATASINK() {
    return Descriptor("b", tr("Data sinks"), "");
}

const Descriptor BaseActorCategories::CATEGORY_CONVERTERS() {
    return Descriptor("bc", tr("Data converters"), "");
}

const Descriptor BaseActorCategories::CATEGORY_BASIC() { 
    return Descriptor("c", tr("Basic analysis"), "");
}

const Descriptor BaseActorCategories::CATEGORY_STATISTIC() {
    return Descriptor("st", tr("Utils"), "");
}

const Descriptor BaseActorCategories::CATEGORY_ALIGNMENT() { 
    return Descriptor("d", tr("Multiple sequence alignment"), "");
}

const Descriptor BaseActorCategories::CATEGORY_ASSEMBLY() {
    return Descriptor("asm", tr("DNA assembly"), ""); 
}

const Descriptor BaseActorCategories::CATEGORY_TRANSCRIPTION() {
    return Descriptor("tr", tr("Transcription factor"), ""); 
}

const Descriptor BaseActorCategories::CATEGORY_SCRIPT() {
    return Descriptor("sa", tr("Custom elements"), "");
}

}//Workflow namespace
}//GB2 namespace
