#ifndef _U2_EXPORT_MSA_TO_MSA_DIALOG_H_
#define _U2_EXPORT_MSA_TO_MSA_DIALOG_H_

#include <U2Core/global.h>

#include <QtGui/QDialog>
#include <ui/ui_ExportMSA2MSADialog.h>

namespace U2 {

class SaveDocumentGroupController;

class ExportMSA2MSADialog : public QDialog, Ui_ExportMSA2MSADialog {
    Q_OBJECT
public:
    ExportMSA2MSADialog(const QString& defaultFileName, const DocumentFormatId& f, bool wholeAlignmentOnly, QWidget* p);

    void updateModel();
    DocumentFormatId formatId;
    QString file;
    bool addToProjectFlag;
    QString translationTable;
    bool exportWholeAlignment;

private slots:
    void sl_exportClicked();

private:
    SaveDocumentGroupController* saveContoller;
    QList<QString> tableID;
};

}//namespace

#endif
