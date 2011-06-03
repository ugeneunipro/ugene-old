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

#include "FileDbi.h"

#include <U2Core/DocumentModel.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/AppContext.h>

namespace U2 {

// FileDbi

FileDbi::FileDbi(FileDbiFactory * f) : U2AbstractDbi(f->getId()), fid(f->getDocumentFormatId()), seqDbi(NULL){
    assert(!fid.isEmpty());
}

void FileDbi::init(const QHash<QString, QString>& props, const QVariantMap&, U2OpStatus& os) {
    bool create = props.value(U2_DBI_OPTION_CREATE, "0").toInt() > 0;
    Q_UNUSED(create);
    assert(!create);
    
    QString url = props.value(U2_DBI_OPTION_URL);
    if(url.isEmpty()) {
        os.setError(FileDbiL10N::tr("File url is empty"));
        state = U2DbiState_Void;
        return;
    }
    Project * prj = AppContext::getProject();
    Document * doc = NULL;
    if(prj == NULL || (doc = prj->findDocumentByURL(url)) == NULL || !doc->isLoaded()) {
        os.setError(FileDbiL10N::tr("File '%1' needed to be added to project and loaded").arg(url));
        state = U2DbiState_Void;
        return;
    }
    if(doc->getDocumentFormat()->getSupportedObjectTypes().contains(GObjectTypes::SEQUENCE)) {
        QList<GObject*> objs = doc->findGObjectByType(GObjectTypes::SEQUENCE);
        QList<DNASequenceObject*> seqObjs;
        foreach(GObject * obj, objs) {
            DNASequenceObject * o = qobject_cast<DNASequenceObject*>(obj);
            if(o != NULL) {
                seqObjs << o;
            } else {
                assert(false);
            }
        }
        if(seqObjs.isEmpty()) {
            os.setError(FileDbiL10N::tr("No sequence objects found in '%1'").arg(url));
            state = U2DbiState_Void;
            return;
        }
        seqDbi = new DNASequenceObjectSequenceDbiWrapper(seqObjs, this);
    }
    initProperties = props;
    state = U2DbiState_Ready;
}

QVariantMap FileDbi::shutdown(U2OpStatus&) {
    return QVariantMap();
}

U2SequenceDbi * FileDbi::getSequenceDbi() {
    return seqDbi;
}

// FileDbiFactory

const U2DbiFactoryId FileDbiFactory::ID_PREFIX("FileDbi_");

FileDbiFactory::FileDbiFactory(const DocumentFormatId & f) : U2DbiFactory(), fid(f) {
    assert(!fid.isEmpty());
}

U2Dbi * FileDbiFactory::createDbi() {
    DocumentFormatRegistry * dfReg = AppContext::getDocumentFormatRegistry();
    if(dfReg != NULL && dfReg->getFormatById(fid) != NULL) {
        return new FileDbi(this);
    }
    return NULL;
}

U2DbiFactoryId FileDbiFactory::getId() const {
    return ID_PREFIX + fid;
}

bool FileDbiFactory::isValidDbi(const QHash<QString, QString>&, const QByteArray&, U2OpStatus&) const {
    return false;
}

} // U2
