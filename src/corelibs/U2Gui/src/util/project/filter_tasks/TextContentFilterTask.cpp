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

#include <U2Core/TextObject.h>
#include <U2Core/U2SafePoints.h>

#include "../ProjectFilterNames.h"

#include "TextContentFilterTask.h"

namespace U2 {

//////////////////////////////////////////////////////////////////////////
/// TextContentFilterTask
//////////////////////////////////////////////////////////////////////////

TextContentFilterTask::TextContentFilterTask(const ProjectTreeControllerModeSettings &settings, const QList<QPointer<Document> > &docs)
    : AbstractProjectFilterTask(settings, ProjectFilterNames::TEXT_CONTENT_FILTER_NAME, docs)
{

}

bool TextContentFilterTask::filterAcceptsObject(GObject *obj) {
    TextObject *textObject = qobject_cast<TextObject *>(obj);
    CHECK(NULL != textObject, false);
    return settings.nameFilterAcceptsString(textObject->getText());
}

//////////////////////////////////////////////////////////////////////////
/// TextContentFilterTaskFactory
//////////////////////////////////////////////////////////////////////////

AbstractProjectFilterTask * TextContentFilterTaskFactory::createNewTask(const ProjectTreeControllerModeSettings &settings,
    const QList<QPointer<Document> > &docs) const
{
    return new TextContentFilterTask(settings, docs);
}

} // namespace U2
