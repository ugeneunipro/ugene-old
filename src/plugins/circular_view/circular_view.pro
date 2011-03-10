include (circular_view.pri)

# Input
HEADERS += src/CircularView.h \
           src/CircularViewPlugin.h \
           src/CircularItems.h \
           src/CircularViewSplitter.h
SOURCES += src/CircularView.cpp \
           src/CircularViewPlugin.cpp \
           src/CircularItems.cpp \
           src/CircularViewSplitter.cpp
RESOURCES += circular_view.qrc
TRANSLATIONS += transl/english.ts transl/russian.ts
