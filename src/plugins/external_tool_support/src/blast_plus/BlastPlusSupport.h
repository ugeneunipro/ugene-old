#ifndef _U2_BLASTPLUS_SUPPORT_H
#define _U2_BLASTPLUS_SUPPORT_H

#include <U2Core/ExternalToolRegistry.h>
#include <U2Gui/ObjectViewModel.h>

#define BLASTN_TOOL_NAME "BlastN"
#define BLASTP_TOOL_NAME "BlastP"
#define BLASTX_TOOL_NAME "BlastX"
#define TBLASTN_TOOL_NAME "TBlastN"
#define TBLASTX_TOOL_NAME "TBlastX"
#define RPSBLAST_TOOL_NAME "RPSBlast"
namespace U2 {

class BlastPlusSupport : public ExternalTool {
    Q_OBJECT
public:
    BlastPlusSupport(const QString& name, const QString& path = "");
public slots:
    void sl_runWithExtFileSpecify();
private:
    QString lastDBPath;
    QString lastDBName;
};

class BlastPlusSupportContext: public GObjectViewWindowContext {
    Q_OBJECT
public:
    BlastPlusSupportContext(QObject* p);

protected slots:
    void sl_showDialog();

protected:
    virtual void initViewContext(GObjectView* view);
    virtual void buildMenu(GObjectView* view, QMenu* m);
private:
    QStringList toolList;
    QString lastDBPath;
    QString lastDBName;
};

}//namespace
#endif // _U2_BLASTPLUS_SUPPORT_H
