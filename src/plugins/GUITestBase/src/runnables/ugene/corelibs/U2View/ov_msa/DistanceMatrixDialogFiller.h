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
#ifndef DISTANCEMATRIXDIALOGFILLER_H
#define DISTANCEMATRIXDIALOGFILLER_H

#include "GTUtilsDialog.h"
namespace U2{

class DistanceMatrixDialogFiller : public Filler
{
public:
    enum SaveFormat{HTML,CSV};
    DistanceMatrixDialogFiller(U2OpStatus &os, bool _hamming=true, bool _counts=true, bool _excludeGaps=true) :
        Filler(os,"DistanceMatrixMSAProfileDialog"),
        hamming(_hamming),
        counts(_counts),
        excludeGaps(_excludeGaps),
        saveToFile(false){}

    DistanceMatrixDialogFiller(U2OpStatus &os, SaveFormat _format,QString _path) :
        Filler(os,"DistanceMatrixMSAProfileDialog"),
        hamming(true),
        counts(true),
        excludeGaps(true),
        saveToFile(true),
        format(_format),
        path(_path){}
    virtual void run();
private:
    bool hamming,counts,excludeGaps,saveToFile;
    SaveFormat format;
    QString path;
};

}

#endif // DISTANCEMATRIXDIALOGFILLER_H
