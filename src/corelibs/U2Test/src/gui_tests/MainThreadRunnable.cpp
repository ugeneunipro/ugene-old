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

#include <QApplication>
#include <QThread>

#include <U2Core/AppContext.h>
#include <U2Core/ServiceModel.h>
#include <U2Core/ServiceTypes.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/U2SafePoints.h>

#include "GUITestService.h"
#include "MainThreadRunnable.h"

namespace U2 {

MainThreadRunnable::MainThreadRunnable(U2OpStatus &os, CustomScenario *scenario) :
    QObject(NULL),
    os(os),
    scenario(scenario)
{
    SAFE_POINT_EXT(NULL != scenario, os.setError("Scenario is NULL"), );
}

MainThreadRunnable::~MainThreadRunnable() {
    delete scenario;
}

void MainThreadRunnable::doRequest() {
    if (QThread::currentThread() == QApplication::instance()->thread()) {
        run();
        return;
    }

    QList<Service *> services = AppContext::getServiceRegistry()->findServices(Service_GUITesting);
    SAFE_POINT_EXT(services.size() == 1, os.setError("Can't get a GUI testing service"), );
    GUITestService *guiTestingService = qobject_cast<GUITestService *>(services.first());
    SAFE_POINT_EXT(NULL != guiTestingService, os.setError("GUI testing service is NULL"), );

    QMetaObject::Connection connection = connect(this,
                                                 SIGNAL(si_request(MainThreadRunnable *)),
                                                 guiTestingService,
                                                 SLOT(sl_requestAsked(MainThreadRunnable *)),
                                                 Qt::BlockingQueuedConnection);
    emit si_request(this);
    disconnect(connection);
}

void MainThreadRunnable::run() {
    SAFE_POINT_EXT(NULL != scenario, os.setError("Scenario is NULL"), );
    scenario->run(os);
}

void MainThreadRunnable::runInMainThread(U2OpStatus &os, CustomScenario *scenario) {
    SAFE_POINT_EXT(NULL != scenario, os.setError("Custom scenario is NULL"), );
    MainThreadRunnable mainThreadRunnable(os, scenario);
    mainThreadRunnable.doRequest();
}

}   // namespace U2
