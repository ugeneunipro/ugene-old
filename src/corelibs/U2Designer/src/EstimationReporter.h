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

#ifndef _U2_ESTIMATION_REPORTER_H_
#define _U2_ESTIMATION_REPORTER_H_

#include <QWebView>
#include <QMessageBox>

#include <U2Core/global.h>
#include <U2Lang/SchemaEstimationTask.h>

namespace U2 {

class U2DESIGNER_EXPORT EstimationReporter {
public:
    static QWebView * generateReport(const Workflow::EstimationResult &er);
    static QMessageBox * createTimeMessage(const Workflow::EstimationResult &er);
};

class ReportGenerationHelper : public QObject {
    Q_OBJECT
public:
    ReportGenerationHelper(QWebView *view);
    void waitLoading();
    bool loadedOk;

public slots:
    void sl_loadFinished(bool ok);

private:
    bool loaded;
};

} //U2

#endif // _U2_ESTIMATION_REPORTER_H_
