/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "SuggestCompleter.h"

#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>

#include <QTreeWidget>
#include <QHeaderView>
#include <QKeyEvent>

namespace U2{

BaseCompleter::BaseCompleter( CompletionFiller *filler, QLineEdit *parent /*= 0*/ )
: QObject(parent), filler(filler), editor(parent) {
    popup = new QTreeWidget;
    popup->setWindowFlags(Qt::Popup);
    popup->setFocusPolicy(Qt::NoFocus);
    popup->setFocusProxy(parent);
    popup->setMouseTracking(true);

    popup->setColumnCount(1);
    popup->setUniformRowHeights(true);
    popup->setRootIsDecorated(false);
    popup->setEditTriggers(QTreeWidget::NoEditTriggers);
    popup->setSelectionBehavior(QTreeWidget::SelectRows);
    popup->setFrameStyle(QFrame::Box | QFrame::Plain);
    popup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    popup->header()->hide();

    popup->installEventFilter(this);
    editor->installEventFilter(this);

    connect(popup, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(doneCompletion()));
    connect(editor, SIGNAL(textEdited(QString)), SLOT(sl_textEdited(QString)));
}

BaseCompleter::~BaseCompleter(){
    delete filler;
    delete popup;
}

bool BaseCompleter::eventFilter(QObject *obj, QEvent *ev){
    if (obj == editor) {
        if (QEvent::FocusOut == ev->type()) {
            QFocusEvent *fe = static_cast<QFocusEvent*>(ev);
            if (Qt::PopupFocusReason == fe->reason()) {
                return true;
            }
        }
        return false;
    }
    if (obj != popup)
        return false;

    if (ev->type() == QEvent::MouseButtonPress) {
        popup->hide();
        return false;
    }

    if (ev->type() == QEvent::KeyPress) {

        bool consumed = false;
        int key = static_cast<QKeyEvent*>(ev)->key();
        switch (key) {
         case Qt::Key_Enter:
         case Qt::Key_Return:
             doneCompletion();
             consumed = true;

         case Qt::Key_Escape:
             editor->setFocus();
             popup->hide();
             consumed = true;

         case Qt::Key_Up:
         case Qt::Key_Down:
         case Qt::Key_Home:
         case Qt::Key_End:
         case Qt::Key_PageUp:
         case Qt::Key_PageDown:
             break;

         default:
             editor->setFocus();
             editor->event(ev);
             break;
        }

        return consumed;
    }

    return false;
}

void BaseCompleter::showCompletion(const QStringList &choices){
    if (choices.isEmpty()){
        popup->hide();
        return;
    }

    popup->setUpdatesEnabled(false);
    popup->clear();
    for (int i = 0; i < choices.count(); ++i) {
        QTreeWidgetItem * item;
        item = new QTreeWidgetItem(popup);
        item->setText(0, choices[i]);
    }
    popup->setCurrentItem(popup->topLevelItem(0));
    popup->resizeColumnToContents(0);
    popup->adjustSize();
    popup->setUpdatesEnabled(true);

    int h = popup->sizeHintForRow(0) * qMin(10, choices.count()) + 3;
    popup->resize(editor->width(), h);
    popup->move(editor->mapToGlobal(QPoint(0, editor->height())));
    popup->show();
}

void BaseCompleter::doneCompletion(){
    popup->hide();
    editor->setFocus();
    QTreeWidgetItem *item = popup->currentItem();
    if (item) {
        QFileInfo f(editor->text());
        QString absPath = f.absoluteDir().absolutePath();
        if(!absPath.endsWith("/")){
            absPath.append("/");
        }
        editor->setText(absPath + item->text(0));
        emit si_editingFinished();
    }
}

void BaseCompleter::sl_textEdited( const QString& typedText){
    if (typedText.isEmpty()){
        popup->hide();
        return;
    }
    showCompletion(filler->getSuggestions(typedText));
}

QStringList FilenameCompletionFiller::getSuggestions( QString str ){
    QString fName = str;
    if(fName.endsWith(".")){
        fName = fName.left(fName.size()-1);
    }

    QStringList choices, hits;
    QFileInfo f(fName);
    QString curExt = f.suffix(), baseName = f.completeBaseName(), completeFileName = f.fileName();
    DocumentFormat *format = AppContext::getDocumentFormatRegistry()->getFormatById(fileFormat);
    QStringList formats;
    if(format){
        formats = format->getSupportedDocumentFileExtensions();
    }
    formats.append("gz");
    choices.append(completeFileName);
    foreach(QString ext, formats){
        if(!curExt.isEmpty()){
            if (ext.startsWith(curExt, Qt::CaseInsensitive)){
                choices.append(baseName + "." + ext);
                if (ext != "gz"){
                    choices.append(baseName + "." + ext + ".gz");
                }
            }
        }  
    }

    if(choices.size() == 1){
        foreach(QString ext, formats){
            choices.append(completeFileName + "." + ext);
            if (ext != "gz"){
                choices.append(completeFileName + "." + ext + ".gz");
            }
        }
    }

    return choices;
}

}