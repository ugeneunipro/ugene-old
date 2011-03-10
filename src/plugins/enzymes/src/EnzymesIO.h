#ifndef _U2_ENZYMES_IO_H_
#define _U2_ENZYMES_IO_H_

#include "EnzymeModel.h"

#include <U2Core/Task.h>

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QSet>

namespace U2 {

class IOAdapterFactory;

enum EnzymeFileFormat {
    EnzymeFileFormat_Unknown,
    EnzymeFileFormat_Bairoch
};

class EnzymesIO : public QObject {
    Q_OBJECT
public:
    static QList<SEnzymeData> readEnzymes(const QString& url, TaskStateInfo& ti);
    static void writeEnzymes(const QString& url, const QString& source, const QSet<QString>& enzymes, TaskStateInfo& ti);
    static QString getFileDialogFilter();
    static QList<SEnzymeData> getDefaultEnzymesList();
    static SEnzymeData findEnzymeById(const QString& id, const QList<SEnzymeData>& enzymes);

private:
    static EnzymeFileFormat detectFileFormat(const QString& url);
    static QList<SEnzymeData> readBairochFile(const QString& url, IOAdapterFactory* iof, TaskStateInfo& ti);
    static void writeBairochFile(const QString& url, IOAdapterFactory* iof, const QString& source, IOAdapterFactory* srciof, const QSet<QString>& enzymes, TaskStateInfo& ti);
};

class LoadEnzymeFileTask : public Task {
    Q_OBJECT
public:
    LoadEnzymeFileTask(const QString& url);
    void run();
    void cleanup() {enzymes.clear();}

    QString url;
    QList<SEnzymeData> enzymes;
};

class SaveEnzymeFileTask : public Task {
    Q_OBJECT
public:
    SaveEnzymeFileTask(const QString& url, const QString& source, const QSet<QString>& enzymes);
    void run();

    QString url;
    QString source;
    QSet<QString> enzymes;
};

#define DEFAULT_ENZYMES_FILE    QString("rebase_v003_t2_com.bairoch.gz")


} //namespace

#endif
