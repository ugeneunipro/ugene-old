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

#ifndef _U2_SUGGESTCOMPLETER_H_
#define _U2_SUGGESTCOMPLETER_H_

#include <U2Core/global.h>
#include <QLineEdit>

class QTreeWidget;

namespace U2 {

class U2GUI_EXPORT CompletionFiller{
public:
    ~CompletionFiller(){};
    virtual QStringList getSuggestions(const QString &str) = 0;
};

class U2GUI_EXPORT FilenameCompletionFiller : public CompletionFiller {
public:
    FilenameCompletionFiller(const QString &_fileFormat):CompletionFiller(),fileFormat(_fileFormat){};

    QStringList getSuggestions(const QString &str);
private:
    const QString fileFormat;
};

class U2GUI_EXPORT MSACompletionFiller : public CompletionFiller {
public:
    MSACompletionFiller():CompletionFiller(),seqNameList(QStringList()), defaultValue(""){};
    MSACompletionFiller(QStringList &_seqNameList, const QString &defVal = ""):CompletionFiller(),seqNameList(_seqNameList), defaultValue(defVal){};

    QStringList getSuggestions(const QString &str);
    void updateSeqList(QStringList &list){seqNameList = list;};
private:
    QStringList seqNameList;
    const QString defaultValue;
};

class U2GUI_EXPORT BaseCompleter : public QObject{
    Q_OBJECT
public:
    BaseCompleter(CompletionFiller *filler, QLineEdit *parent = 0);
    ~BaseCompleter();
    bool eventFilter(QObject *obj, QEvent *ev);
    void showCompletion(const QStringList &choices);
signals:
    void si_editingFinished();

protected slots:
    void doneCompletion();
    void sl_textEdited(const QString&);

private:
    CompletionFiller *filler;
    QLineEdit *editor;
    QTreeWidget *popup;
};

} // U2

#endif // _U2_URLLINEEDIT_H_