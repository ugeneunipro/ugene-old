#ifndef _U2_EXPORT_ANNOTATIONS_2_CSV_DIALOG_H_
#define _U2_EXPORT_ANNOTATIONS_2_CSV_DIALOG_H_

#include <QtGui/QDialog>
#include <ui/ui_ExportAnnotationsDialog.h>

namespace U2 {

class ExportAnnotationsDialog : public QDialog, public Ui::ExportAnnotationsDialog {
    Q_OBJECT
    Q_DISABLE_COPY(ExportAnnotationsDialog)
public:
    static const QString CSV_FORMAT_ID;
    
public:
    explicit ExportAnnotationsDialog(const QString & filename, QWidget *parent);
    
    QString filePath()const;
    bool exportSequence()const;
    void setExportSequenceVisible(bool value);
    
    QString fileFormat()const;

protected:
    virtual void changeEvent(QEvent *e);

private slots:
    void sl_onChooseFileButtonClicked();
    void sl_onFormatChanged(const QString &);

private:
    QList<QString> supportedFormatsExts;
};

} // namespace U2

#endif 
