#ifndef _U2_WORKFLOW_SCENE_SERIALIZER_H_
#define _U2_WORKFLOW_SCENE_SERIALIZER_H_

#include <U2Lang/SchemaSerializer.h>

class QGraphicsItem;

namespace U2 {

class WorkflowScene;
class WorkflowProcessItem;
class WorkflowBusItem;

class SceneSerializer {
public:
    static void saveItems(const QList<QGraphicsItem*>& items, QDomElement& proj);
    
    static void scene2xml(const WorkflowScene* scene, QDomDocument& xml);
    static QString xml2scene(const QDomElement& projectElement, WorkflowScene* scene, QMap<ActorId, ActorId>& remapping, 
        bool ignoreErrors = false, bool select = false);
};

} // U2

#endif // _U2_WORKFLOW_SCENE_SERIALIZER_H_
