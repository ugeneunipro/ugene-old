include (ugeneui.pri)

# Input
HEADERS += src/app_settings/AppSettingsDialogController.h \
           src/app_settings/AppSettingsDialogTree.h \
           src/app_settings/AppSettingsGUIImpl.h \
           src/app_settings/directories_settings/DirectoriesSettingsGUIController.h \
           src/app_settings/format_settings/FormatSettingsGUIController.h \
           src/app_settings/logview_settings/LogSettingsGUIController.h \
           src/app_settings/network_settings/NetworkSettingsGUIController.h \
           src/app_settings/resource_settings/ResourceSettingsGUIController.h \
           src/app_settings/user_apps_settings/UserApplicationsSettingsGUIController.h \
           src/main_window/AboutDialogController.h \
           src/main_window/SplashScreen.h \
           src/main_window/CheckUpdatesTask.h \
           src/main_window/DockManagerImpl.h \
           src/main_window/MainWindowImpl.h \
           src/main_window/MDIManagerImpl.h \
           src/main_window/MenuManager.h \
           src/main_window/ShutdownTask.h \
           src/main_window/TmpDirChangeDialogController.h \
           src/main_window/ToolBarManager.h \
           src/plugin_viewer/PluginViewerController.h \
           src/plugin_viewer/PluginViewerImpl.h \
           src/project_support/DocumentFormatSelectorController.h \
           src/project_support/DocumentProviderSelectorController.h \
           src/project_support/DocumentReadingModeSelectorController.h \
           src/project_support/MultipleDocumentsReadingModeSelectorController.h \
           src/project_support/ExportProjectDialogController.h \
           src/project_support/ProjectImpl.h \
           src/project_support/ProjectLoaderImpl.h \
           src/project_support/ProjectServiceImpl.h \
           src/project_support/ProjectTasksGui.h \
           src/project_view/ProjectViewDocTree.h \
           src/project_view/ProjectViewImpl.h \
           src/project_view/ProjectViewSearchBox.h \
           src/shtirlitz/Shtirlitz.h \
           src/shtirlitz/StatisticalReportController.h \
           src/task_view/TaskStatusBar.h \
           src/task_view/TaskViewController.h \
           src/update/UgeneUpdater.h \
           src/utils/MultilingualHtmlView.h \
           src/welcome_page/WelcomePageController.h \
           src/welcome_page/WelcomePageMdi.h \
           src/welcome_page/WelcomePageWidget.h
FORMS += src/app_settings/directories_settings/ui/DirectoriesSettingsWidget.ui \
         src/app_settings/format_settings/ui/FormatSettingsWidget.ui \
         src/app_settings/logview_settings/ui/LogSettingsWidget.ui \
         src/app_settings/network_settings/ui/NetworkSettingsWidget.ui \
         src/app_settings/resource_settings/ui/ResourceSettingsWidget.ui \
         src/app_settings/ui/AppSettingsDialog.ui \
         src/app_settings/user_apps_settings/ui/UserApplicationsSettingsWidget.ui \
         src/main_window/ui/AboutDialog.ui \
         src/main_window/ui/TmpDirChangeDialog.ui \
         src/plugin_viewer/ui/PluginViewerWidget.ui \
         src/project_support/ui/CreateNewProjectWidget.ui \
         src/project_support/ui/DocumentFormatSelectorDialog.ui \
         src/project_support/ui/DocumentProviderSelectorDialog.ui \
         src/project_support/ui/ExportProjectDialog.ui \
         src/project_support/ui/MultipleSequenceFilesReadingMode.ui \
         src/project_support/ui/SequenceReadingModeSelectorDialog.ui \
         src/project_support/ui/SaveProjectDialog.ui \
         src/project_view/ui/ProjectViewWidget.ui \
         src/shtirlitz/ui/StatisticalReport.ui
SOURCES += src/Main.cpp \
           src/app_settings/AppSettingsDialogController.cpp \
           src/app_settings/AppSettingsGUIImpl.cpp \
           src/app_settings/directories_settings/DirectoriesSettingsGUIController.cpp \
           src/app_settings/format_settings/FormatSettingsGUIController.cpp \
           src/app_settings/logview_settings/LogSettingsGUIController.cpp \
           src/app_settings/network_settings/NetworkSettingsGUIController.cpp \
           src/app_settings/resource_settings/ResourceSettingsGUIController.cpp \
           src/app_settings/user_apps_settings/UserApplicationsSettingsGUIController.cpp \
           src/main_window/AboutDialogController.cpp \
           src/main_window/SplashScreen.cpp \
           src/main_window/CheckUpdatesTask.cpp \
           src/main_window/DockManagerImpl.cpp \
           src/main_window/MainWindowImpl.cpp \
           src/main_window/MDIManagerImpl.cpp \
           src/main_window/MenuManager.cpp \
           src/main_window/ShutdownTask.cpp \
           src/main_window/TmpDirChangeDialogController.cpp \
           src/main_window/ToolBarManager.cpp \
           src/plugin_viewer/PluginViewerController.cpp \
           src/plugin_viewer/PluginViewerImpl.cpp \
           src/project_support/DocumentFormatSelectorController.cpp \
           src/project_support/DocumentProviderSelectorController.cpp \
           src/project_support/DocumentReadingModeSelectorController.cpp \
           src/project_support/MultipleDocumentsReadingModeSelectorController.cpp \
           src/project_support/ExportProjectDialogController.cpp \
           src/project_support/ProjectImpl.cpp \
           src/project_support/ProjectLoaderImpl.cpp \
           src/project_support/ProjectServiceImpl.cpp \
           src/project_support/ProjectTasksGui.cpp \
           src/project_view/BuiltInObjectViews.cpp \
           src/project_view/ProjectViewDocTree.cpp \
           src/project_view/ProjectViewImpl.cpp \
           src/project_view/ProjectViewSearchBox.cpp \
           src/shtirlitz/Shtirlitz.cpp \
           src/shtirlitz/StatisticalReportController.cpp \
           src/task_view/TaskStatusBar.cpp \
           src/task_view/TaskViewController.cpp \
           src/update/UgeneUpdater.cpp \
           src/utils/MultilingualHtmlView.cpp \
           src/welcome_page/WelcomePageController.cpp \
           src/welcome_page/WelcomePageMdi.cpp \
           src/welcome_page/WelcomePageWidget.cpp
RESOURCES += ugeneui.qrc
TRANSLATIONS += transl/english.ts \
                transl/russian.ts

