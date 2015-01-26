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

#ifndef _U2_GT_UTILS_SEQUENCE_VIEW_H
#define _U2_GT_UTILS_SEQUENCE_VIEW_H

#include <QString>
#include <api/GTGlobals.h>

namespace U2 {

class U2OpStatus;
class ADVSingleSequenceWidget;

class GTUtilsSequenceView {
public:
    static void getSequenceAsString(U2OpStatus &os, QString &sequence);
    static QString getSequenceAsString(U2OpStatus &os, int number = 0);
    static QString getBeginOfSequenceAsString(U2OpStatus &os, int length);
    static QString getEndOfSequenceAsString(U2OpStatus &os, int length);
    static int getLengthOfSequence(U2OpStatus &os);
    static void checkSequence(U2OpStatus &os, const QString &expectedSequence);
    static void selectSequenceRegion(U2OpStatus &os, int from, int to);

  	static void openSequenceView(U2OpStatus &os, const QString &sequenceName);
    static void addSequenceView(U2OpStatus &os, const QString &sequenceName);

    static void goToPosition(U2OpStatus &os, int position);

    static ADVSingleSequenceWidget * getSeqWidgetByNumber(U2OpStatus &os, int number = 0, const GTGlobals::FindOptions& = GTGlobals::FindOptions());
    static int getSeqWidgetsNumber(U2OpStatus &os);
    static QString getSeqName(U2OpStatus &os, int number = 0);
    static QString getSeqName(U2OpStatus &os, ADVSingleSequenceWidget *seqWidget);
};

} // namespace U2

#endif // GTSEQUENCEVIEWUTILS_H
