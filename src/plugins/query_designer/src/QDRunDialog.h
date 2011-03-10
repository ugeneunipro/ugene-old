#ifndef _U2_QD_RUN_DIALOG_H_
#define _U2_QD_RUN_DIALOG_H_

#include <ui/ui_RunQueryDialog.h>
#include <ui/ui_QDDialog.h>

#include <U2Core/Task.h>
#include <U2Core/U2Region.h>


namespace U2 {

class QDScheme;
class Document;
class QDScheduler;
class SaveDocumentTask;
class AnnotationTableObject;

class QDRunDialogTask : public Task {
    Q_OBJECT
public:
    QDRunDialogTask(QDScheme* scheme, const QString& inUri, const QString& outUri, bool addToProject);
protected:
    virtual QList<Task*> onSubTaskFinished(Task* subTask);
private slots:
    void sl_updateProgress();
private:
    QList<Task*> init();
    void setupQuery(Document* doc);
private:
    QDScheme* scheme;
    QString inUri;
    QString output;
    bool addToProject;
    Task* openProjTask;
    Task* loadTask;
    QDScheduler* scheduler;
    Document* inDoc;
    AnnotationTableObject* ato;
};

class QueryViewController;

class QDRunDialog : public QDialog, public Ui_RunQueryDlg {
    Q_OBJECT
public:
    QDRunDialog(QDScheme* _scheme, QWidget* parent, const QString& defaultIn = QString(), const QString& defaultOut = QString());
private slots:
    void sl_run();
    void sl_selectFile();
private:
    QDScheme* scheme;
};

class ADVSequenceObjectContext;
class CreateAnnotationWidgetController;

class QDDialog : public QDialog, public Ui_QDDialog {
    Q_OBJECT
public:
    QDDialog(ADVSequenceObjectContext* ctx);
private:
    void addAnnotationsWidget();
    void connectGUI();
    void setParameters();
    QVector<U2Region> getLocation() const;
private slots:
    void sl_selectScheme();
    void sl_radioCustomRangeToggled(bool checked);
    void sl_rangeStartChanged(int i);
    void sl_rangeEndChanged(int i);
    void sl_okBtnClicked();
private:
    ADVSequenceObjectContext* ctx;
    CreateAnnotationWidgetController* cawc;
    QDScheme* scheme;
    QTextDocument* txtDoc;
};

}//namespace

#endif
