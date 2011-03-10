#ifndef _U2_HMMIO_H_
#define _U2_HMMIO_H_

#include <U2Core/DNAAlphabet.h>
#include <U2Core/Task.h>

#include <QtCore/QObject>
#include <QtCore/QString>

struct plan7_s;
struct msa_struct;

namespace U2 {

class TaskStateInfo;
class IOAdapterFactory;

class HMMIO : public QObject {
    Q_OBJECT
public:

    static void writeHMM2(IOAdapterFactory* iof, const QString& url, TaskStateInfo& si, plan7_s *hmm);

    static void readHMM2(IOAdapterFactory* iof, const QString& url, TaskStateInfo& si, plan7_s **ret_hmm); 

    static plan7_s * cloneHMM( plan7_s * src );

    //utility methods, TODO: move to a separate class

    static const QString HMM_ID;
    static const QString HMM_EXT;
    static QString getHMMFileFilter();

    static DNAAlphabetType convertHMMAlphabet(int hmmAtype);
};

class HMMReadTask: public Task {
    Q_OBJECT
public:
    HMMReadTask(const QString& url);
    ~HMMReadTask();
    void run();
    plan7_s* getHMM() const {return hmm;}
    const QString& getURL() const {return url;}

private:
    plan7_s*        hmm;
    QString         url;
};

class HMMWriteTask : public Task {
    Q_OBJECT
public:
    HMMWriteTask(const QString& url, plan7_s* s, uint = 0);
    virtual void run();
private:
    QString url;
    plan7_s* hmm;
    uint fileMode;
};

}//namespace

#endif
