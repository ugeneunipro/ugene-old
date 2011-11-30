include (U2Remote.pri)

# Input
HEADERS += src/DistributedComputingUtil.h \
           src/PingTask.h \
           src/ProtocolInfo.h \
           src/ProtocolUI.h \
           src/RemoteMachine.h \
           src/RemoteMachineMonitor.h \
           src/RemoteMachineMonitorDialogController.h \
           src/RemoteMachineMonitorDialogImpl.h \
           src/RemoteMachineScanDialogImpl.h \
           src/RemoteMachineScanner.h \
           src/RemoteMachineSettingsDialog.h \
           src/RemoteMachineTasks.h \
           src/RemoteWorkflowRunTask.h \
           src/Serializable.h \
           src/SerializeUtils.h \
           src/SynchHttp.h \
           src/TaskDistributor.h
FORMS += src/ui/RemoteMachineMonitorDialog.ui \
         src/ui/RemoteMachineScanDialog.ui \
         src/ui/RemoteMachineSettingsDialog.ui
SOURCES += src/DistributedComputingUtil.cpp \
           src/PingTask.cpp \
           src/ProtocolInfo.cpp \
           src/ProtocolUI.cpp \
           src/RemoteMachine.cpp \
           src/RemoteMachineMonitor.cpp \
           src/RemoteMachineMonitorDialogController.cpp \
           src/RemoteMachineMonitorDialogImpl.cpp \
           src/RemoteMachineScanDialogImpl.cpp \
           src/RemoteMachineScanner.cpp \
           src/RemoteMachineSettingsDialog.cpp \
           src/RemoteMachineTasks.cpp \
           src/RemoteWorkflowRunTask.cpp \
           src/Serializable.cpp \
           src/SerializeUtils.cpp \
           src/SynchHttp.cpp

TRANSLATIONS += transl/chinese.ts \
                transl/czech.ts \
                transl/english.ts \
                transl/russian.ts
