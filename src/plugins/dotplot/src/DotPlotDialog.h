#ifndef _U2_DOT_PLOT_DIALOG_H_
#define _U2_DOT_PLOT_DIALOG_H_

#include <ui/ui_DotPlotDialog.h>

#include <U2Algorithm/RepeatFinderSettings.h>
#include <QtGui/QDialog>

namespace U2 {
class ADVSequenceObjectContext;

class DotPlotDialog : public QDialog, public Ui_DotPlotDialog{
    Q_OBJECT
public:
    DotPlotDialog(QWidget *parent, const QList<ADVSequenceObjectContext *>& sequences, int minLen, int identity, ADVSequenceObjectContext *seqX, ADVSequenceObjectContext *seqY, bool dir, bool inv, const QColor &dColor = QColor(), const QColor &iColor = QColor());

    virtual void accept();

    ADVSequenceObjectContext    *getXSeq() const {return xSeq;}
    ADVSequenceObjectContext    *getYSeq() const {return ySeq;}

    int getMinLen() const;
    int getMismatches() const;
    RFAlgorithm getAlgo() const;

    bool isDirect() const;
    bool isInverted() const;

    const QColor& getDirectColor() const {return directColor;}
    const QColor& getInvertedColor() const {return invertedColor;}

protected slots:
    void sl_minLenHeuristics();
    void sl_hundredPercent();

    void sl_directInvertedCheckBox();

    void sl_directColorButton();
    void sl_invertedColorButton();

    void sl_directDefaultColorButton();
    void sl_invertedDefaultColorButton();

private:
    QList<ADVSequenceObjectContext *> sequences;
    ADVSequenceObjectContext *xSeq, *ySeq;

    QColor directColor, invertedColor;

    void updateColors();
};

} //namespace

#endif
