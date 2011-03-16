include (biostruct3d_view.pri)

# Input
HEADERS += src/BallAndStickGLRenderer.h \
           src/BioStruct3DColorScheme.h \
           src/BioStruct3DGLRender.h \
           src/BioStruct3DGLWidget.h \
           src/BioStruct3DSplitter.h \
           src/BioStruct3DViewPlugin.h \
           src/ExportImageDialog.h \
           src/GLFrameManager.h \
           src/GraphicUtils.h \
           src/MolecularSurfaceRenderer.h \
           src/TubeGLRenderer.h \
           src/VanDerWaalsGLRenderer.h \
           src/WormsGLRenderer.h \
           src/SettingsDialog.h \
           src/StructuralAlignmentDialog.h \
           src/gl2ps/gl2ps.h

FORMS +=   src/ui/ExportImageDialog.ui \
           src/ui/SettingsDialog.ui \
           src/ui/StructuralAlignmentDialog.ui

SOURCES += src/BallAndStickGLRenderer.cpp \
           src/BioStruct3DColorScheme.cpp \
           src/BioStruct3DGLRender.cpp \
           src/BioStruct3DGLWidget.cpp \
           src/BioStruct3DSplitter.cpp \
           src/BioStruct3DViewPlugin.cpp \
           src/ExportImageDialog.cpp \
           src/GLFrameManager.cpp \
           src/GraphicUtils.cpp \
           src/MolecularSurfaceRenderer.cpp \
           src/TubeGLRenderer.cpp \
           src/VanDerWaalsGLRenderer.cpp \
           src/WormsGLRenderer.cpp \
           src/SettingsDialog.cpp \
           src/StructuralAlignmentDialog.cpp \
           src/gl2ps/gl2ps.cpp

RESOURCES += biostruct3d_view.qrc
TRANSLATIONS += transl/english.ts transl/russian.ts

