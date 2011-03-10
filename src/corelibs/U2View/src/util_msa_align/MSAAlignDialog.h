#ifndef _U2_MSA_ALIGN_DIALOG_H_
#define _U2_MSA_ALIGN_DIALOG_H_

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QFrame>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>

#include <U2Core/GUrl.h>

namespace U2 {

class MSAAlignAlgRegistry;
class MSAAlignAlgorithmMainWidget;
class MSAAlignAlgorithmEnv;

class U2VIEW_EXPORT MSAAlignDialog: public QDialog {
    Q_OBJECT

public:
    MSAAlignDialog(const QString& algorithmName, QWidget* p = NULL);
    const QString getResultFileName();
    const QString getAlgorithmName();
    QMap<QString,QVariant> getCustomSettings();

protected:
    QVBoxLayout *verticalLayout;
    QGroupBox *fileSelectBox;
    QHBoxLayout *horizontalLayout;
    QLineEdit *resultFileNameEdit;
    QPushButton *selectFileNameButton;
    QSpacerItem *verticalSpacer;
    QFrame *buttonFrame;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer;
    QPushButton *alignButton;
    QPushButton *cancelButton;

private slots:    
    void sl_onFileNameButtonClicked();

private:
    const QString& algorithmName;
    MSAAlignAlgorithmMainWidget* customGUI;
    MSAAlignAlgorithmEnv* algoEnv;
    static QString alignmentPath;    
    void addGuiExtension();
    void accept();
    void setupUi(QDialog *dialog);
};

} // namespace

#endif //  _U2_MSA_ALIGN_DIALOG_H_
