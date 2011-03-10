#include "ExportUtils.h"

#include "ExportSequencesDialog.h"
#include "ExportSequenceTask.h"

namespace U2 {

void ExportUtils::loadDNAExportSettingsFromDlg(ExportSequenceTaskSettings& s, const ExportSequencesDialog& d)  {
    s.fileName = d.file;
    s.merge = d.merge;
    s.mergeGap = d.mergeGap;
    s.allAminoFrames = d.translateAllFrames;
    s.strand = d.strand;
    s.formatId = d.formatId;
    s.mostProbable = d.mostProbable;
    s.saveAnnotations = d.withAnnotations;
}

Task* ExportUtils::wrapExportTask(AbstractExportTask* t, bool addToProject) {
    if (!addToProject) {
        return t;
    }
    return new AddDocumentAndOpenViewTask(t);
}



QString ExportUtils::genUniqueName(const QSet<QString>& names, QString prefix) {
    if (!names.contains(prefix)) {
        return prefix;
    }
    QString name = prefix;
    int i=0;
    do {
        if (!names.contains(name)) {
            break;
        }
        name = prefix + "_" + QString::number(++i);
    } while(true);
    return name;
}

}//namespace
