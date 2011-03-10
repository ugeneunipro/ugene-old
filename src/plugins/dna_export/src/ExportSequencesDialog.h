#ifndef _U2_EXPORT_SEQUENCES_DIALOG_H_
#define _U2_EXPORT_SEQUENCES_DIALOG_H_

#include <U2Core/global.h>

#include <QtGui/QDialog>
#include <ui/ui_ExportSequencesDialog.h>

namespace U2 {

class SaveDocumentGroupController;

class ExportSequencesDialog : public QDialog, Ui_ExportSequencesDialog {
    Q_OBJECT
public:
    ExportSequencesDialog(bool multiMode, bool allowComplement, bool allowTranslation, bool allowBackTranslation, const QString& defaultFileName, const DocumentFormatId& f, QWidget* p);

    void updateModel();
    bool multiMode;
    DocumentFormatId formatId;
    QString file;
    
    TriState strand;//Yes-> direct, No->complement, Unkn -> both
    bool translate;
    bool translateAllFrames;
    bool useSpecificTable;
    QString translationTable;

    bool backTranslate;
    bool mostProbable;

    bool merge;
    bool addToProject;
    int mergeGap;

    bool withAnnotations;

    void disableAllFramesOption(bool v);
    void disableStrandOption(bool v);
    void disableAnnotationsOption(bool v);

private slots:
    void sl_exportClicked();
    void sl_translationTableEnabler();
    void sl_formatChanged(int);

private:
    SaveDocumentGroupController* saveGroupContoller;
    QList<QString> tableID;
};

}//namespace

#endif
