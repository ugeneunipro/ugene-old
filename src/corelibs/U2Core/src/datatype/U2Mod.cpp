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

#include "U2Mod.h"

namespace U2 {

const qint64 U2ModType::objUpdatedName        = 1;

const qint64 U2ModType::msaUpdatedAlphabet    = 3001;
const qint64 U2ModType::msaAddedRows          = 3002;
const qint64 U2ModType::msaAddedRow           = 3003;
const qint64 U2ModType::msaRemovedRows        = 3004;
const qint64 U2ModType::msaRemovedRow         = 3005;
const qint64 U2ModType::msaUpdatedRowContent  = 3006;
const qint64 U2ModType::msaUpdatedGapModel    = 3007;
const qint64 U2ModType::msaSetNewRowsOrder    = 3008;
const qint64 U2ModType::msaUpdatedRowName     = 3009;

} // namespace
