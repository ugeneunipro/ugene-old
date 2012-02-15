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

#ifndef _U2_REMOVE_PART_FROM_SEQUENCE_CONTROLLER_H_
#define _U2_REMOVE_PART_FROM_SEQUENCE_CONTROLLER_H_

#include <U2Core/U2Region.h>
#include <U2Core/U1AnnotationUtils.h>

#include <QtGui/QDialog>

class Ui_RemovePartFromSequenceDialog;

namespace U2{

class U2GUI_EXPORT RemovePartFromSequenceDialogController : public QDialog {
    Q_OBJECT
public:
    RemovePartFromSequenceDialogController(U2Region _toDelete, U2Region _source, const QString & docUrl, QWidget *p = NULL);
    ~RemovePartFromSequenceDialogController();
    virtual void accept();

    bool modifyCurrentDocument() const;;
    QString getNewDocumentPath() const;;
    U1AnnotationUtils::AnnotationStrategyForResize getStrategy();
    U2Region getRegionToDelete() const {return toDelete;};
    bool mergeAnnotations() const;;
    DocumentFormatId getDocumentFormatId() const;;

private slots:
    void sl_browseButtonClicked();
    void sl_indexChanged(int);
    void sl_mergeAnnotationsToggled(bool);

private:
    QString filter;
    U2Region toDelete;
    U2Region source;
    Ui_RemovePartFromSequenceDialog* ui;
};

}//ns

#endif

