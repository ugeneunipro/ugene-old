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

#include "XMLTestUtils.h"

#include <U2Core/GUrlUtils.h>
#include <U2Core/U2SafePoints.h>

namespace U2 {

QList<XMLTestFactory*>  XMLTestUtils::createTestFactories() {
    QList<XMLTestFactory*> res;

    res.append(XMLMultiTest::createFactory());
    res.append(GTest_DeleteTmpFile::createFactory());
    res.append(GTest_Fail::createFactory());
    res.append(GTest_CreateTmpFolder::createFactory());

    return res;
}

void XMLTestUtils::replacePrefix(const GTestEnvironment *env, QString &path){
    QString result;

    const QString EXPECTED_OUTPUT_DIR_PREFIX = "!expected!";
    const QString TMP_DATA_DIR_PREFIX = "!tmp_data_dir!";
    const QString COMMON_DATA_DIR_PREFIX = "!common_data_dir!";

    // Determine which environment variable is required
    QString envVarName;
    QString prefix;
    if (path.startsWith(EXPECTED_OUTPUT_DIR_PREFIX)) {
        envVarName = "EXPECTED_OUTPUT_DIR";
        prefix = EXPECTED_OUTPUT_DIR_PREFIX;
    }
    else if (path.startsWith(TMP_DATA_DIR_PREFIX)) {
        envVarName = "TEMP_DATA_DIR";
        prefix = TMP_DATA_DIR_PREFIX;
    }
    else if (path.startsWith(COMMON_DATA_DIR_PREFIX)) {
        envVarName = "COMMON_DATA_DIR";
        prefix = COMMON_DATA_DIR_PREFIX;
    }
    else {
        FAIL(QString("Unexpected 'prefix' value in the path: '%1'!").arg(path), );
    }

    // Replace with the correct value
    QString prefixPath = env->getVar(envVarName);
    SAFE_POINT(!prefixPath.isEmpty(), QString("No value for environment variable '%1'!").arg(envVarName), );
    prefixPath += "/";

    int prefixSize = prefix.size();
    QStringList relativePaths = path.mid(prefixSize).split(";");

    foreach (const QString &path, relativePaths) {
        QString fullPath = prefixPath + path;
        result += fullPath + ";";
    }

    path = result.mid(0, result.size() - 1); // without the last ';'
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

