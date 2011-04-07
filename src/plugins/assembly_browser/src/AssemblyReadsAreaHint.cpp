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

#include <QtGui/QBoxLayout>
#include <QtGui/QMouseEvent>
#include <QtGui/QApplication>

namespace U2 {

const QPoint AssemblyReadsAreaHint::OFFSET_FROM_CURSOR(13, 13);
static const int HINT_MAX_WIDTH = 200;

AssemblyReadsAreaHint::AssemblyReadsAreaHint(QWidget * p): QFrame(p), fromToLabel(new QLabel(this)),
    lengthLabel(new QLabel(this)), cigarLabel(new QLabel(this)), strandLabel(new QLabel(this)) {
        QBoxLayout * top = new QVBoxLayout(this);
        top->setMargin(2);
        setLayout(top);
        top->addWidget(fromToLabel);
        top->addWidget(lengthLabel);
        top->addWidget(cigarLabel);
        top->addWidget(strandLabel);

        installEventFilter(this);
        fromToLabel->installEventFilter(this);
        lengthLabel->installEventFilter(this);
        cigarLabel->installEventFilter(this);
        strandLabel->installEventFilter(this);

        {
            QPalette p(palette());
            p.setColor(QPalette::Background, QColor(245, 245, 206));
            setPalette(p);
        }

        setWindowFlags(Qt::ToolTip);
        setWindowOpacity(0.8);
        setMaximumHeight(layout()->minimumSize().height());
        setMaximumWidth(HINT_MAX_WIDTH);
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
    QWidget * p = qobject_cast<QWidget*>(parent());
    QPoint curInParentCoords = p->mapFromGlobal(QCursor::pos());
    if(!p->rect().contains(curInParentCoords)) {
        hide();
    }
}

} // U2
