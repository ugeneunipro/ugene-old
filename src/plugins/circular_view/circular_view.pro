include (circular_view.pri)

# Input
HEADERS += src/CircularItems.h \
           src/CircularView.h \
           src/CircularViewPlugin.h \
           src/CircularViewSplitter.h \
           src/ExportImageCircularViewDialog.h \
           src/RestrictionMapWidget.h
SOURCES += src/CircularItems.cpp \
           src/CircularView.cpp \
           src/CircularViewPlugin.cpp \
           src/CircularViewSplitter.cpp \
           src/ExportImageCircularViewDialog.cpp \
           src/RestrictionMapWidget.cpp
RESOURCES += circular_view.qrc
TRANSLATIONS += transl/czech.ts transl/english.ts transl/russian.ts
