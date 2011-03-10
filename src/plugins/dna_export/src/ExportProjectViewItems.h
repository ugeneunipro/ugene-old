#ifndef _U2_EXPORT_PROJECT_VIEW_ITEMS_H_
#define _U2_EXPORT_PROJECT_VIEW_ITEMS_H_

#include <U2Core/global.h>

#include <QtGui/QAction>
#include <QtGui/QMenu>

namespace U2 {

class ExportProjectViewItemsContoller : public QObject {
    Q_OBJECT
public:
    ExportProjectViewItemsContoller(QObject* p);

private slots:
    void sl_addToProjectViewMenu(QMenu&);

    void sl_saveSequencesToSequenceFormat();
    void sl_saveSequencesAsAlignment();
    void sl_saveAlignmentAsSequences();
    void sl_exportNucleicAlignmentToAmino();
    void sl_importAnnotationsFromCSV();
    void sl_exportChromatogramToSCF();
    void sl_exportAnnotationsToCSV();

private:
    void addExportMenu(QMenu& m);
    void addImportMenu(QMenu& m);

    QAction* exportSequencesToSequenceFormatAction;
    QAction* exportSequencesAsAlignmentAction;
    QAction* exportAlignmentAsSequencesAction;
    QAction* exportNucleicAlignmentToAminoAction;
    QAction* importAnnotationsFromCSVAction;
    QAction* exportDNAChromatogramAction;
    QAction* exportAnnotations2CSV;

};

}//namespace

#endif
