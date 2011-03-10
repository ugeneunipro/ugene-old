#ifndef _DOCUMENT_FORMAT_REGISTRY_IMPL_H_
#define _DOCUMENT_FORMAT_REGISTRY_IMPL_H_

#include "private.h"
#include <U2Core/DocumentModel.h>

namespace U2 {

class U2PRIVATE_EXPORT DocumentFormatRegistryImpl  : public DocumentFormatRegistry {
public:
    DocumentFormatRegistryImpl(QObject* p = NULL) : DocumentFormatRegistry(p) {init();}

    virtual bool registerFormat(DocumentFormat* dfs);

    virtual bool unregisterFormat(DocumentFormat* dfs);

    virtual QList<DocumentFormatId> getRegisteredFormats() const;

    virtual DocumentFormat* getFormatById(DocumentFormatId id) const;

    virtual DocumentFormat* selectFormatByFileExtension(const QString& fileExt) const;

    virtual QList<DocumentFormatId> selectFormats(const DocumentFormatConstraints& c) const;

private:
    void init();
    QList<DocumentFormat*>      formats;
};

}//namespace
#endif
