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

#ifndef _U2_C_INTERFACE_UNIT_TESTS_H_
#define _U2_C_INTERFACE_UNIT_TESTS_H_

#include <U2Core/U2OpStatus.h>
#include <unittest.h>

namespace U2 {

class SchemeSimilarityUtils {
public:
    static void            checkSchemesSimilarity( SchemeHandle assembledScheme,
                            const QString &pathToProperScheme, U2OpStatus &stateInfo );

private:
    static int              getSchemeDescriptionStartPos( const QString &schemeContent );
    static int              getSchemeDescriptionEndPos( const QString &schemeContent );
    static QString          getSchemeContentByHandle( SchemeHandle scheme,
                                U2OpStatus &stateInfo );
    static QString          getSchemeContentByFilePath( const QString &pathToScheme,
                                U2OpStatus &stateInfo );
    static QString          readFileContent( QFile &file, U2OpStatus &stateInfo );
    static void             skipSchemeSpecificNames( QString &schemeContent );
    static QStringList      getNonSpaceStatementsFromScheme( const QString &schemeContent );
    static void             skipElementNames( QString &schemeContent );
    static void             skipElementIds( QString &schemeContent );
    static void             skipActorBindingsBlockBoundaries( QString &schemeContent );
};

DECLARE_TEST( CInterfaceUnitTests, basic_align );
DECLARE_TEST( CInterfaceUnitTests, basic_align_sas );
DECLARE_TEST( CInterfaceUnitTests, faqual2fastq );
DECLARE_TEST( CInterfaceUnitTests, faqual2fastq_sas );
DECLARE_TEST( CInterfaceUnitTests, msa2clustal );
DECLARE_TEST( CInterfaceUnitTests, query2alignment );
DECLARE_TEST( CInterfaceUnitTests, seq2gen );
DECLARE_TEST( CInterfaceUnitTests, tfbs );
DECLARE_TEST( CInterfaceUnitTests, build_test_HMM );
DECLARE_TEST( CInterfaceUnitTests, search_HMM );
DECLARE_TEST( CInterfaceUnitTests, search_TFBS );
DECLARE_TEST( CInterfaceUnitTests, call_variants );
DECLARE_TEST( CInterfaceUnitTests, chip_seq );
DECLARE_TEST( CInterfaceUnitTests, chip_seq_with_control );
DECLARE_TEST( CInterfaceUnitTests, tuxedo_main );
DECLARE_TEST( CInterfaceUnitTests, tuxedo_main_paired );
DECLARE_TEST( CInterfaceUnitTests, tuxedo_multiple_dataset );
DECLARE_TEST( CInterfaceUnitTests, tuxedo_multiple_dataset_paired );
DECLARE_TEST( CInterfaceUnitTests, tuxedo_single_dataset );
DECLARE_TEST( CInterfaceUnitTests, tuxedo_single_dataset_paired );
DECLARE_TEST( CInterfaceUnitTests, find_sequences );
DECLARE_TEST( CInterfaceUnitTests, gene_by_gene_report );
DECLARE_TEST( CInterfaceUnitTests, merge_sequence_annotation );

} // namespace U2

DECLARE_METATYPE( CInterfaceUnitTests, basic_align );
DECLARE_METATYPE( CInterfaceUnitTests, basic_align_sas );
DECLARE_METATYPE( CInterfaceUnitTests, faqual2fastq );
DECLARE_METATYPE( CInterfaceUnitTests, faqual2fastq_sas );
DECLARE_METATYPE( CInterfaceUnitTests, msa2clustal );
DECLARE_METATYPE( CInterfaceUnitTests, query2alignment );
DECLARE_METATYPE( CInterfaceUnitTests, seq2gen );
DECLARE_METATYPE( CInterfaceUnitTests, tfbs );
DECLARE_METATYPE( CInterfaceUnitTests, build_test_HMM );
DECLARE_METATYPE( CInterfaceUnitTests, search_HMM );
DECLARE_METATYPE( CInterfaceUnitTests, search_TFBS );
DECLARE_METATYPE( CInterfaceUnitTests, call_variants );
DECLARE_METATYPE( CInterfaceUnitTests, chip_seq );
DECLARE_METATYPE( CInterfaceUnitTests, chip_seq_with_control );
DECLARE_METATYPE( CInterfaceUnitTests, tuxedo_main );
DECLARE_METATYPE( CInterfaceUnitTests, tuxedo_main_paired );
DECLARE_METATYPE( CInterfaceUnitTests, tuxedo_multiple_dataset );
DECLARE_METATYPE( CInterfaceUnitTests, tuxedo_multiple_dataset_paired );
DECLARE_METATYPE( CInterfaceUnitTests, tuxedo_single_dataset );
DECLARE_METATYPE( CInterfaceUnitTests, tuxedo_single_dataset_paired );
DECLARE_METATYPE( CInterfaceUnitTests, find_sequences );
DECLARE_METATYPE( CInterfaceUnitTests, gene_by_gene_report );
DECLARE_METATYPE( CInterfaceUnitTests, merge_sequence_annotation );

#endif // _U2_C_INTERFACE_UNIT_TESTS_H_