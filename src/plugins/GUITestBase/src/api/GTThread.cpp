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

#include <U2Test/GUITestService.h>
#include <U2Test/MainThreadRunnable.h>

#include "GTGlobals.h"
#include "GTThread.h"

namespace U2 {

#define GT_CLASS_NAME "ThreadWaiter"

ThreadWaiter::ThreadWaiter(U2OpStatus &os) :
    os(os),
    startValue(0),
    endValue(0)
{
    timer.setInterval(GUITestService::TIMER_INTERVAL);
    connect(&timer, SIGNAL(timeout()), SLOT(sl_timeout()));
}

#define GT_METHOD_NAME "wait"
void ThreadWaiter::wait() {
    GUITestService *guiTestService = GUITestService::getGuiTestService();
    GT_CHECK(NULL != guiTestService, "GUITestService is NULL");
    startValue = guiTestService->getMainThreadTimerValue();
    timer.start();
    while (endValue <= startValue) {
        GTGlobals::sleep(GUITestService::TIMER_INTERVAL);
    }
    timer.stop();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "sl_timeout"
void ThreadWaiter::sl_timeout() {
    GUITestService *guiTestService = GUITestService::getGuiTestService();
    GT_CHECK_NO_MESSAGE(NULL != guiTestService, "GUITestService is NULL");
    endValue = guiTestService->getMainThreadTimerValue();
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTThread"

#define GT_METHOD_NAME "waitForMainThread"
void GTThread::waitForMainThread(U2OpStatus &os) {
    ThreadWaiter waiter(os);
    waiter.wait();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "runInMainThread"
void GTThread::runInMainThread(U2OpStatus &os, CustomScenario *scenario) {
    MainThreadRunnable::runInMainThread(os, scenario);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}   // namespace U2
