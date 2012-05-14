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

#include "GTUtilsMsaEditorSequenceArea.h"
#include "api/GTWidget.h"
#include "U2View/MSAEditorSequenceArea.h"

namespace U2 {

void GTUtilsMSAEditorSequenceArea::checkSelectedRect(U2OpStatus &os, const QRect &expectedRect)
{
    MSAEditorSequenceArea *msaEditArea = qobject_cast<MSAEditorSequenceArea*>(GTWidget::findWidget(os, "msa_editor_sequence_area"));
    CHECK_SET_ERR(msaEditArea != NULL, "MsaEditorSequenceArea not found");

    CHECK_SET_ERR(expectedRect == msaEditArea->getSelection().getRect(), "Unexpected selection region");
}

} // namespace
