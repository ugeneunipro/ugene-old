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

AssemblyReadsAreaHint::AssemblyReadsAreaHint(QWidget * p): QFrame(p), fromToLabel(new QLabel(this)),
lengthLabel(new QLabel(this)), cigarLabel(new QLabel(this)), strandLabel(new QLabel(this)), nameLabel(new QLabel(this)),
seqLabel(new QLabel(this)){
    QBoxLayout * top = new QVBoxLayout(this);
    top->setMargin(2);
    setLayout(top);
    top->addWidget(nameLabel);
    top->addWidget(fromToLabel);
    top->addWidget(lengthLabel);
    top->addWidget(cigarLabel);
    top->addWidget(strandLabel);
    top->addWidget(seqLabel);
    top->setSpacing(0);
    top->setSizeConstraint(QLayout::SetMinimumSize);
    
    setMaximumHeight(layout()->minimumSize().height());
    setMaximumWidth(HINT_MAX_WIDTH);
    
    installEventFilter(this);
    nameLabel->installEventFilter(this);
    fromToLabel->installEventFilter(this);
    lengthLabel->installEventFilter(this);
    cigarLabel->installEventFilter(this);
    strandLabel->installEventFilter(this);
    seqLabel->installEventFilter(this);
    
    nameLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    fromToLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    lengthLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    cigarLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    strandLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    seqLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    
    nameLabel->setWordWrap(true);
    
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

void AssemblyReadsAreaHint::setLength(qint64 len) {
    lengthLabel->setText(tr("<b>Length</b>: %1").arg(len));
}

void AssemblyReadsAreaHint::setFromTo(qint64 from, qint64 to) {
    fromToLabel->setText(tr("<b>From</b> %1 <b>to</b> %2").arg(from).arg(to));
}

void AssemblyReadsAreaHint::setCigar(const QString & ci) {
    QString cigar;
    if(ci.isEmpty()) {
        cigar = tr("no information");
    }

    for(int i = 0; i < ci.size(); ++i) {
        QChar ch = ci.at(i);
        if(ch.isNumber()) {
            cigar.append(ch);
        } else {
            cigar.append(QString("<font color='#0000FF'>%1</font>").arg(ch));
        }
    }
    cigarLabel->setText(tr("<b>Cigar</b>: %1").arg(cigar));
}

void AssemblyReadsAreaHint::setStrand(bool onCompl) {
    const QString DIRECT_STR(tr("direct"));
    const QString COMPL_STR(tr("complement"));
    strandLabel->setText(tr("<b>Strand</b>: %1").arg(onCompl ? COMPL_STR : DIRECT_STR));
}

void AssemblyReadsAreaHint::setName(const QByteArray & n) {
    assert(n.size() <= 255); // sam format
    nameLabel->setText(tr("<b>%1</b>").arg(QString(n)));
}

void AssemblyReadsAreaHint::setRawSequence(const QByteArray & s) {
    QString bytes(s);
    QString headTransl = tr("Raw sequence:&nbsp;");
    assert(headTransl.size() < 30);
    QString head = QString("<table cellspacing='0'><tr><td><b>%1</b></td>").arg(headTransl);
    QString str = head;
    const int rowSize = LETTER_MAX_COUNT - headTransl.size();
    const int ROWS_MAX_NUM = 4;
    for(int i = 0; i < ROWS_MAX_NUM; ++i) {
        QString what = QString("<td><pre>%1").arg(bytes.mid(i * rowSize, rowSize));
        if(i == ROWS_MAX_NUM - 1 && ROWS_MAX_NUM * rowSize < bytes.size()) {
            what.append("...");
        }
        what.append("</pre></td>");
        if(i == 0) {
            what.append("</tr>");
        } else {
            what.prepend("<tr><td>&nbsp;</td>");
            what.append("</tr>");
        }
        str.append(what);
        if((i + 1) * rowSize >= bytes.size()) {
            break;
        }
    }
    str.append("</table>");
    seqLabel->setText(str);
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

void AssemblyReadsAreaHint::leaveEvent(QEvent * e) {
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

void AssemblyReadsAreaHint::setData(const U2AssemblyRead& r) {
    setName(r->name);
    {
        qint64 len = U2AssemblyUtils::getEffectiveReadLength(r);
        setLength(len);
        setFromTo(r->leftmostPos + 1, r->leftmostPos + len);
    }
    setCigar(U2AssemblyUtils::cigar2String(r->cigar));
    setStrand(r->complementary);
    setRawSequence(r->readSequence);
}

} // U2
