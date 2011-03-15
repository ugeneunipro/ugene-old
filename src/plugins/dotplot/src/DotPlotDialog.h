/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

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
