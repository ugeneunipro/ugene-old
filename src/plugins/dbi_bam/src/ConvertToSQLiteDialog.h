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

#ifndef _U2_BAM_CONVERT_TO_SQLITE_DIALOG_H_
#define _U2_BAM_CONVERT_TO_SQLITE_DIALOG_H_

#include <qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QDialog>
#else
#include <QtWidgets/QDialog>
#endif
#include <QtCore/QList>
#include <U2Core/GUrl.h>
#include "ui_ConvertToSQLiteDialog.h"

namespace U2 {
namespace BAM {

QT_FORWARD_DECLARE_CLASS(BAMInfo)

class ConvertToSQLiteDialog : public QDialog
{
    Q_OBJECT
public:
    ConvertToSQLiteDialog(const GUrl& sourceUrl, BAMInfo& bamInfo, bool sam);

    const GUrl &getDestinationUrl()const;
    QString getReferenceUrl()const;
    bool        addToProject() const;
    void        hideAddToProjectOption();
public slots:
    virtual void accept();

private slots:
    void on_destinationUrlButton_clicked();
    void sl_assemblyCheckChanged(QTableWidgetItem * item);
    void sl_bamInfoButtonClicked();
    void sl_refUrlButtonClicked();
    void sl_selectAll();
    void sl_unselectAll();
    void sl_inverseSelection();

private:
    void hideReferenceUrl();
    void hideReferencesTable();
    void hideReferenceMessage();
    bool referenceFromFile();
    bool checkReferencesState();

private:
    Ui::ConvertToSQLiteDialog ui;

    GUrl destinationUrl;
    GUrl sourceUrl;
    BAMInfo &bamInfo;

    bool askIfDestFileExist;
};

} // namespace BAM
} // namespace U2

#endif // _U2_BAM_CONVERT_TO_SQLITE_DIALOG_H_
