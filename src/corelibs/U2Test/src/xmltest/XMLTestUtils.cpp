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

#include "XMLTestUtils.h"

#include <U2Core/GUrlUtils.h>

namespace U2 {

QList<XMLTestFactory*>  XMLTestUtils::createTestFactories() {
    QList<XMLTestFactory*> res;
    
    res.append(XMLMultiTest::createFactory());
    res.append(GTest_DeleteTmpFile::createFactory());
    res.append(GTest_Fail::createFactory());
    res.append(GTest_CreateTmpFolder::createFactory());

    return res;
}

void XMLMultiTest::init(XMLTestFormat *tf, const QDomElement& el) {

    // This attribute is used to avoid mixing log messages between different tests
    // Each test that listens to log should set this attribute to "true"
    // See also: GTestLogHelper
    bool lockForLogListening = false;
    if ("true" == el.attribute("lockForLogListening")) {
        lockForLogListening = true;
    }

    QDomNodeList subtaskNodes = el.childNodes();
    QList<Task*> subs;
    for(int i=0;i<subtaskNodes.size(); i++) {
        QDomNode n = subtaskNodes.item(i);
        if (!n.isElement()) {
            continue;
        }
        QDomElement subEl= n.toElement();
        QString name = subEl.tagName();
        QString err;
        GTest* subTest = tf->createTest(name, this, env, subEl, err);
        if (!err.isEmpty()) {
            stateInfo.setError(err);
            break;
        }
        assert(subTest);

        subs.append(subTest);
    }
    if (!hasError()) {
        if (lockForLogListening) {
            addTaskResource(TaskResourceUsage(RESOURCE_LISTEN_LOG_IN_TESTS, TaskResourceUsage::Write, true));
        }
        else {
            addTaskResource(TaskResourceUsage(RESOURCE_LISTEN_LOG_IN_TESTS, TaskResourceUsage::Read, true));
        }

        foreach(Task* t, subs) {
            addSubTask(t);
        }
    }
}


Task::ReportResult XMLMultiTest::report() {
    if (!hasError()) {
        Task* t = getSubtaskWithErrors();
        if (t!=NULL) {
            stateInfo.setError(t->getError());
        }
    }
    return ReportResult_Finished;
}

void GTest_Fail::init(XMLTestFormat*, const QDomElement& el) {
    msg = el.attribute("msg");
}

Task::ReportResult GTest_Fail::report() {
    stateInfo.setError(QString("Test failed: %1").arg(msg));
    return ReportResult_Finished;
}

void GTest_DeleteTmpFile::init(XMLTestFormat*, const QDomElement& el) {
    url = el.attribute("file");
    if (url.isEmpty()) {
        failMissingValue("url");
        return;
    }
    url = env->getVar("TEMP_DATA_DIR") + "/" + url;
}

Task::ReportResult GTest_DeleteTmpFile::report() {
    if (!QFile::exists(url)) {
        stateInfo.setError(QString("TMP file not found: %1").arg(url));
    } else if(!QFileInfo(url).isDir()) {
        QFile::remove(url);
    } else{
        GUrlUtils::removeDir(url, stateInfo);
    }
    return ReportResult_Finished;
}

void GTest_CreateTmpFolder::init(XMLTestFormat*, const QDomElement& el) {
    url = el.attribute("folder");
    if (url.isEmpty()) {
        failMissingValue("folder");
        return;
    }
    url = env->getVar("TEMP_DATA_DIR") + "/" + url;

}

Task::ReportResult GTest_CreateTmpFolder::report() {
    if (!QDir(url).exists()) {
        bool ok = QDir::root().mkpath(url);
        if (!ok) {
            stateInfo.setError(QString("Cannot create folder: %1").arg(url));
        }
    }
    return ReportResult_Finished;
}


}//namespace

