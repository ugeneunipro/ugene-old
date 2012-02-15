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

#ifndef _U2_UMUSCLE_ADAPTER_H_
#define _U2_UMUSCLE_ADAPTER_H_

#include <QtCore/QObject>

namespace U2 {

class MAlignment;
class TaskStateInfo;

class MuscleAdapter : public QObject {
Q_OBJECT
public:
    static void align(const MAlignment& ma, MAlignment& res, TaskStateInfo& ti, bool mhack = true);

    static void refine(const MAlignment& ma, MAlignment& res, TaskStateInfo& ti);

    static void align2Profiles(const MAlignment& ma1, const MAlignment& ma2, MAlignment& res, TaskStateInfo& ti);
    
    static void addUnalignedSequencesToProfile( const MAlignment& ma, const MAlignment& unalignedSeqs, MAlignment& res, TaskStateInfo& ti);

private:
    static void alignUnsafe(const MAlignment& ma, MAlignment& res, TaskStateInfo& ti, bool mhack);
    
    static void refineUnsafe(const MAlignment& ma, MAlignment& res, TaskStateInfo& ti);

    static void align2ProfilesUnsafe(const MAlignment& ma1, const MAlignment& ma2, MAlignment& res, TaskStateInfo& ti);

    static void addUnalignedSequencesToProfileUnsafe(const MAlignment& ma, const MAlignment& unalignedSeqs, MAlignment& res, TaskStateInfo& ti);
};

}//namespace

#endif
