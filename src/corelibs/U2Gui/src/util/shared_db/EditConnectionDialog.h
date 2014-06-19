/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#ifndef _U2_EDIT_CONNECTION_DIALOG_H_
#define _U2_EDIT_CONNECTION_DIALOG_H_

#include <QtGui/QDialog>

#include <U2Core/U2DbiUtils.h>

namespace Ui {
class EditConnectionDialog;
}

namespace U2 {

class AuthenticationWidget;

class EditConnectionDialog : public QDialog {
    Q_DISABLE_COPY(EditConnectionDialog)
    Q_OBJECT
public:
    EditConnectionDialog(QWidget *parent = 0, const U2DbiId &dbiUrl = QString(), const QString &name = QString());
    ~EditConnectionDialog();

    QString     getName()       const;
    U2DbiId     getDbUrl()      const;

public slots:
    void accept();
    
private:
    void init(const U2DbiId &dbUrl, const QString &name);
    void initTabOrder();
    void saveCredentials() const;
    void removeCredentials() const;
    bool checkFields();

    Ui::EditConnectionDialog *ui;

    static const QString DEFAULT_PORT;
};

}   // namespace U2

#endif // _U2_EDIT_CONNECTION_DIALOG_H_
