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

class U2GUI_EXPORT BaseCompleter : public QObject{
    Q_OBJECT
public:
    BaseCompleter(QString fileFormat, QLineEdit *parent = 0);
    ~BaseCompleter();
    bool eventFilter(QObject *obj, QEvent *ev);
    void showCompletion(const QStringList &choices);
signals:
    void si_editingFinished();

protected slots:
    void doneCompletion();
    virtual void sl_textEdited(const QString&) = 0;

protected:
    QString fileFormat;
    QLineEdit *editor;
    QTreeWidget *popup;
};

class U2GUI_EXPORT FilenameSuggestCompletion : public BaseCompleter{
    Q_OBJECT
public:
    FilenameSuggestCompletion(QString fileFormat, QLineEdit *parent /* = 0 */):BaseCompleter(fileFormat, parent){};
    ~FilenameSuggestCompletion(){};

protected slots:
    void sl_textEdited(const QString&);

private:
    QString fileFormat;
};

} // U2

#endif // _U2_URLLINEEDIT_H_