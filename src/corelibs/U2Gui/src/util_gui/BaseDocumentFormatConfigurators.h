#ifndef _U2_BASE_DOC_FORMAT_CONFIG_H_
#define _U2_BASE_DOC_FORMAT_CONFIG_H_

#include <U2Core/global.h>
#include <U2Core/DocumentFormatConfigurators.h>

namespace U2 {

class U2GUI_EXPORT BaseDocumentFormatConfigurators  {
public:
    static void initBuiltInConfigurators();

    static void loadDefaultFormatSettings(const DocumentFormatId& format, QVariantMap& formatSettings);
    static void saveDefaultFormatSettings(const DocumentFormatId& format, const QVariantMap& formatSettings);

};

class U2GUI_EXPORT MultiPartDocFormatConfigurator : public DocumentFormatConfigurator {
    Q_OBJECT
public:
    MultiPartDocFormatConfigurator(DocumentFormatId id) : DocumentFormatConfigurator(id) {};

    virtual void configure(QVariantMap& info);


};


}//namespace
#endif
