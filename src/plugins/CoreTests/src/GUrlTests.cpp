/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "GUrlTests.h"

#include <U2Core/AppContext.h>
#include <U2Core/GObject.h>
#include <U2Core/GHints.h>
#include <U2Core/GUrl.h>

namespace U2 {

#define ORIGINAL_URL_ATTR       "original"
#define EXPECTED_RESULT_ATTR    "expected-result"
#define PLATFORM_ATTR           "platform"
#define PLATFORM_WIN            "win"
#define PLATFORM_UNIX           "unix"

    void GTest_ConvertPath::init(XMLTestFormat*, const QDomElement& el) {
        originalUrl = el.attribute(ORIGINAL_URL_ATTR);
        expectedResult = el.attribute(EXPECTED_RESULT_ATTR);
        platform = el.attribute(PLATFORM_ATTR);

#ifdef Q_OS_WIN
        QString currPlatform = PLATFORM_WIN;
#else
        QString currPlatform = PLATFORM_UNIX;
#endif

        runThisTest = (platform == currPlatform);
        if(runThisTest) {
            GUrl gurl(originalUrl);
            result = gurl.getURLString();
            isFileUrl = (gurl.getType() == GUrl_File);
        }
    }

    Task::ReportResult GTest_ConvertPath::report() {
        if(runThisTest) {
            if(!isFileUrl) {
                stateInfo.setError(tr("%1 isn't a File URL.").arg(originalUrl));
            } else if (expectedResult != result) {
                stateInfo.setError(tr("%1 was converted into %2, while %3 was expected").arg(originalUrl).arg(result).arg(expectedResult));
            }
        }
        return ReportResult_Finished;
    }


    /*******************************
    * GUrlTests
    *******************************/
    QList<XMLTestFactory*> GUrlTests::createTestFactories() {
        QList<XMLTestFactory*> res;
        res.append(GTest_ConvertPath::createFactory());
        return res;
    }


}//namespace
