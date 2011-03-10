#include "XMLTestUtils.h"

namespace U2 {

QList<XMLTestFactory*>  XMLTestUtils::createTestFactories() {
    QList<XMLTestFactory*> res;
    
    res.append(XMLMultiTest::createFactory());
    res.append(GTest_DeleteTmpFile::createFactory());
    res.append(GTest_Fail::createFactory());

    return res;
}

void XMLMultiTest::init(XMLTestFormat *tf, const QDomElement& el) {
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
    if (!hasErrors()) {
        foreach(Task* t, subs) {
            addSubTask(t);
        }
    }
}


Task::ReportResult XMLMultiTest::report() {
    if (!hasErrors()) {
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
    }
    return ReportResult_Finished;
}


}//namespace

