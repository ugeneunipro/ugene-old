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

#ifndef _U2_WORKFLOW_BASE_LIB_H_
#define _U2_WORKFLOW_BASE_LIB_H_

#include <U2Lang/Datatype.h>

namespace U2 {
namespace Workflow {

class U2LANG_EXPORT BaseActorCategories : public QObject {
    Q_OBJECT
public:
    static const Descriptor CATEGORY_DATASRC();
    static const Descriptor CATEGORY_DATASINK();
    static const Descriptor CATEGORY_CONVERTERS();
    static const Descriptor CATEGORY_BASIC();
    static const Descriptor CATEGORY_ALIGNMENT();
    static const Descriptor CATEGORY_ASSEMBLY();
    static const Descriptor CATEGORY_TRANSCRIPTION();
    static const Descriptor CATEGORY_SCRIPT();
    
}; // BaseActorCategories

}//Workflow namespace
}//GB2 namespace

#endif
