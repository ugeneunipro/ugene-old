#ifndef _U2_POSITION_SELECTOR_H_
#define _U2_POSITION_SELECTOR_H_

#include <U2Core/global.h>

#include <QtGui/QLineEdit>
#include <QtGui/QValidator>
#include <QtGui/QDialog>

namespace U2 {

class U2MISC_EXPORT PositionSelector : public QWidget {
    Q_OBJECT
public:
    PositionSelector(QWidget* p, int rangeStart, int rangeEnd);
    PositionSelector(QDialog* d, int rangeStart, int rangeEnd, bool autoclose);

    ~PositionSelector();
    
    QLineEdit* getPosEdit() const {return posEdit;}

signals:
    void si_positionChanged(int pos);

private slots:
    void sl_onButtonClicked(bool);
    void sl_onReturnPressed();

private:
    void init();
    void exec();

    int rangeStart;
    int rangeEnd;
    QLineEdit* posEdit;
    bool autoclose;
    QDialog* dialog;
};

}//namespace

#endif
