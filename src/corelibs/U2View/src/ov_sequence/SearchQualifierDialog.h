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

#ifndef _U2_SEARCH_QUALIFIER_DIALOG_H_
#define _U2_SEARCH_QUALIFIER_DIALOG_H_

#include <U2Core/global.h>
#include <U2Core/Task.h>

#include <U2View/AnnotationsTreeView.h>

#include <QtGui/QDialog>

class Ui_SearchQualifierDialog;

namespace U2 {

class AnnotationTableObject;
class AnnotationsTreeView;
class AVItem;

class U2VIEW_EXPORT SearchQualifierDialog: public QDialog {
    Q_OBJECT
public:
    SearchQualifierDialog(QWidget* p, AnnotationsTreeView * _treeView);
    ~SearchQualifierDialog();
    
    virtual void accept();
    
protected:
    bool eventFilter(QObject *obj, QEvent *e);
    void clearPrevResults();
    
protected slots:
    void sl_searchTaskStateChanged();
    void sl_searchAll();
private:
    void search(bool searchAll = false);
    AnnotationsTreeView * treeView;
    Ui_SearchQualifierDialog* ui;
    AVItem* groupToSearchIn;
    AVItem * parentAnnotationofPrevResult;
    int indexOfPrevResult;
    
};

}//namespace
#endif //_U2_SEARCH_QUALIFIER_DIALOG_H_
