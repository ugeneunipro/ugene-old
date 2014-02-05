/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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


#include <QtCore/QEventLoop>

#include <U2Core/Log.h>

#include "ForeverTask.h"

#define FOREVER_TASK_LOG_CAT "Forever task"

namespace U2 {

Logger log( FOREVER_TASK_LOG_CAT );

ForeverTask::ForeverTask() : Task( "Forever task", TaskFlag_None ) {
    log.trace( tr( "Forever task created" ) );
}

ForeverTask::~ForeverTask() {
}

void ForeverTask::run() {
    QEventLoop loop;
    loop.exec();
}

} // U2
