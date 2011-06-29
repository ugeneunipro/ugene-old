/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include <assert.h>

#include <QtCore/QFile>

#include <U2Core/AppContext.h>
#include "GTest.h"

namespace U2 {

/* TRANSLATOR U2::GTest */

//////////////////////////////////////////////////////////////////////////
// GTest

GTest::GTest(const QString& taskName, GTest* cp, const GTestEnvironment* _env, 
             TaskFlags flags, const QList<GTest*>& subtasks)
: Task(taskName, flags), contextProvider(cp), env(_env)
{
    assert(env!=NULL);
    foreach(Task* t, subtasks) {
        addSubTask(t);
    }
}

QObject* GTest::getContext(const QString& name) const {
    assert(contextProvider != NULL);
    return contextProvider->subtestsContext.value(name);
}

void GTest::addContext(const QString& name, QObject* v) {
    assert(contextProvider != NULL);
    assert(!contextProvider->subtestsContext.contains(name));
    contextProvider->subtestsContext[name] = v;
}

void GTest::removeContext(const QString& name) {
    assert(contextProvider != NULL);
    assert(contextProvider->subtestsContext.contains(name));
    contextProvider->subtestsContext.remove(name);
}

void GTest::failMissingValue( const QString& name) {
    stateInfo.setError(QString("Mandatory attribute not set: %1").arg(name));
}

//////////////////////////////////////////////////////////////////////////
// GTestSuite

GTestSuite::~GTestSuite() {
    qDeleteAll(tests);
    qDeleteAll(excluded);
}


static QStringList findAllFiles(const QString& dirPath, const QString& ext, bool recursive, int rec) {
    QStringList res;
    if (rec > 100) { //symlink or other err
        //todo: report err?
        return res;
    }
    QDir dir(dirPath);
    
    //add files first
    QStringList files = ext.isEmpty() ? dir.entryList(QDir::Files) : dir.entryList(ext.split(":"), QDir::Files);
    foreach(const QString& file, files) {
        QFileInfo fi(dir.absolutePath() + "/" + file);
        res.append(fi.absoluteFilePath());
    }
    
    
    //process subdirs if needed
    if (recursive) {
        QStringList subDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        foreach(QString sub, subDirs) {
            QString subDirPath = dirPath + "/" + sub;
            QStringList subRes = findAllFiles(subDirPath, ext, true, rec++);
            res+=subRes;
        }
    }
    return res;
}

static bool exclude(const QList<QRegExp>& xlist, const QString& url) {
    foreach(const QRegExp& r, xlist) {
        if (r.exactMatch(url)) {
            return true;
        }
    }
    return false;
}

static QString addExcludeTests(const QString & fullTestDirPath, const QString & str, QList<QRegExp> & xList) {
    QString err;

    if(str.isEmpty()) {
        return err;
    }

    foreach(const QString& s, str.split(",")) {
        QRegExp r(fullTestDirPath + "/" + s.trimmed(), Qt::CaseSensitive, QRegExp::Wildcard);
        if (!r.isValid()) {
            err = QString("Invalid exclude: %1").arg(s);
            break;
        }
        xList << r;
    }
    return err;
}

GTestSuite* GTestSuite::readTestSuite(const QString& url, QString& err) {
    QFile f(url);
    if (!f.open(QIODevice::ReadOnly)) {
        err = ("cant_open_file");
        return NULL;
    }
    QByteArray  xmlData = f.readAll();
    f.close();

    QDomDocument suiteDoc;
    bool res = suiteDoc.setContent(xmlData);
    if (!res) {
        err = ("not_an_xml_suite_file");
        return NULL;
    }
    
    if (suiteDoc.doctype().name()!="UGENE_TEST_FRAMEWORK_SUITE") {
        err = ("not_a_test_suite_file");
        return NULL;
    }

    QDomElement suiteEl = suiteDoc.documentElement();
    if (suiteEl.tagName()!="suite") {
        err = ("suite_elem_not_found");
        return NULL;
    }
    
    //Name 
    QString suiteName = suiteEl.attribute("name");
    if (suiteName.isEmpty()) {
        err = ("suite_name_is_empty");
        return NULL;
    }

    //Test timeout
    QString testTimeout = suiteEl.attribute("test-timeout", "0");

    //Env
    GTestEnvironment suiteEnv;
    QDomNodeList envNodes = suiteEl.elementsByTagName("env-var");
    for(int i=0;i<envNodes.size(); i++) {
        QDomNode n = envNodes.item(i);
        assert(n.isElement());
        if (!n.isElement()) {
            continue;
        }
        QDomElement varEl = n.toElement();
        QString varName = varEl.attribute("name");
        if (varName.isEmpty()) {
            err = ("var_tag_without_name");
            return NULL;
        }
        QString varVal = varEl.attribute("value");
        suiteEnv.setVar(varName, varVal);
    }



    //Tests
    QFileInfo suiteUrl(url);
    QString suiteDir = suiteUrl.absoluteDir().absolutePath();
    QList<GTestRef*> suiteTests;
    QList<GTestRef*> excluded;
    QDomNodeList testDirEls = suiteEl.elementsByTagName("test-dir");
    for(int i=0;i<testDirEls.size(); i++) {
        QDomNode n = testDirEls.item(i);
        assert(n.isElement());
        if (!n.isElement()) {
            continue;
        }
        QDomElement testDirEl = n.toElement();
        QString dirPath = testDirEl.attribute("path");
        if (dirPath.isEmpty()) {
            err = ("path_attribute_not_found");
            break;
        }
        QString fullTestDirPath = suiteDir + "/" + dirPath;
        QFileInfo testDir(fullTestDirPath);
        if (!testDir.exists()) {
            err = QString("test_dir_not_exists %1").arg(fullTestDirPath);
            break;
        }
        
        QList<QRegExp> xlist;
        err = addExcludeTests(fullTestDirPath, testDirEl.attribute("exclude"), xlist);
        if(!err.isEmpty()) {
            break;
        }
        
        if(sizeof(void*) == 8) { // means that it is 64 bit system
            err = addExcludeTests(fullTestDirPath, testDirEl.attribute("exclude_64"), xlist);
            if(!err.isEmpty()) {
                break;
            }
        }
        
        QString testFormatName = testDirEl.attribute("test-format");
        bool recursive = testDirEl.attribute("recursive") != "false";
        QString testExt = testDirEl.attribute("test-ext");
        QStringList testURLs = findAllFiles(fullTestDirPath, testExt, recursive, 0);
        foreach(const QString& tUrl, testURLs) {
            int shortNameLen = tUrl.length() - fullTestDirPath.length() - 1; // minus '/' char
            assert(shortNameLen > 0);
            QString tShortName = tUrl.right(shortNameLen);
            GTestRef* tref = new GTestRef(tUrl, tShortName, testFormatName);
            if (exclude(xlist, tUrl)) {
                excluded << tref;
            } else {
                suiteTests << tref;
            }
        }
    }
    if (!err.isEmpty()) {
        qDeleteAll(suiteTests);
        qDeleteAll(excluded);
        return NULL;
    }
    
    GTestSuite* suite = new GTestSuite();
    suite->url = suiteUrl.absoluteFilePath();
    suite->name = suiteName;
    suite->env  = suiteEnv;
    suite->tests = suiteTests;
    suite->excluded = excluded;
    // testTimeout format is unchecked, but it will fail to 0 it's ok
    suite->testTimeout = testTimeout.toInt();
    suite->testTimeout = (suite->testTimeout == 0) ? -1 : suite->testTimeout;   // -1 means timeout check disabled

    foreach( GTestRef * r, suiteTests ) {
        r->setSuite( suite );
    }
    foreach( GTestRef * r, excluded ) {
        r->setSuite( suite );
    }
    
    return suite;
}

QList<GTestSuite*> GTestSuite::readTestSuiteList( const QString& url, QStringList& errs )
{
    QList<GTestSuite*> result;
    QFile* suitListFile;
    suitListFile=new QFile(url);
    //QString dir = AppContext::getSettings()->getValue(SETTINGS_ROOT + "lastDir", QString()).toString();
    QString dir = QFileInfo(url).dir().absolutePath();
    if (suitListFile!=NULL){
        suitListFile->open(QIODevice::ReadOnly | QIODevice::Text);
        QString suiteFileContent = suitListFile->readAll();
        QStringList suiteNamesList = suiteFileContent.split(QRegExp("\\s+"));
        QString suiteName;
        foreach(suiteName,suiteNamesList){
            suiteName = suiteName.trimmed();
            if (suiteName.startsWith("#")) { //this is a comment line
                continue;
            }
            QFileInfo fifs(dir + "/" + suiteName);
            QString urlfs = fifs.absoluteFilePath();
            QString errt;
            GTestSuite* ts = GTestSuite::readTestSuite(urlfs, errt);
            if (ts == NULL) {
                assert(!errt.isEmpty());
                errs << errt;
            } else {
                result << ts;
            }
        }
    }
    return result;
}

//////////////////////////////////////////////////////////////////////////
// GTestState

void GTestState::clearState() {
    if (state == TriState_Unknown) {
        assert(errMessage.isEmpty());
        return;
    }
    errMessage.clear(); 
    state = TriState_Unknown;
    emit si_stateChanged(this);
}

void GTestState::setFailed(const QString& err) {
    if (state == TriState_No) {
        assert(!errMessage.isEmpty());
        return;
    }
    assert(!err.isEmpty());
    assert(errMessage.isEmpty());
    
    errMessage = err; 
    state = TriState_No;
    emit si_stateChanged(this);
}

void GTestState::setPassed() {
    if (state == TriState_Yes) {
        assert(errMessage.isEmpty());
        return;
    }
    errMessage.clear(); 
    state = TriState_Yes;
    emit si_stateChanged(this);
}

}//namespace
