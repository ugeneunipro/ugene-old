/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#ifndef WORKFLOWMETADIALOGFILLER_H
#define WORKFLOWMETADIALOGFILLER_H

#include "utils/GTUtilsDialog.h"

namespace U2{
using namespace HI;

class WorkflowMetaDialogFiller : public Filler
{
public:
    WorkflowMetaDialogFiller(HI::GUITestOpStatus& os, QString _url, QString _name): Filler(os, "WorkflowMetaDialog"),
        url(_url),
        name(_name){}
    void commonScenario();
private:
    QString url, name;
};
}
#endif // WORKFLOWMETADIALOGFILLER_H
