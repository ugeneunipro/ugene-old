#ifndef _U2_WORKFLOW_SERIALIZER_H_
#define _U2_WORKFLOW_SERIALIZER_H_

#include <U2Lang/ActorModel.h>
#include <U2Lang/Schema.h>

class QDomDocument;
class QDomElement;

namespace U2 {
namespace Workflow {

/**
 * utility class for serialization/deserialization schema and all schema elements
 * serialization to xml become obsolete. See HRSchemaSerializer
 *
 */
class U2LANG_EXPORT SchemaSerializer : public QObject {
    Q_OBJECT
public:
    // old schemas support
    static const QMap<QString, QString> ELEM_TYPES_MAP;
    static QString getElemType(const QString & t);
    
    static const QString WORKFLOW_DOC;
    static void readConfiguration(Configuration*, const QDomElement& );
    static void readParamAliases( QMap<QString, QString> &, const QDomElement& );
    static void saveIterations(const QList<Iteration>&, QDomElement& );
    static void readIterations(QList<Iteration>&, const QDomElement&,const QMap<ActorId, ActorId>&);
    static QDomElement saveActor(const Actor*, QDomElement&);
    static QDomElement savePort(const Port*, QDomElement&);
    static QDomElement saveLink(const Link*, QDomElement&);
    static QString readMeta(Metadata* meta, const QDomElement& proj);
    static void updatePortBindings(const QList<Actor*> & procs);
    
    static void schema2xml(const Schema& scema, QDomDocument& xml);
    static QString xml2schema(const QDomElement& projectElement, Schema* schema, QMap<ActorId, ActorId>& remapping, bool stopOnErrors = true); 
    
private:
    static QMap<QString, QString> initElemTypesMap();
    
}; // SchemaSerializer

} //namespace Workflow
} //namespace U2
#endif
