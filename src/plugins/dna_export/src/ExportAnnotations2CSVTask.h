#ifndef _U2_EXPORT_ANNOTATIONS_2_CSV_TASK_H_
#define _U2_EXPORT_ANNOTATIONS_2_CSV_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/DNATranslation.h>

namespace U2 {

class Annotation;

class ExportAnnotations2CSVTask : public Task {
    Q_OBJECT
public:
    ExportAnnotations2CSVTask(
        const QList<Annotation*>& annotations,
        const QByteArray& sequence,
        DNATranslation *complementTranslation,
        bool exportSequence, const QString& url, bool append = false);

    void run();
private:
    QList<Annotation*> annotations;
    QByteArray sequence;
    DNATranslation *complementTranslation;
    bool exportSequence;
    QString url;
    bool append;
};

} // namespace U2

#endif 
