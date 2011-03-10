#ifndef _U2_CREATE_UINDEX_DLG_H_
#define _U2_CREATE_UINDEX_DLG_H_

#include <ui/ui_CreateFileIndexDialog.h>
#include <QtGui/QDialog>

namespace U2 {

    struct CreateFileIndexDialogModel {
        QStringList inputUrls;
        QString     outputUrl;
        bool        gzippedOutput;
        bool        addToProject;
    }; // CreateFileIndexDialogModel

    class CreateFileIndexDialog : public QDialog, Ui_CreateFileIndexDialog {
        Q_OBJECT
    public:
        CreateFileIndexDialog(QWidget* p, bool enableAddToProjButton);

        CreateFileIndexDialogModel getModel() const;

        virtual void accept();

    private:
        CreateFileIndexDialogModel model;

        private slots:
            void sl_openInputFile();
            void sl_openOutputFile();
    };

} // U2

#endif
