#ifndef _U2_PROJECT_UTILS__H_
#define _U2_PROJECT_UTILS__H_

#include <U2Core/ProjectModel.h>
#include <U2Core/GAutoDeleteList.h>

class QDomDocument;

typedef QMap<QString,QString> StrMap;

namespace U2 {

    //TODO eliminate dependency on object views and move to private??

class U2GUI_EXPORT ProjectParser : public QObject {
    Q_OBJECT
public:
    virtual Project* createProjectFromXMLModel(const QString& url, const QDomDocument& xmlDoc, TaskStateInfo& si) = 0;
    virtual QString getVersion(){assert(false); return QString();};
};

class U2GUI_EXPORT ProjectParserRegistry {
public:
    ProjectParser* getProjectParserByVersion(const QString& id);
    static ProjectParserRegistry* instance();

private:
    ProjectParserRegistry(ProjectParserRegistry &ppr){Q_UNUSED(ppr); assert(0);};
    ProjectParserRegistry(){init();};
    void init();
    GAutoDeleteList<ProjectParser> parsers;
};


class ProjectParser10 : public ProjectParser {
    Q_OBJECT
public:
    ProjectParser10(){};
    Project* createProjectFromXMLModel(const QString& url, const QDomDocument& xmlDoc, TaskStateInfo& si);
    QString getVersion(){return QString("1.0");};
};

class U2GUI_EXPORT ProjectFileUtils : public QObject {
    Q_OBJECT

public:

    static void saveProjectFile(TaskStateInfo& ts, Project* p, const QString& projectUrl,   const StrMap& docUrlRemap = StrMap());

    static void loadXMLProjectModel(const QString& url, TaskStateInfo& si, QDomDocument& doc, QString& version);
};



} //namespace

#endif
