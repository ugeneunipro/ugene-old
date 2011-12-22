/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include <U2Core/U1AnnotationUtils.h>

namespace U2 {

QList< QVector<U2Region> > U1AnnotationUtils::fixLocationsForReplacedRegion(const U2Region& region2Remove, qint64 region2InsertLength,  
        const QVector<U2Region>& original, AnnotationStrategyForResize s )
{
    
    QList< QVector<U2Region> > res;
    qint64 dLen = region2InsertLength - region2Remove.length;
    if (s == AnnotationStrategyForResize_Resize) {
        if (region2Remove.length == region2InsertLength) {
            res << original;
            return res;
        }
    }
    res << QVector<U2Region>();
    QVector<U2Region>& updated =  res[0];

    foreach(U2Region r, original) {
        //if location ends before modification
		if (r.endPos() <= region2Remove.startPos) {
            updated << r;
            continue;
        }
        // if location starts after the modification
        if (r.startPos >= region2Remove.endPos()) { 
            r.startPos += dLen;
            updated << r;
            continue;
        }
        if (s == AnnotationStrategyForResize_Remove) {
            continue;
        }
        if (s == AnnotationStrategyForResize_Resize) {
            // if location contains modified region -> update it length
            if (r.contains(region2Remove)) {
				// if set A = set B - do nothing
				if(!(r.startPos == region2Remove.startPos && r.endPos() == region2Remove.endPos())){
					r.length += dLen;
					updated << r;
				}
            }
			// if location partly contain (in the end) region2remove - update length
			else if(r.contains(U2Region(region2Remove.startPos,0))){
				if(dLen < 0){
					r.length -= (r.endPos() - region2Remove.startPos) ;
				}
				updated << r;
			}
			else if(r.contains(U2Region(region2Remove.endPos(),0))){
				if(dLen < 0){
					int diff = region2Remove.endPos() - r.startPos;
					r.startPos += diff + dLen;
					r.length -= diff; 
				}
				updated << r;
			}
            continue;
        }
        assert(s == AnnotationStrategyForResize_Split_To_Joined || s == AnnotationStrategyForResize_Split_To_Separate);
        //leave left part in original(updated) locations and push right into new one
        bool join = (s == AnnotationStrategyForResize_Split_To_Joined);
        U2Region interR = r.intersect(region2Remove);
        U2Region leftR = r.startPos < interR.startPos ? U2Region(r.startPos, interR.startPos - r.startPos) : U2Region();
        U2Region rightR = r.endPos() > interR.endPos() ? U2Region(interR.endPos()+dLen, r.endPos() - interR.endPos()) : U2Region();
        if (leftR.isEmpty()) {
            if (!rightR.isEmpty()) {
                updated << rightR;
            }
            continue;
        }
        updated << leftR;
        if (!rightR.isEmpty()) {
            if (join) {
                updated << rightR;
            } else {
                QVector<U2Region> extraAnnReg;
                extraAnnReg << rightR;
                res << extraAnnReg;
            }
        }
    }
    return res;
}

int U1AnnotationUtils::getRegionFrame(int sequenceLen, U2Strand strand, bool order, int region, const QVector<U2Region>& location) {
    int frame = 0;
    const U2Region& r = location.at(region);
    if (strand.isCompementary()) {
        frame = (sequenceLen - r.endPos()) % 3;
    } else {
        frame = r.startPos % 3;
    }
    if (!order){ //join -> need to join region with prev regions to derive frame
        if (strand.isCompementary()) {
            int offset = 0;
            for (int i = location.size(); --i > region;) {
                const U2Region& rb = location.at(i);
                offset += rb.length;
            }
            int dFrame = offset % 3;
            frame = (frame + (3 - dFrame)) % 3;
        } else {
            int offset = 0;
            for (int i = 0; i < region; i++) {
                const U2Region& rb = location.at(i);
                offset += rb.length;
            }
            int dFrame = offset % 3;
            frame = (frame + (3 - dFrame)) % 3;
        }
    }
    return frame;
}

bool U1AnnotationUtils::isSplitted( const U2Location& location, const U2Region& seqRange )
{
    QVector<U2Region> regions = location->regions;
    if (regions.size() != 2) {
        return false;
    }
    if ( regions[0].endPos() == seqRange.endPos() && regions[1].startPos == seqRange.startPos) {
        return true; 
    }
    
    return false;
}


} //namespace
