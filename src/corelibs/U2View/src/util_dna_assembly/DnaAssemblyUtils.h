#ifndef _U2_DNA_ASSEMBLEY_UTILS_H_
#define _U2_DNA_ASSEMBLEY_UTILS_H_

#include <U2Core/global.h>

namespace U2 {

class DnaAssemblyToRefTaskSettings;

class U2VIEW_EXPORT DnaAssemblySupport : public QObject {
    Q_OBJECT
public:
    DnaAssemblySupport();

private slots:
    void sl_showDnaAssemblyDialog();
    void sl_showBuildIndexDialog();

}; 

// class DnaAssemblyLoadShortReadsTask : public Task {
//     Q_OBJECT
// public:
//     DnaAssemblyLoadShortReadsTask(DnaAssemblyToRefTaskSettings& settings, const QList<GUrl>& shortReads);
//     virtual void prepare();
//     QList<Task*> onSubTaskFinished(Task* subTask);
//     QList<DN
//     virtual ReportResult report();
// private:
//     DnaAssemblyToRefTaskSettings& settings;
//     const QList<GUrl>& shortReads;
// }; 

} // U2


#endif // _U2_DNA_ASSEMBLEY_TASK_H_
