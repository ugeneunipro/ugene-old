#ifndef _U2_TEXT_EDITOR_DIALOG_H_
#define _U2_TEXT_EDITOR_DIALOG_H_

#include <U2Core/global.h>
#include <QtGui/QDialog>

class Ui_TextEditorDialog;

namespace U2 {

class U2MISC_EXPORT TextEditorDialog : public QDialog {
    Q_OBJECT
public:
    TextEditorDialog(QWidget* parent, const QString& title, const QString& label, const QString& text, bool acceptOnEnter);
    ~TextEditorDialog();
    QString getText();

protected:
    bool eventFilter(QObject *, QEvent *);

private:
    Ui_TextEditorDialog* ui;
};

} // namespace

#endif
