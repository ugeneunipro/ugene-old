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

#ifndef _U2_CREATE_FRAGMENT_DIALOG_H_
#define _U2_CREATE_FRAGMENT_DIALOG_H_

#include "DNAFragment.h"
#include <U2Gui/RegionSelector.h>
#include <ui/ui_CreateFragmentDialog.h>

namespace U2 {

class CreateAnnotationWidgetController;
class ADVSequenceObjectContext;
class DNASequenceSelection;

class CreateFragmentDialog : public QDialog, public Ui_CreateFragmentDialog {
    Q_OBJECT
public:
    CreateFragmentDialog(ADVSequenceObjectContext* seqCtx, QWidget* p);
    CreateFragmentDialog(U2SequenceObject* seqObj, const U2Region& region, QWidget* p);
    virtual void accept();
    const DNAFragment& getFragment() { return dnaFragment; }
private:
    CreateAnnotationWidgetController* ac;
    U2SequenceObject*  seqObj;
    QList<AnnotationTableObject *> relatedAnnotations;
    QSet<QString>  enzymesSelection;
    void setupAnnotationsWidget();
    DNAFragment dnaFragment;
    RegionSelector* rs;
};


} //namespace

#endif //  _U2_CREATE_FRAGMENT_DIALOG_H_
