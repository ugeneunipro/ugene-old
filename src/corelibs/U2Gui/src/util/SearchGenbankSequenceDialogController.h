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

#ifndef _U2_SEARCH_GENBANK_SEQUENCE_DIALOG_CONTROLLER_H_
#define _U2_SEARCH_GENBANK_SEQUENCE_DIALOG_CONTROLLER_H_

#include <U2Core/global.h>

#include <QtGui/QDialog>
#include <QtCore/QList>
#include <QtCore/QString>

class Ui_SearchGenbankSequenceDialog;

namespace U2 {


struct QueryBlock {
    QString type;
    QString term;
};

class SearchGenbankSequenceDialogController;

class QueryBuilderController : public QObject {
    Q_OBJECT
private:
    QMap<QWidget*,QWidget*> removeWidgetCallbacks;
    SearchGenbankSequenceDialogController* parentController;
public:
    QueryBuilderController(SearchGenbankSequenceDialogController* parent);
    void removeQueryBlockWidget(QPushButton* callbackButton);
private slots:
    void sl_addQueryBlockWidget();
    void sl_removeQueryBlockWidget();


};


class U2GUI_EXPORT SearchGenbankSequenceDialogController : public QDialog {
    Q_OBJECT
public:
    SearchGenbankSequenceDialogController(QWidget *p = NULL);
    ~SearchGenbankSequenceDialogController();
    void addQueryBlockWidget(QWidget* w);
    void removeQueryBlockWidget(QWidget* w);
private:
    Ui_SearchGenbankSequenceDialog* ui;
    QueryBuilderController* queryBlockController;
};

} // namespace

#endif //_U2_SEARCH_GENBANK_SEQUENCE_DIALOG_CONTROLLER_H_
