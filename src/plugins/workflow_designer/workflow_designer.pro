include (workflow_designer.pri)

# Input
HEADERS += src/ActorCfgModel.h \
           src/ChooseItemDialog.h \
           src/CreateScriptWorker.h \
           src/HRSceneSerializer.h \
           src/ItemViewStyle.h \
           src/IterationListWidget.h \
           src/SceneSerializer.h \
           src/SchemaAliasesConfigurationDialogImpl.h \
           src/SchemaConfigurationDialog.h \
           src/WorkflowDesignerPlugin.h \
           src/WorkflowDocument.h \
           src/WorkflowEditor.h \
           src/WorkflowEditorDelegates.h \
           src/WorkflowMetaDialog.h \
           src/WorkflowPalette.h \
           src/WorkflowSamples.h \
           src/WorkflowSceneIOTasks.h \
           src/WorkflowSettingsController.h \
           src/WorkflowViewController.h \
           src/WorkflowViewItems.h \
           src/cmdline/WorkflowCMDLineTasks.h \
           src/library/BaseDocWorker.h \
           src/library/CoreLib.h \
           src/library/DocActors.h \
           src/library/DocWorkers.h \
           src/library/FindWorker.h \
           src/library/GenericReadActor.h \
           src/library/GenericReadWorker.h \
           src/library/ImportAnnotationsWorker.h \
           src/library/RemoteDBFetcherWorker.h \
           src/library/ScriptWorker.h \
           src/library/SequenceSplitWorker.h \
           src/library/Text2SequenceWorker.h \
           src/library/SequencesToMSAWorker.h \
           src/library/FilterAnnotationsWorker.h \
           src/library/CDSearchWorker.h \
	   src/library/StatisticWorkers.h \
	   src/library/ReverseComplementWorker.h \
	   src/library/MSA2SequenceWorker.h \
           src/util/SaveSchemaImageUtils.h 
FORMS += src/ui/ChooseItemDialog.ui \
         src/ui/CreateScriptBlockDialog.ui \
         src/ui/SchemaAliasesConfigurationDialog.ui \
         src/ui/SchemaConfigurationDialog.ui \
         src/ui/WorkflowEditorWidget.ui \
         src/ui/WorkflowMetaDialog.ui \
         src/ui/WorkflowSettingsWidget.ui
SOURCES += src/ActorCfgModel.cpp \
           src/ChooseItemDialog.cpp \
           src/CreateScriptWorker.cpp \
           src/HRSceneSerializer.cpp \
           src/ItemViewStyle.cpp \
           src/IterationListWidget.cpp \
           src/SceneSerializer.cpp \
           src/SchemaAliasesConfigurationDialogImpl.cpp \
           src/SchemaConfigurationDialog.cpp \
           src/WorkflowDesignerPlugin.cpp \
           src/WorkflowDocument.cpp \
           src/WorkflowEditor.cpp \
           src/WorkflowEditorDelegates.cpp \
           src/WorkflowMetaDialog.cpp \
           src/WorkflowPalette.cpp \
           src/WorkflowSamples.cpp \
           src/WorkflowSceneIOTasks.cpp \
           src/WorkflowSettingsController.cpp \
           src/WorkflowViewController.cpp \
           src/WorkflowViewItems.cpp \
           src/cmdline/WorkflowCMDLineTasks.cpp \
           src/library/BaseDocWorker.cpp \
           src/library/CoreLib.cpp \
           src/library/DocActors.cpp \
           src/library/DocWorkers.cpp \
           src/library/FindWorker.cpp \
           src/library/GenericReadActor.cpp \
           src/library/GenericReadWorker.cpp \
           src/library/ImportAnnotationsWorker.cpp \
           src/library/RemoteDBFetcherWorker.cpp \
           src/library/ScriptWorker.cpp \
           src/library/SequenceSplitWorker.cpp \
           src/library/Text2SequenceWorker.cpp \
           src/library/FilterAnnotationsWorker.cpp \
           src/library/SequencesToMSAWorker.cpp \
           src/library/CDSearchWorker.cpp \
		   src/library/StatisticWorkers.cpp \
		   src/library/ReverseComplementWorker.cpp \
		   src/library/MSA2SequenceWorker.cpp \
           src/util/SaveSchemaImageUtils.cpp 
RESOURCES += workflow_designer.qrc
TRANSLATIONS += transl/english.ts transl/russian.ts
