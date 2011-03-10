#ifndef _U2_SEQ_PASTER_WIDGET_CONTROLLER_H_
#define _U2_SEQ_PASTER_WIDGET_CONTROLLER_H_

#include <U2Core/global.h>
#include <U2Core/DNASequence.h>

#include <QtGui/QWidget>

class Ui_SeqPasterWidget;

namespace U2{

class U2GUI_EXPORT SeqPasterWidgetController : public QWidget {
    Q_OBJECT
public:
    SeqPasterWidgetController(QWidget *p = NULL, const QByteArray& initText = QByteArray());
    ~SeqPasterWidgetController();

    QString validate(); 
    DNASequence getSequence() const {return resultSeq;}
    void disableCustomSettings();
    void setPreferredAlphabet(DNAAlphabet *alp);

    static QByteArray getNormSequence(DNAAlphabet * alph, const QByteArray & seq, bool replace, QChar replaceChar);
    
private slots:
    void sl_currentindexChanged(const QString&);
private:
    DNAAlphabet *preferred;
    DNASequence resultSeq;
    Ui_SeqPasterWidget* ui;
};

}//ns

#endif
