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

#ifndef _U2_URLLINEEDIT_H_
#define _U2_URLLINEEDIT_H_

#include <QLineEdit>

#include <U2Core/global.h>

namespace U2 {

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

protected:
    void focusOutEvent(QFocusEvent *event);

private slots:
    void sl_onBrowse();
    void sl_onBrowseWithAdding();
    void sl_editingFinished();

signals:
    void si_finished();
    
private:
    QString FileFilter;
    QString type;
    bool multi;
    bool isPath;
    bool saveFile;
    QString fileFormat;

private:
    void browse(bool addFiles = false);
    void checkExtension(QString &name);
};

} // U2

#endif // _U2_URLLINEEDIT_H_
