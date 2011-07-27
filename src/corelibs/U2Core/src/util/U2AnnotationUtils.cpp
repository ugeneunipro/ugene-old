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

#include <U2Core/U2AnnotationUtils.h>

namespace U2 {

QVector<U2Region> U2AnnotationUtils::fixLocationsForRemovedRegion( const U2Region& regionToDelete, 
                                                                QVector<U2Region>& regionList, 
                                                                AnnotationStrategyForResize s )
{
    //assert (s == AnnotationStrategyForResize_Remove || s == AnnotationStrategyForResize_Resize);
    QVector<U2Region> toDelete, toReplace;
    foreach(U2Region reg, regionList){
        toDelete.append(reg);
        if (s != AnnotationStrategyForResize_Remove){
            if(reg.contains(regionToDelete)){
                reg.length -= regionToDelete.length;
            }else if (regionToDelete.contains(reg)) {
                reg.length = 0;
            }else if(reg.intersects(regionToDelete)){
                if(reg.startPos <= regionToDelete.startPos){
                    reg.length -= reg.endPos() - regionToDelete.startPos;
                }else{
                    reg.length -= regionToDelete.endPos() - reg.startPos;
                    reg.startPos = regionToDelete.startPos;
                }
            }else if(reg.startPos >= regionToDelete.endPos()){
                reg.startPos -= regionToDelete.length;
            } 
        }
        else {
            if(reg.intersects(regionToDelete) || regionToDelete.contains(reg)){
                reg.length = 0;
            }else if(reg.startPos >= regionToDelete.endPos()){
                reg.startPos -= regionToDelete.length;
            }
        }
        if (reg.length != 0) {
            toDelete.pop_back();
            toReplace.append(reg);
        }
    }
    regionList.clear();
    regionList << toReplace;

    return toDelete;
}

QVector<U2Region> U2AnnotationUtils::fixLocationsForInsertedRegion( qint64 insertPos, qint64 len, 
                                                                 QVector<U2Region>& regionList, 
                                                                 AnnotationStrategyForResize s,
                                                                 Annotation *an, AnnotationTableObject *ato)
{
    QVector<U2Region> toReplace, toDelete, toSplit;
    foreach(U2Region reg, regionList){
        if(reg.endPos() <= insertPos){
            toReplace.append(reg);
        } else {
            if (s == AnnotationStrategyForResize_Resize){
                if(reg.startPos < insertPos && reg.endPos() > insertPos){
                    reg.length += len;
                    toReplace.append(reg);
                }else if(reg.startPos >= insertPos){
                    reg.startPos += len;
                    toReplace.append(reg);
                }                                
            }else if(s == AnnotationStrategyForResize_Split_To_Joined){
                if(reg.startPos < insertPos && reg.endPos() > insertPos){
                    U2Region firstPart, secondPart;
                    firstPart.startPos = reg.startPos;
                    firstPart.length = insertPos - reg.startPos;
                    secondPart.startPos = firstPart.endPos() + len;
                    secondPart.length = reg.length - firstPart.length;
                    toReplace.append(firstPart);
                    toReplace.append(secondPart);
                }else if(reg.startPos >= insertPos){
                    reg.startPos += len;
                    toReplace.append(reg);
                }                                
            }else if(s == AnnotationStrategyForResize_Split_To_Separate){
                if(reg.startPos < insertPos && reg.endPos() > insertPos){
                    U2Region firstPart, secondPart;
                    firstPart.startPos = reg.startPos;
                    firstPart.length = insertPos - reg.startPos;
                    secondPart.startPos = firstPart.endPos() + len;
                    secondPart.length = reg.length - firstPart.length;
                    toReplace.append(firstPart);
                    toSplit.append(secondPart);
                }else if(reg.startPos >= insertPos){
                    reg.startPos += len;
                    toSplit.append(reg);
                }                      
            }else if(s == AnnotationStrategyForResize_Remove){
                if(reg.startPos < insertPos && reg.endPos() > insertPos){
                    toDelete.append(reg);
                }else if(reg.startPos >= insertPos){
                    reg.startPos += len;
                    toReplace.append(reg);
                }                                
            }
        }
    }

    if(s == AnnotationStrategyForResize_Split_To_Separate && toSplit.size() > 0){
        assert(an != NULL && ato != NULL);
        Annotation *splitted = new Annotation(an->data());
        splitted->setAnnotationName(an->getAnnotationName());
        QStringList groupsList;
        foreach(AnnotationGroup *group, an->getGroups()){
            groupsList.append(group->getGroupName());
        }
        splitted->replaceRegions(toSplit);
        ato->addAnnotation(splitted, groupsList);
    }
    regionList.clear();
    regionList << toReplace;
    return toDelete;
}

QVector<U2Region> U2AnnotationUtils::fixLocationsForReplacedRegion( const U2Region& regionToReplace, 
                                                                 qint64 newLen, QVector<U2Region>& loc, 
                                                                 AnnotationStrategyForResize s )
{
    if (s == AnnotationStrategyForResize_Remove) {
        QVector<U2Region> l1 = fixLocationsForRemovedRegion(regionToReplace, loc, s);
        QVector<U2Region> l2 = fixLocationsForInsertedRegion(regionToReplace.startPos, newLen, loc, s);
        assert(l2.isEmpty()); Q_UNUSED(l2);
        return l1;
    } else {
        int offset = newLen - regionToReplace.length;
        if (s == AnnotationStrategyForResize_Resize && offset == 0) {
            return QVector<U2Region>();
        }
        assert(s == AnnotationStrategyForResize_Resize); // FIXME do we ever need to SPLIT when replacing ???

        QVector<U2Region> toReplace, toDelete;
        foreach(U2Region reg, loc){
            if(reg.endPos() <= regionToReplace.startPos){
                toReplace.append(reg);
            } else {
                if (reg.contains(regionToReplace)) {
                    reg.length += offset;
                } else if (reg.startPos >= regionToReplace.endPos()) {
                    reg.startPos += offset;
                } else {
                    // start pos and/or end pos lie inside the regionToReplace
                    // let's assume offset is applied at the region end
                    if (offset > 0) {
                        if (reg.endPos() <= regionToReplace.endPos()) {
                            // leave it as is
                        } else {
                            // append tail
                            reg.length += offset;
                        }
                    } else {
                        if (reg.endPos() <= regionToReplace.endPos() + offset) {
                            // leave it as is
                        } else if (reg.startPos < regionToReplace.endPos() + offset && reg.endPos() >= regionToReplace.endPos()) {    
                            // crop inner subregion
                            reg.length += offset;
                        } else if (reg.startPos >= regionToReplace.endPos() + offset && reg.endPos() <= regionToReplace.endPos()) {
                            // drop the region
                            reg.length = 0;
                        } else if (reg.startPos < regionToReplace.endPos() + offset) {
                            // crop tail
                            assert(reg.endPos() < regionToReplace.endPos() && reg.endPos() > regionToReplace.endPos() + offset);
                            reg.length -= reg.endPos() - regionToReplace.startPos;
                        } else {
                            // crop head
                            assert(reg.startPos >= regionToReplace.endPos() + offset && reg.startPos < regionToReplace.endPos());
                            assert(reg.endPos() > regionToReplace.endPos());
                            reg.length = reg.endPos() - regionToReplace.endPos();
                            reg.startPos = regionToReplace.endPos() + offset;
                        }
                    }
                }
                assert(reg.length >= 0);
                if (reg.length == 0) {
                    toDelete.append(reg);
                } else {
                    toReplace.append(reg);
                }
            }
        }
        loc.clear();
        loc << toReplace;
        return toDelete;
    }
}

int U2AnnotationUtils::getRegionFrame(int sequenceLen, U2Strand strand, bool order, int region, const QVector<U2Region>& location) {
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

bool U2AnnotationUtils::isSplitted( const U2Location& location, const U2Region& seqRange )
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
