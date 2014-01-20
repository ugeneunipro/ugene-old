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

#include "GTUtilsLog.h"
#include "U2Core/LogCache.h"

namespace U2 {

GTLogTracer::GTLogTracer()
: wasError(false) {
    LogServer::getInstance()->addListener(this);
}

GTLogTracer::~GTLogTracer(){
    LogServer::getInstance()->removeListener(this);
}

void GTLogTracer::onMessage(const LogMessage &msg) {

    if (msg.level == LogLevel_ERROR) {
        wasError = true;
    }

}

QList<LogMessage*> GTLogTracer::getMessages(){
    return LogCache::getAppGlobalInstance()->messages;
}

bool GTLogTracer::checkMessage(QString s){
    foreach (LogMessage* message, getMessages()){
        if(message->text.contains(s,Qt::CaseInsensitive)){
            return true;
        }
    }
    return false;
}

#define GT_CLASS_NAME "GTUtilsLog"
#define GT_METHOD_NAME "check"
void GTUtilsLog::check(U2OpStatus &os, GTLogTracer& l) {
    GTGlobals::sleep(500);
    GT_CHECK(l.hasError() == false, "There is an error in log");
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

} // namespace
