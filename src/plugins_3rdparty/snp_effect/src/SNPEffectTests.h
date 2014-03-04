/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#ifndef _U2_SNP_EFFECT_TEST_H_
#define _U2_SNP_EFFECT_TEST_H_

#include <U2Test/GTest.h>
#include <U2Test/XMLTestUtils.h>

namespace U2 {
enum SNPResultType{
    SNPResultInGene = 1,
    SNPResultRegulatory
};

class GTest_SNPResultTable {
public:
    GTest_SNPResultTable():
      resultType(SNPResultInGene)
      ,initialized(false) {}

    bool init(const QString& fileName, SNPResultType _resultType);
    QStringList findSNPRaw(const QString& chrName, const QString& allele, qint64 pos, const QString& geneId);
    void clear();
    qint64 getColumnIdxByName(const QString& name);

private:
    void initOrders();
private:
    SNPResultType resultType;
    bool initialized;
    QList< QStringList > raws;
    QStringList ingeneOrder;
    QStringList regulatoryOrder;
};


class GTest_FindInSNPResultTable : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_FindInSNPResultTable, "find-in-snp-result-table", TaskFlags_NR_FOSCOE)

    void prepare();
    ReportResult report();
private:
    QString         chrName;
    QString         allele;
    qint64          pos;
    QString         geneId;
    SNPResultType   resultType;
    QString         fileName;
    QString         columnName;
    QString         expectedValue;

    GTest_SNPResultTable    resTable;
};

class SNPResultTests {
public:
    static QList<XMLTestFactory*> createTestFactories();
};


} //namespace

#endif // _U2_SNP_EFFECT_TEST_H_
