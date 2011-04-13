#ifndef _U2_DNA_SEQUENCE_GENERATOR_DIALOG_H_
#define _U2_DNA_SEQUENCE_GENERATOR_DIALOG_H_

#include "ui_DNASequenceGeneratorDialog.h"
#include "ui_BaseContentDialog.h"


namespace U2 {

class SaveDocumentGroupController;

class DNASequenceGeneratorDialog : public QDialog, public Ui_DNASequenceGeneratorDialog {
    Q_OBJECT
public:
    DNASequenceGeneratorDialog(QWidget* p=NULL);
private slots:
    void sl_browseReference();
    void sl_configureContent();
    void sl_generate();
    void sl_refButtonToggled(bool checked);
private:
    SaveDocumentGroupController* saveGroupContoller;
    static QMap<char, qreal> content;
};

class BaseContentDialog : public QDialog, public Ui_BaseContentDialog {
    Q_OBJECT
public:
    BaseContentDialog(QMap<char, qreal>& percentMap_, QWidget* p=NULL);
private slots:
    void sl_save();
private:
    QMap<char, qreal>& percentMap;
};

} //namespace

#endif
