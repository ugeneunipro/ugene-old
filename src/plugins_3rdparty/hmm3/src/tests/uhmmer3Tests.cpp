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

#include "uhmmer3BuildTests.h"
#include "uhmmer3SearchTests.h"
#include "uhmmer3PhmmerTests.h"

#include "uhmmer3Tests.h"

namespace U2 {

QList< XMLTestFactory* > UHMMER3Tests::createTestFactories() {
    QList< XMLTestFactory* > res;
    res << GTest_CompareHmmFiles::createFactory();
    res << GTest_UHMMER3Build::createFactory();
    res << GTest_UHMM3Search::createFactory();
    res << GTest_UHMM3SearchCompare::createFactory();
    res << GTest_UHMM3Phmmer::createFactory();
    res << GTest_UHMM3PhmmerCompare::createFactory();
    return res;
}

} // GB2
