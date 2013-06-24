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
#ifndef _S3_GENE_H_
#define _S3_GENE_H_

#include <U2Core/U2Region.h>
#include <U2Core/U2Type.h>

namespace U2 {

/* Class for S3 gene storage. Used in vizualization and filtration methods*/

class U2CORE_EXPORT Gene {
public:
    Gene(){}
    Gene(const U2DataId& featureId, QString &name, const QString &accession, const QString& _altname, const QString &note, const U2Region &region, const U2Region &cdsRegion, QVector<U2Region> &exons, bool complemented);

    void setFeatureId(const U2DataId& _featureId);
    void setName(const QString& _name);
    void setAccession(const QString& _accession);
    void setAltName(const QString& _altName);
    void setNote(const QString& _note);
    void setRegion(const U2Region& _region);
    void setCDSRegion(const U2Region& _cdsRegion);
    void setExons(const QVector<U2Region>& _exons);
    void setComplement(bool _complemented);
    void setDisease(const QString& _disease);
    void setType(const QString& _type);

    bool operator== ( const Gene & gene ) const { 
        return gene.getName() == name; 
    }

    bool operator<(const U2Region &r) const {return region < r;}
    bool operator>(const U2Region &r) const {return region > r;}

    inline bool intersects(const U2Region& r) const {
        return region.intersects(r);
    }

    const U2DataId &getFeatureId() const;
    const QString &getName() const;
    const QString &getAltName() const;
    const QString &getAccession() const;
    const QString &getNote() const;
    const U2Region &getRegion() const;
    const U2Region &getCDSRegion() const;
    const QString &getDisease() const;
    const QString &getType() const;
    const QVector<U2Region> &getExons() const;
    bool isComplemented() const;

    bool isCoding() const;
private:
    U2DataId            featureId;
    QString             name;
    QString             altname;
    QString             accession;
    QString             disease;
    QString             type;
    QString             note;
    U2Region            region;
    U2Region            cdsRegion;
    QVector<U2Region>   exons;
    bool                complement;
    //bool coding;
};

} //namespace

#endif
