#ifndef _U2_RANGE_SELECTOR_H_
#define _U2_RANGE_SELECTOR_H_ 

#include <U2Core/global.h>

#include <QtGui/QLineEdit>
#include <QtGui/QDialog>

namespace U2 {

class U2MISC_EXPORT RangeSelector : public QWidget {
    Q_OBJECT
public:
    RangeSelector(QWidget* p, int rangeStart, int rangeEnd);
    RangeSelector(QDialog* d, int rangeStart, int rangeEnd, int len, bool autoclose);

    ~RangeSelector();
    int getStart() const;
    int getEnd() const;

signals:
    void si_rangeChanged(int startPos, int endPos);

private slots:
    void sl_onGoButtonClicked(bool);
    void sl_onMinButtonClicked(bool);
    void sl_onMaxButtonClicked(bool);
    void sl_onReturnPressed();

private:
    void init();
    void exec();

    int         rangeStart;
    int         rangeEnd;
    int         len;
    QLineEdit*  startEdit;
    QLineEdit*  endEdit;
    QDialog*    dialog;
    bool        autoclose;
};

}//namespace

#endif
