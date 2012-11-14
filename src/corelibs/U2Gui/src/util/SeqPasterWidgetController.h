/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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
    SeqPasterWidgetController(QWidget *p = NULL, const QByteArray& initText = QByteArray(), bool needWarning = false);
    ~SeqPasterWidgetController();

    QString validate(); 
    DNASequence getSequence() const {return resultSeq;}
    void disableCustomSettings();
    void setPreferredAlphabet(DNAAlphabet *alp);
    void selectText();
    void setEventFilter(QObject* evFilter);

    static QByteArray getNormSequence(DNAAlphabet * alph, const QByteArray & seq, bool replace, QChar replaceChar);
    
private slots:
    void sl_currentindexChanged(const QString&);
private:
    DNAAlphabet *preferred;
    DNASequence resultSeq;
    Ui_SeqPasterWidget* ui;
    bool additionalWarning;
};

}//ns

#endif
