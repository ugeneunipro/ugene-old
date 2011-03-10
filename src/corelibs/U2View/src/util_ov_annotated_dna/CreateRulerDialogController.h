#ifndef _U2_CREATE_RULER_DIALOG_CONTROLLER_H_
#define _U2_CREATE_RULER_DIALOG_CONTROLLER_H_


#include <ui/ui_CreateRulerDialog.h>

#include <U2Core/U2Region.h>

#include <QtCore/QSet>
#include <QtGui/QDialog>

namespace U2 {

class U2VIEW_EXPORT CreateRulerDialogController : public QDialog, public Ui_CreateRulerDialog {
    Q_OBJECT
public:
    CreateRulerDialogController(const QSet<QString>& namesToFilter, const U2Region& range, int offset = -1, QWidget* p=NULL);
    
    virtual void accept ();

private slots:
    void sl_colorButtonClicked();

private:
    void updateColorSample();

public:
    QString         name;
    int             offset;
    QColor          color;
    QSet<QString>   filter;

};


}// namespace

#endif
