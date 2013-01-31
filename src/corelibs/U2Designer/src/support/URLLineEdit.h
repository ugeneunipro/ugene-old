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

#ifndef _U2_URLLINEEDIT_H_
#define _U2_URLLINEEDIT_H_

#include <QLineEdit>

#include <U2Core/global.h>
#include <QtGui>
#include <QObject>

class QLineEdit;
class QTreeWidget;

namespace U2 {

class GSuggestCompletion;

class U2DESIGNER_EXPORT URLLineEdit : public QLineEdit {
    Q_OBJECT
public:
    URLLineEdit(const QString &filter,
                const QString &type,
                bool multi,
                bool isPath,
                bool saveFile,
                QWidget *parent,
                const QString &format = "");

    bool isMulti();
    void editingFinished();

protected:
    void focusOutEvent(QFocusEvent *event);

private slots:
    void sl_onBrowse();
    void sl_onBrowseWithAdding();

signals:
    void si_finished();
    
private:
    QString FileFilter;
    QString type;
    bool multi;
    bool isPath;
    bool saveFile;
    QString fileFormat;
    GSuggestCompletion *completer;

private:
    void browse(bool addFiles = false);
    void checkExtension(QString &name);
};

class GSuggestCompletion : public QObject{
    Q_OBJECT
public:
    GSuggestCompletion(QString fileFormat, URLLineEdit *parent = 0);
    ~GSuggestCompletion();
    bool eventFilter(QObject *obj, QEvent *ev);
    void showCompletion(const QStringList &choices);

public slots:
    void doneCompletion();
    void sl_textEdited(const QString&);

private:
    QString fileFormat;
    URLLineEdit *editor;
    QTreeWidget *popup;
};

} // U2

#endif // _U2_URLLINEEDIT_H_
