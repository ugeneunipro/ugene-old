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

#include "AssemblyReadsAreaHint.h"
#include "AssemblyReadsArea.h"
#include "ShortReadIterator.h"

#include <U2Core/U2AssemblyUtils.h>

#include <QtGui/QBoxLayout>
#include <QtGui/QMouseEvent>
#include <QtGui/QApplication>

namespace U2 {

const QPoint AssemblyReadsAreaHint::OFFSET_FROM_CURSOR(13, 13);
static const int HINT_MAX_WIDTH = 200;

AssemblyReadsAreaHint::AssemblyReadsAreaHint(QWidget * p): QFrame(p), label(new QLabel(this)) {
    QBoxLayout * top = new QVBoxLayout(this);
    top->setMargin(2);
    setLayout(top);
    top->addWidget(label);
    top->setSpacing(0);
    top->setSizeConstraint(QLayout::SetMinimumSize);
    
    setMaximumHeight(layout()->minimumSize().height());
    setMaximumWidth(HINT_MAX_WIDTH);
    
    installEventFilter(this);
    label->installEventFilter(this);
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    
    {
        QPalette p(palette());
        p.setColor(QPalette::Background, QColor(245, 245, 206));
        setPalette(p);
    }
    
// hack: tooltip do not works on linux, popup do not work on windows
#ifdef Q_OS_WIN
    setWindowFlags(Qt::ToolTip);
#else
    setWindowFlags(Qt::Popup);
#endif
    setWindowOpacity(0.8);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    setMouseTracking(true);
    setLineWidth(1);
    setFrameShape(QFrame::Box);
}

static QString getCigarString(const QString & ci) {
    if(ci.isEmpty()) {
        return AssemblyReadsAreaHint::tr("no information");
    }
    
    QString cigar;
    for(int i = 0; i < ci.size(); ++i) {
        QChar ch = ci.at(i);
        if(ch.isNumber()) {
            cigar.append(ch);
        } else {
            cigar.append(QString("<b>%1 </b>").arg(ch));
        }
    }
    return cigar;
}

static const QString DIRECT_STR(AssemblyReadsAreaHint::tr("direct"));
static const QString COMPL_STR(AssemblyReadsAreaHint::tr("complement"));

QString getReadSequence(const QByteArray & bytes) {
    QString ret(bytes);
    if(ret.size() < AssemblyReadsAreaHint::LETTER_MAX_COUNT) {
        return ret;
    }
    return ret.mid(0, AssemblyReadsAreaHint::LETTER_MAX_COUNT) + "...";
}

QString AssemblyReadsAreaHint::getReadDataAsString(const U2AssemblyRead & r) {
    QString ret;
    ret += QString("> %1\n").arg(QString(r->name));
    ret += QString("%1\n\n").arg(QString(r->readSequence));
    {
        qint64 len = U2AssemblyUtils::getEffectiveReadLength(r);
        ret += QString("From %1 to %2\n").arg(r->leftmostPos + 1).arg(r->leftmostPos + len);
        ret += QString("Length: %1\n").arg(len);
        ret += QString("Row: %1\n").arg(r->packedViewRow + 1);
    }
    ret += QString("Cigar: %1\n").arg(QString(U2AssemblyUtils::cigar2String(r->cigar)));
    {
        bool onCompl = ReadFlagsUtils::isComplementaryRead(r->flags);
        ret += QString("Strand: %1\n").arg(onCompl ? COMPL_STR : DIRECT_STR);
    }
    if(ReadFlagsUtils::isUnmappedRead(r->flags)) {
        ret += "Unmapped\n";
    }
    return ret;
}

QString getReadNameWrapped(QString n) {
    QString ret;
    while(!n.isEmpty()) {
        n = n.trimmed();
        if(n.size() > AssemblyReadsAreaHint::LETTER_MAX_COUNT) {
            QString sub = n.mid(0, AssemblyReadsAreaHint::LETTER_MAX_COUNT);
            int pos = sub.lastIndexOf(QRegExp("\\s+"));
            if(pos == -1) {
                pos = sub.size();
            } 
            ret += sub.mid(0, pos) + "<br>";
            n = n.mid(pos);
        } else {
            ret += n;
            n.clear();
        }
    }
    return ret;
}

void AssemblyReadsAreaHint::setData(const U2AssemblyRead& r) {
    QString text;
    text += "<table cellspacing=\"0\" cellpadding=\"0\" align=\"left\" width=\"20%\">";
    text += QString("<tr><td><b>%1</b></td></tr>").arg(getReadNameWrapped(r->name));
    {
        qint64 len = U2AssemblyUtils::getEffectiveReadLength(r);
        text += QString("<tr><td><b>From</b>&nbsp;%1&nbsp;<b>to</b>&nbsp;%2&nbsp;<b>Row</b>:&nbsp;%3</td></tr>").
            arg(r->leftmostPos + 1).
            arg(r->leftmostPos + len).
            arg(r->packedViewRow + 1);
        text += QString("<tr><td><b>Length</b>:&nbsp;%1</td></tr>").arg(len);
    }
    text += QString("<tr><td><b>Cigar</b>:&nbsp;%1</td></tr>").arg(getCigarString(U2AssemblyUtils::cigar2String(r->cigar)));
    {
        bool onCompl = ReadFlagsUtils::isComplementaryRead(r->flags);
        text += QString("<tr><td><b>Strand</b>:&nbsp;%1</td></tr>").arg(onCompl ? COMPL_STR : DIRECT_STR);
    }
    text += QString("<tr><td><b>Read sequence</b>:&nbsp;%1</td></tr>").arg(getReadSequence(r->readSequence));
    if(ReadFlagsUtils::isUnmappedRead(r->flags)) {
        text += QString("<tr><td><b><font color=\"red\">%1</font></b></td></tr>").arg(tr("Unmapped"));
    }
    text += "</table>";
    label->setText(text);
    setMaximumHeight(layout()->minimumSize().height());
}

bool AssemblyReadsAreaHint::eventFilter(QObject *, QEvent * event) {
    QMouseEvent * e = dynamic_cast<QMouseEvent*>(event);
    if(e != NULL) {
        QWidget * p = qobject_cast<QWidget*>(parent());
        QMouseEvent eventToParent(e->type(), p->mapFromGlobal(QCursor::pos()), e->button(), e->buttons(), e->modifiers());
        QApplication::sendEvent(p, &eventToParent);
        return true;
    } else {
        return false;
    }
}

void AssemblyReadsAreaHint::leaveEvent(QEvent *) {
    AssemblyReadsArea * p = qobject_cast<AssemblyReadsArea*>(parent());
    QPoint curInParentCoords = p->mapFromGlobal(QCursor::pos());
    if(!p->rect().contains(curInParentCoords)) {
        p->sl_hideHint();
    }
}

void AssemblyReadsAreaHint::mouseMoveEvent(QMouseEvent * e) {
    AssemblyReadsArea * p = qobject_cast<AssemblyReadsArea*>(parent());
    p->sl_hideHint();
    QFrame::mouseMoveEvent(e);
}

} // U2
