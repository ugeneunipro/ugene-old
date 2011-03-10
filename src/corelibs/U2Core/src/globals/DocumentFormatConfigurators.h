#ifndef _U2_DOC_FORMAT_CONFIG_H_
#define _U2_DOC_FORMAT_CONFIG_H_

#include <U2Core/global.h>

namespace U2 {

class U2CORE_EXPORT DocumentFormatConfigurator : public QObject {
    Q_OBJECT
public:
    DocumentFormatConfigurator(const DocumentFormatId& _formatId, QObject* p = NULL): QObject(p), formatId(_formatId){}
    
    DocumentFormatId getFormatId() const {return formatId;}

    virtual void configure(QVariantMap& info) = 0;

protected:
    DocumentFormatId formatId;
};


class U2CORE_EXPORT DocumentFormatConfigurators : public QObject {
    Q_OBJECT
public:
    DocumentFormatConfigurators(QObject* p = NULL): QObject(p){}
    virtual ~DocumentFormatConfigurators();


    void registerConfigurator(DocumentFormatConfigurator* c);
    
    void unregisterConfigurator(const DocumentFormatId& id);

    DocumentFormatConfigurator* findConfigurator(const DocumentFormatId& id) const;

private:
    QMap<DocumentFormatId, DocumentFormatConfigurator*> configs;
};


}//namespace

#endif
