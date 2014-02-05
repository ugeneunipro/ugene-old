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

#ifndef _U2_MSA_ALIGN_DIALOG_H_
#define _U2_MSA_ALIGN_DIALOG_H_

#include <U2Core/global.h>
#include <QtGui/QDialog>

class Ui_performAlignmentDialog;

namespace U2 {

class MSAAlignAlgRegistry;
class MSAAlignAlgorithmMainWidget;
class MSAAlignAlgorithmEnv;

class U2VIEW_EXPORT MSAAlignDialog: public QDialog {
    Q_OBJECT

public:
    MSAAlignDialog(const QString& algorithmName, bool useFileMenu, QWidget* p = NULL);
    const QString getAlgorithmName();
    QMap<QString,QVariant> getCustomSettings();
    const QString getFileName();

private slots:    
    void sl_onFileNameButtonClicked();

private:
    Ui_performAlignmentDialog* ui;
    const QString& algorithmName;
    MSAAlignAlgorithmMainWidget* customGUI;
    MSAAlignAlgorithmEnv* algoEnv;
    static QString alignmentPath;
    bool openFileMode;
    void addGuiExtension();
    void accept();
};

} // namespace

#endif //  _U2_MSA_ALIGN_DIALOG_H_
