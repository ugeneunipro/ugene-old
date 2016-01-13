/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GObject.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/L10n.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2SafePoints.h>

#include "Primer.h"
#include "PrimerLibrary.h"
#include "ImportPrimerFromObjectTask.h"

namespace U2 {

ImportPrimerFromObjectTask::ImportPrimerFromObjectTask(GObject *object) :
    Task(tr("Import primer from the shared database object: %1").arg(NULL == object ? "N/A" : object->getGObjectName()),
         TaskFlags(TaskFlag_ReportingIsEnabled | TaskFlag_ReportingIsSupported))
{
    SAFE_POINT_EXT(NULL != object, setError(L10N::nullPointerError("an input object")), );
    SAFE_POINT_EXT(GObjectTypes::SEQUENCE == object->getGObjectType(), setError(tr("A non-sequence object was provided")), );
    sequenceObject = qobject_cast<U2SequenceObject *>(object);
    SAFE_POINT_EXT(NULL != sequenceObject, setError(L10N::nullPointerError("sequence object")), );
    SAFE_POINT_EXT(sequenceObject->getAlphabet(), setError(L10N::nullPointerError("sequence alphabet")), );
    CHECK_EXT(BaseDNAAlphabetIds::NUCL_DNA_DEFAULT() == sequenceObject->getAlphabet()->getId(), setError(tr("The sequence has an unsupported alphabet: only standard nucleotide alphabet is supported")), );
    CHECK_EXT(sequenceObject->getSequenceLength() < Primer::MAX_LEN, setError(tr("Can't convert a sequence to primer: the sequence is too long")), );
}

void ImportPrimerFromObjectTask::run() {
    primer.name = sequenceObject->getGObjectName();
    primer.sequence = sequenceObject->getWholeSequenceData(stateInfo);
    CHECK_OP(stateInfo, );

    QRegExp regExp("[^ACGT]");
    if (primer.sequence.contains(regExp)) {
        setError(tr("The primer sequence contains non-ACGT symbols"));
        return;
    }

    PrimerLibrary *library = PrimerLibrary::getInstance(stateInfo);
    CHECK_OP(stateInfo, );
    library->addRawPrimer(primer, stateInfo);
}

QString ImportPrimerFromObjectTask::generateReport() const {
    const QString docName = (NULL == sequenceObject->getDocument() ? tr("Without document") : sequenceObject->getDocument()->getName());
    QString report = QString("<b>%1</b>%2: <font color='%3'>%4</font>").arg(docName).arg(sequenceObject->getGObjectName());
    if (isCanceled()) {
        return report.arg(L10N::errorColorLabelHtmlStr()).arg(tr("cancelled"));
    }
    if (hasError()) {
        return report.arg(L10N::errorColorLabelHtmlStr()).arg(tr("error")) + ": " + getError();
    }
    return report.arg(L10N::successColorLabelHtmlStr()).arg(tr("success"));
}

}   // namespace U2
