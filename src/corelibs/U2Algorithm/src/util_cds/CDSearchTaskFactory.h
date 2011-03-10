#ifndef _U2_CDS_TASK_FACTORY_H_
#define _U2_CDS_TASK_FACTORY_H_

#include <U2Core/AnnotationData.h>


namespace U2 {

class U2ALGORITHM_EXPORT CDDNames {
public:
    static QString CDD_DB() { return "CDD"; }
    static QString PFAM_DB() { return "Pfam"; }
    static QString SMART_DB() { return "Smart"; }
    static QString COG_DB() { return "Cog"; }
    static QString KOG_DB() { return "Kog"; }
    static QString PRK_DB() { return "Prk"; }
    static QString TIGR_DB() { return "Tigr"; }
};

class DNAAlphabet;

class U2ALGORITHM_EXPORT CDSearchSettings {        
public:
    CDSearchSettings() : ev(0.01f), alp(NULL) {}
    float ev;
    DNAAlphabet* alp;
    QByteArray query;
    QString localDbFolder;
    QString dbName;
};

class Task;

class U2ALGORITHM_EXPORT CDSearchResultListener {
public:
    virtual QList<SharedAnnotationData> getCDSResults() const = 0;
    virtual Task* getTask() const = 0;
};

class U2ALGORITHM_EXPORT CDSearchFactory {
public:
    virtual CDSearchResultListener* createCDSearch(const CDSearchSettings& settings) const = 0;
};

} //namespace

#endif
