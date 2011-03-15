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

#ifndef _U2_MSA_ALIGN_DIALOG_H_
#define _U2_MSA_ALIGN_DIALOG_H_

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QFrame>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>

#include <U2Core/GUrl.h>

namespace U2 {

class MSAAlignAlgRegistry;
class MSAAlignAlgorithmMainWidget;
class MSAAlignAlgorithmEnv;

class U2VIEW_EXPORT MSAAlignDialog: public QDialog {
    Q_OBJECT

public:
    MSAAlignDialog(const QString& algorithmName, QWidget* p = NULL);
    const QString getResultFileName();
    const QString getAlgorithmName();
    QMap<QString,QVariant> getCustomSettings();

protected:
    QVBoxLayout *verticalLayout;
    QGroupBox *fileSelectBox;
    QHBoxLayout *horizontalLayout;
    QLineEdit *resultFileNameEdit;
    QPushButton *selectFileNameButton;
    QSpacerItem *verticalSpacer;
    QFrame *buttonFrame;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer;
    QPushButton *alignButton;
    QPushButton *cancelButton;

private slots:    
    void sl_onFileNameButtonClicked();

private:
    const QString& algorithmName;
    MSAAlignAlgorithmMainWidget* customGUI;
    MSAAlignAlgorithmEnv* algoEnv;
    static QString alignmentPath;    
    void addGuiExtension();
    void accept();
    void setupUi(QDialog *dialog);
};

} // namespace

#endif //  _U2_MSA_ALIGN_DIALOG_H_
