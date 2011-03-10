#ifndef _U2_SELECTION_UTILS_H_
#define _U2_SELECTION_UTILS_H_

#include <U2Core/U2Region.h>
#include <QtCore/QSet>

namespace U2 {

class GObject;
class Document;
class MultiGSelection;
class GSelection;

class U2CORE_EXPORT SelectionUtils {
public:

    static QSet<GObject*>   findObjects(GObjectType t, const MultiGSelection* s, UnloadedObjectFilter f);

    static QList<GObject*>  findObjectsKeepOrder(GObjectType t, const MultiGSelection* s, UnloadedObjectFilter f);

    static QSet<GObject*>   findObjects(GObjectType t, const GSelection* s, UnloadedObjectFilter f);

    static QList<GObject*>  findObjectsKeepOrder(GObjectType t, const GSelection* s, UnloadedObjectFilter f);

    static QSet<Document*>  findDocumentsWithObjects(GObjectType t, const MultiGSelection* s,
                                UnloadedObjectFilter f, bool deriveDocsFromObjectSelection);

    static QSet<Document*>  findDocumentsWithObjects(GObjectType t, const GSelection* s,
                                UnloadedObjectFilter f, bool deriveDocsFromObjectSelection);

    static bool isDocumentInSelection(const Document* doc, const MultiGSelection& ms, bool deriveDocsFromObjectSelection);

    static QList<Document*> getSelectedDocs(const MultiGSelection& ms);

    static QList<GObject*> getSelectedObjects(const MultiGSelection& ms);

    //todo: find a better place: like DNAUtils..
    static U2Region normalizeRegionBy3(U2Region reg, int seqLen, bool direct);
};




}//namespace

#endif

