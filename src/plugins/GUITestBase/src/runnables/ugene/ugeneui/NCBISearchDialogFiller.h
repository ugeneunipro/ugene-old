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

#ifndef NCBISEARCHDIALOGFILLER_H
#define NCBISEARCHDIALOGFILLER_H

#include "GTUtilsDialog.h"

namespace U2 {
class NCBISearchDialogFiller : public Filler
{
public:
    NCBISearchDialogFiller(U2OpStatus &os, QString _query, bool _doubleEnter = false, int _resultLimit=-1) :
        Filler(os, "SearchGenbankSequenceDialog"),
        query(_query),
        doubleEnter(_doubleEnter),
        resultLimit(_resultLimit){}
    virtual void run();

private:
    QString query;

    bool doubleEnter;
    int resultLimit;

    bool shownCorrect();
    int getResultNumber();
};

}//namespace

#endif // NCBISEARCHDIALOGFILLER_H
