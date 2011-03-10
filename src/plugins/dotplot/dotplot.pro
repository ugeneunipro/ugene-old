include (dotplot.pri)

# Input
HEADERS += src/DotPlotSplitter.h \
           src/DotPlotDialog.h \
           src/DotPlotFilesDialog.h \
           src/DotPlotTasks.h \
           src/DotPlotClasses.h \
           src/DotPlotWidget.h \
           src/DotPlotPlugin.h

FORMS +=   src/ui/DotPlotDialog.ui \
           src/ui/DotPlotFilesDialog.ui

SOURCES += src/DotPlotSplitter.cpp \
           src/DotPlotDialog.cpp \
           src/DotPlotFilesDialog.cpp \
           src/DotPlotTasks.cpp \
           src/DotPlotClasses.cpp \
           src/DotPlotWidget.cpp \
           src/DotPlotPlugin.cpp

RESOURCES += dotplot.qrc
TRANSLATIONS += transl/english.ts transl/russian.ts
