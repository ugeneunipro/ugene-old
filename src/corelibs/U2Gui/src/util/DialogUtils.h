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

#ifndef _U2_DIALOG_UTILS_H_
#define _U2_DIALOG_UTILS_H_

#include <U2Core/global.h>

#include <QtCore/QObject>
#include <QtGui/QWidget>
#include <QtGui/QLineEdit>

namespace U2 {

class DocumentFormatConstraints;
class Logger;
class TaskStateInfo;


class U2GUI_EXPORT DialogUtils : public QObject {
    Q_OBJECT
public:
    static void showProjectIsLockedWarning(QWidget* p = NULL);

    static QString prepareFileFilter(const QString& name, const QStringList& exts, bool any = true, const QStringList& extraExts = QStringList(".gz"));

    static QString prepareDocumentsFileFilter(const DocumentFormatId& fid, bool any, const QStringList& extraExts = QStringList(".gz"));

    //returns filter for all formats supported. All-docs filter is returned first if any==true
    static QString prepareDocumentsFileFilter(bool any, const QStringList& extraExts = QStringList(".gz"));

    static QString prepareDocumentsFileFilter(const DocumentFormatConstraints& c, bool any);

    static QString prepareDocumentsFileFilterByObjType(const GObjectType& t, bool any);


    static QString getLastOpenFileDir(const QString& toolType = QString(), const QString& defaultVal = QString());

    static void setLastOpenFileDir(const QString& ld, const QString& toolType = QString());
    
    static QPair<QString, QString> selectFileForScreenShot(QWidget * parent);
};

class U2GUI_EXPORT LastOpenDirHelper {
public:
    LastOpenDirHelper(const QString& domain = QString(), const QString& defaultVal = QString()); //reads dir
    virtual ~LastOpenDirHelper(); //if url is not empty -> derives dir from url and stores it
    
    operator const QString&() const {return dir;} 
    void saveURLDir2LastOpenDir();

    QString domain;

    //last used dir
    QString dir;

    // value returned by file dialog
    QString url;
};

class U2GUI_EXPORT FileLineEdit : public QLineEdit {
    Q_OBJECT
public:
    FileLineEdit(const QString& filter, const QString& type, bool multi, QWidget *parent)
        : QLineEdit(parent), FileFilter(filter), type(type), multi(multi) {}
private slots:
    void sl_onBrowse();

private:
    QString FileFilter;
    QString type;
    bool    multi;
};

} // namespace

#endif
