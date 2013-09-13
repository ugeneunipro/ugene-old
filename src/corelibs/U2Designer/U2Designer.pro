include (U2Designer.pri)

# Input
HEADERS += src/BreakpointHitCountDialog.h \
           src/DatasetsController.h \
           src/DatasetsListWidget.h \
           src/DatasetWidget.h \
           src/DelegateEditors.h \
           src/DesignerUtils.h \
           src/DirectoryItem.h \
           src/EditBreakpointLabelsDialog.h \
           src/EditMarkerGroupDialog.h \
           src/EstimationReporter.h \
           src/FileItem.h \
           src/GrouperEditor.h \
           src/GrouperEditorWidget.h \
           src/MarkerEditor.h \
           src/MarkerEditorWidget.h \
           src/NewBreakpointDialog.h \
           src/NewGrouperSlotDialog.h \
           src/OutputFileDialog.h \
           src/PropertyWidget.h \
           src/QDScheduler.h \
           src/UrlItem.h \
           src/WorkflowGUIUtils.h \
           src/dashboard/Dashboard.h \
           src/dashboard/OutputFilesWidget.h \
           src/dashboard/ParametersWidget.h \
           src/dashboard/ProblemsWidget.h \
           src/dashboard/ResourcesWidget.h \
           src/dashboard/StatisticsWidget.h \
           src/dashboard/TableWidget.h \
           src/support/OutputDirectoryWidget.h \
           src/support/URLLineEdit.h \
           src/wizard/ElementSelectorController.h \
           src/wizard/PairedDatasetsController.h \
           src/wizard/PropertyWizardController.h \
           src/wizard/RadioController.h \
           src/wizard/SelectorActors.h \
           src/wizard/WDWizardPage.h \
           src/wizard/WidgetController.h \
           src/wizard/WizardController.h \
           src/wizard/WizardPageController.h
FORMS += src/ui/AnnsActionDialog.ui \
         src/ui/BreakpointHitCountDialog.ui \
         src/ui/CreateDirectoryDialog.ui \
         src/ui/DatasetWidget.ui \
         src/ui/DirectoryOptions.ui \
         src/ui/EditBreakpointLabelsDialog.ui \
         src/ui/EditFloatMarkerWidget.ui \
         src/ui/EditIntegerMarkerWidget.ui \
         src/ui/EditMarkerDialog.ui \
         src/ui/EditMarkerGroupDialog.ui \
         src/ui/EditStringMarkerWidget.ui \
         src/ui/GrouperEditorWidget.ui \
         src/ui/MarkerEditorWidget.ui \
         src/ui/MsaActionDialog.ui \
         src/ui/NewBreakpointDialog.ui \
         src/ui/NewGrouperSlotDialog.ui \
         src/ui/OutputFileDialog.ui \
         src/ui/SequenceActionDialog.ui \
         src/ui/StringActionDialog.ui
SOURCES += src/BreakpointHitCountDialog.cpp \
           src/DatasetsController.cpp \
           src/DatasetsListWidget.cpp \
           src/DatasetWidget.cpp \
           src/DelegateEditors.cpp \
           src/DesignerUtils.cpp \
           src/DirectoryItem.cpp \
           src/EditBreakpointLabelsDialog.cpp \
           src/EditMarkerGroupDialog.cpp \
           src/EstimationReporter.cpp \
           src/FileItem.cpp \
           src/GrouperEditor.cpp \
           src/GrouperEditorWidget.cpp \
           src/MarkerEditor.cpp \
           src/MarkerEditorWidget.cpp \
           src/NewBreakpointDialog.cpp \
           src/NewGrouperSlotDialog.cpp \
           src/OutputFileDialog.cpp \
           src/PropertyWidget.cpp \
           src/QDScheduler.cpp \
           src/UrlItem.cpp \
           src/WorkflowGUIUtils.cpp \
           src/dashboard/Dashboard.cpp \
           src/dashboard/OutputFilesWidget.cpp \
           src/dashboard/ParametersWidget.cpp \
           src/dashboard/ProblemsWidget.cpp \
           src/dashboard/ResourcesWidget.cpp \
           src/dashboard/StatisticsWidget.cpp \
           src/dashboard/TableWidget.cpp \
           src/support/OutputDirectoryWidget.cpp \
           src/support/URLLineEdit.cpp \
           src/wizard/ElementSelectorController.cpp \
           src/wizard/PairedDatasetsController.cpp \
           src/wizard/PropertyWizardController.cpp \
           src/wizard/RadioController.cpp \
           src/wizard/SelectorActors.cpp \
           src/wizard/WDWizardPage.cpp \
           src/wizard/WidgetController.cpp \
           src/wizard/WizardController.cpp \
           src/wizard/WizardPageController.cpp
RESOURCES += U2Designer.qrc
TRANSLATIONS += transl/chinese.ts \
                transl/czech.ts \
                transl/english.ts \
                transl/russian.ts
