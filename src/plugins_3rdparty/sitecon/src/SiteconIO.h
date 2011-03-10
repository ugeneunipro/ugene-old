#ifndef _U2_SITECON_IO_H_
#define _U2_SITECON_IO_H_

#include <QtCore/QObject>
#include <QtCore/QString>

#include "SiteconAlgorithm.h"

#include <U2Core/Task.h>

namespace U2 {

class IOAdapterFactory;
class DiPropertySitecon;

class SiteconIO : public QObject {
    Q_OBJECT
public:

    static const QString SITECON_ID;
    static const QString SITECON_EXT;
    static QString getFileFilter(bool includeAll = true);
    static SiteconModel readModel(IOAdapterFactory* iof, const QString& url, TaskStateInfo& si);
    static void writeModel(IOAdapterFactory* iof, const QString& url, TaskStateInfo& si, const SiteconModel& model);
};

class SiteconReadTask: public Task {
    Q_OBJECT
public:
    SiteconReadTask(const QString& url) 
        : Task(tr("Read SITECON model"), TaskFlag_None), url(url) {}
    void run();
    SiteconModel getResult() const {return model;}
    QString getURL() const {return url;}
private:
    QString url;
    SiteconModel model;
};

class SiteconWriteTask : public Task {
    Q_OBJECT
public:
    SiteconWriteTask(const QString& url, const SiteconModel& model, uint f = 0) 
        : Task(tr("Save SITECON model"), TaskFlag_None), url(url), model(model), fileMode(f) {}
    virtual void run();
private:
    QString url;
    SiteconModel model;
    uint fileMode;
};


}//namespace

#endif
