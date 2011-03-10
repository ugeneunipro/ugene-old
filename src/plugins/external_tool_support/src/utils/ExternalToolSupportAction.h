#ifndef _U2_EXTERNAL_TOOL_SUPPORT_ACTION_H
#define _U2_EXTERNAL_TOOL_SUPPORT_ACTION_H

#include <U2Core/ExternalToolRegistry.h>
#include <U2Gui/ObjectViewModel.h>

namespace U2 {

class ExternalToolSupprotAction : public GObjectViewAction {
    Q_OBJECT
public:
    ExternalToolSupprotAction(QObject* p, GObjectView* v, const QString& _text, int order, const QStringList& _toolNames);

    ExternalToolSupprotAction(const QString& text, QObject* p, const QStringList& _toolNames);

    const QStringList getToolNames(){return toolNames;}
private slots:
    void sl_pathChanged();
private:
    QStringList toolNames;
};

}//namespace
#endif // _U2_EXTERNAL_TOOL_SUPPORT_ACTION_H
