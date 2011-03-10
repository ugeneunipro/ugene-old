#ifndef _U2_REMOVE_PART_FROM_SEQUENCE_CONTROLLER_H_
#define _U2_REMOVE_PART_FROM_SEQUENCE_CONTROLLER_H_

#include <U2Misc/DialogUtils.h>
#include <U2Core/U2Region.h>
#include <U2Core/RemovePartFromSequenceTask.h>

#include <QtGui/QDialog>

class Ui_RemovePartFromSequenceDialog;

namespace U2{

class U2GUI_EXPORT RemovePartFromSequenceDialogController : public QDialog {
    Q_OBJECT
public:
    RemovePartFromSequenceDialogController(U2Region _toDelete, U2Region _source, const QString & docUrl, QWidget *p = NULL);
    ~RemovePartFromSequenceDialogController();
    virtual void accept();

    bool modifyCurrentDocument() const;;
    QString getNewDocumentPath() const;;
    U2AnnotationUtils::AnnotationStrategyForResize getStrategy();
    U2Region getRegionToDelete() const {return toDelete;};
    bool mergeAnnotations() const;;
    DocumentFormatId getDocumentFormatId() const;;

private slots:
    void sl_browseButtonClicked();
    void sl_indexChanged(int);
    void sl_mergeAnnotationsToggled(bool);

private:
    QString filter;
    U2Region toDelete;
    U2Region source;
    Ui_RemovePartFromSequenceDialog* ui;
};

}//ns

#endif

