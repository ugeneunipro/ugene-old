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

#include <QApplication>
#include <QWebFrame>

#include "EstimationReporter.h"

namespace U2 {

static QWebFrame * frame(QWebView *view) {
    return view->page()->mainFrame();
}

static void eval(QWebView *view, const QString &code) {
    frame(view)->evaluateJavaScript(code);
}

static QString getHtml(const QString &loadUrl) {
    QFile file(loadUrl);
    bool opened = file.open(QIODevice::ReadOnly);
    if (!opened) {
        coreLog.error("Can not load " + loadUrl);
        return "";
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QString html = stream.readAll();
    file.close();

    return html;
}

QWebView * EstimationReporter::generateReport(const Workflow::EstimationResult &er) {
    QWebView *result = new QWebView();
    ReportGenerationHelper helper(result);
    result->setHtml(getHtml(":U2Designer/html/EstimationReport.html"));
    helper.waitLoading();
    SAFE_POINT(helper.loadedOk, "Can not load html", result);

    eval(result, "setTime('" + QString::number(er.timeSec) + "')");
    eval(result, "setRAM('" + QString::number(er.ramMb) + "')");
    //eval(result, "setHDD('" + QString::number(er.hddMb) + "')");
    //eval(result, "setCPU('" + QString::number(er.cpuCount) + "')");

    result->setContextMenuPolicy(Qt::NoContextMenu);
    return result;
}

static QString toTimeString(qint64 timeSec) {
    qint64 hours = timeSec / 3600;
    qint64 minutes = (timeSec - (hours * 3600)) / 60;
    qint64 minutesUp = (timeSec + 59 - (hours * 3600)) / 60;

    QString result;
    QString m = QObject::tr("m");
    QString h = QObject::tr("h");
    if (minutes > 0 || hours > 0) {
        result = QString::number(minutesUp) + m;
    } else {
        result = "< 1" + m;
    }
    if (hours > 0) {
        result = QString::number(hours) + h + " " + result;
    }
    return result;
}

QMessageBox * EstimationReporter::createTimeMessage(const Workflow::EstimationResult &er) {
    QMessageBox *result = new QMessageBox(
        QMessageBox::Information,
        QObject::tr("Workflow Estimation"),
        QObject::tr("Approximate estimation time of the workflow run is ") + toTimeString(er.timeSec) + ".",
        QMessageBox::Close);
    return result;
}

ReportGenerationHelper::ReportGenerationHelper(QWebView *view) {
    loaded = false;
    loadedOk = false;
    connect(view, SIGNAL(loadFinished(bool)), SLOT(sl_loadFinished(bool)));
}

void ReportGenerationHelper::sl_loadFinished(bool ok) {
    loaded = true;
    loadedOk = ok;
}

void ReportGenerationHelper::waitLoading() {
    while (!loaded) {
        QApplication::processEvents();
    }
}

} // U2
