#ifndef _U2_EXPORT_UTILS_H_
#define _U2_EXPORT_UTILS_H_

#include <U2Core/global.h>


namespace U2 {

class Task;
class ExportSequencesDialog;
class ExportSequenceTaskSettings;
class AbstractExportTask;
class Annotation;

class ExportUtils: public QObject {
    Q_OBJECT
public:

    static void loadDNAExportSettingsFromDlg(ExportSequenceTaskSettings& s, const ExportSequencesDialog& d);

    static Task* wrapExportTask(AbstractExportTask* t, bool addToProject);

    // generates unique name using prefix + numbers
    static QString genUniqueName(const QSet<QString>& names, QString prefix);
    
    static Task * saveAnnotationsTask(const QString & filepath, const DocumentFormatId & format, const QList<Annotation*> & annList);
};

}//namespace

#endif
