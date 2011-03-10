#ifndef _U2_EXPORT_ANNOTATIONS_2_CSV_DIALOG_H_
#define _U2_EXPORT_ANNOTATIONS_2_CSV_DIALOG_H_

#include <QtGui/QDialog>

namespace U2 {

namespace Ui {
    class ExportAnnotations2CSVDialog;
}

class ExportAnnotations2CSVDialog : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(ExportAnnotations2CSVDialog)
public:
    explicit ExportAnnotations2CSVDialog(QWidget *parent);
    virtual ~ExportAnnotations2CSVDialog();

    QString getFileName()const;
    void setFileName(const QString& value);

    bool getExportSequence()const;
    void setExportSequence(bool value);
    void setExportSequenceEnabled(bool value);

protected:
    virtual void changeEvent(QEvent *e);

private:
    Ui::ExportAnnotations2CSVDialog *m_ui;

private slots:
    void on_chooseFileButton_clicked();
};

} // namespace U2

#endif 
