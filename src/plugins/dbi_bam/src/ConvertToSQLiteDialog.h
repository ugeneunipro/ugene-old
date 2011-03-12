#ifndef _U2_BAM_CONVERT_TO_SQLITE_DIALOG_H_
#define _U2_BAM_CONVERT_TO_SQLITE_DIALOG_H_

#include <QtGui/QDialog>
#include <QtCore/QList>
#include <U2Core/GUrl.h>
#include "ui_ConvertToSQLiteDialog.h"

namespace U2 {
namespace BAM {

class ConvertToSQLiteDialog : public QDialog
{
    Q_OBJECT
public:
    ConvertToSQLiteDialog();

    const GUrl &getSourceUrl()const;
    const GUrl &getDestinationUrl()const;

public slots:
    virtual void accept();

private slots:
    void on_destinationUrlButton_clicked();
    void on_sourceUrlButton_clicked();

private:
    Ui::ConvertToSQLiteDialog ui;

    GUrl sourceUrl;
    GUrl destinationUrl;
};

} // namespace BAM
} // namespace U2

#endif // _U2_BAM_CONVERT_TO_SQLITE_DIALOG_H_
