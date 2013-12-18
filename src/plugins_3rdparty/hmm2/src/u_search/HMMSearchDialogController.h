#ifndef _U2_HMMSEARCH_DIALOG_CONTROLLER_H_
#define _U2_HMMSEARCH_DIALOG_CONTROLLER_H_

#include "uhmmsearch.h"

#include <ui/ui_HMMSearchDialog.h>

#include <U2Core/Task.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DNASequence.h>
#include <U2Core/AnnotationData.h>

#include <QtGui/QDialog>
#include <QtCore/QPointer>

struct plan7_s;

namespace U2 {

class CreateAnnotationWidgetController;
class CreateAnnotationModel;
class AnnotationTableObject;
class CreateAnnotationsTask;
class LoadUnloadedDocumentTask;
class DNATranslation;
class HMMSearchTask;
class HMMSearchTaskResult;
class U2SequenceObject;
class HMMReadTask;

class HMMSearchDialogController : public QDialog, public Ui_HMMSearchDialog {
    Q_OBJECT
public:
    HMMSearchDialogController(const U2SequenceObject* obj, QWidget* p = NULL);

public slots:
    void reject();
    
private slots:
    void sl_hmmFileClicked();
    void sl_okClicked();

	void sl_expSpinBoxValueChanged(int); 

    void sl_onStateChanged();
    void sl_onProgressChanged();

private:
    DNASequence                         dnaSequence;
    Task*                               searchTask;
    CreateAnnotationWidgetController*   createController;
};


//////////////////////////////////////////////////////////////////////////
// tasks

class HMMSearchToAnnotationsTask : public Task {
    Q_OBJECT
public:
    HMMSearchToAnnotationsTask(const QString& hmmFile, const DNASequence& s, AnnotationTableObject* aobj,
        const QString& group, const QString& aname, 
        const UHMMSearchSettings& settings);

    virtual QList<Task*> onSubTaskFinished(Task* subTask);
    QString generateReport() const;

private:

    QString                     hmmFile;
    DNASequence                 dnaSequence;
    QString                     agroup;
    QString                     aname;
    UHMMSearchSettings          settings;
    
    HMMReadTask*                readHMMTask;
    HMMSearchTask*              searchTask;
    CreateAnnotationsTask*      createAnnotationsTask;
    QPointer<AnnotationTableObject> aobj;
};

} // namespace U2

#endif
