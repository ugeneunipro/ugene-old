/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef _U2_OPENGL_WIDGET_H_
#define _U2_OPENGL_WIDGET_H_

#include <QtOpenGL/QGLWidget>
#include <QtGui/QMenu>
#include <QtGui/QActionGroup>
#include <QtGui/QAction>
#include <QtGui/QColor>
#include <QtCore/QTimer>


#include <U2Core/Vector3D.h>
#include <U2Core/U2Region.h>
#include <memory>

namespace U2 { 

class Document;
class BioStruct3DObject;
class BioStruct3D;
class BioStruct3DGLRenderer;
class BioStruct3DColorScheme;
class BioStruct3DColorSchemeFactory;
class BioStruct3DGLRendererFactory;
class MolecularSurfaceRendererFactory;
class AnnotatedDNAView;
class Annotation;
class AnnotationTableObject;
class MolecularSurfaceCalcTask;
class Task;
class AnnotationSelection;
class DNASequenceObject;
class ADVSequenceObjectContext;
class LRegionsSelection;
class GLFrameManager;
class GLFrame;
class MolecularSurface;
class MolecularSurfaceRenderer;


/*!
* @class BioStruct3DGLWidget BioStruct3DGLWidget.h
* @brief Class for BioStruct3DObject visualization.
*
* This widget provides a 3D graphical view of a macromolecular structure.   In terms
* of the Model-View architecture we consider
* the BioStruct3D the model and GLWidget a view of this model.
*
* The widget relies on various GLRenderer and ColorScheme subclasses to handle the
* rendering of the 3d objects.
*
* Also it includes actions for visualization control.
*/ 
class BioStruct3DGLWidget : public QGLWidget
{
    Q_OBJECT
    static const int GL_MATRIX_SIZE = 16;
    static int widgetCount;

public:
    QString BACKGROUND_COLOR_NAME;

    QString PRODUCT_NAME;
    QString PLUGIN_NAME;
    QString COLOR_SCHEME_NAME;
    QString RENDERER_NAME;
    QString OBJECT_ID_NAME;

    QString SELECTION_COLOR_NAME;

    QString LEFT_EYE_COLOR_NAME;
    QString RIGHT_EYE_COLOR_NAME;

    QString RENDER_DETAIL_LEVEL_NAME;
    QString SHADING_LEVEL_NAME;
    QString ANAGLYPH_NAME;
    QString EYES_SHIFT_NAME;


    /*!
    * Constructor.
    * @param bsObj BioStruct3DObject contains 3d model for visualization
    * @param view DnaView context for connecting structure 3D representation and sequence view
    * @param manager GlFrameManager is required for OpenGL frame manipulation
    * @param parent Parent widget
    */ 
    BioStruct3DGLWidget(const BioStruct3DObject* bsObj, const AnnotatedDNAView* view, GLFrameManager* manager, QWidget *parent);
    /*!
    * Destructor.
    */ 
    ~BioStruct3DGLWidget();
    /*!
    * @return Secondary structure annotation colors obtained from AppContext.
    */
    const QMap<QString, QColor> getSecStructAnnotationColors() const;
    /*!
    * @return Molecular chain colors obtained from AppContext.
    */
    const QMap<int, QColor> getChainColors() const;
    /*!
    * @return Contexts for corresponding biostruct3d primary structure (sequences).
    */
    const QList<ADVSequenceObjectContext*> getSequenceContexts() const;
    /*!
    * @return Indices of biostruct3d models to be drawn.
    */
    const QList<int> getActiveModelIndexList() const { return activeModelIndexList; }
    /*!
    * @return BioStruct3D const reference.
    */
    const BioStruct3D& getBioStruct3D() const { return biostruc; }
    /*!
    * @return BioStruct3DObject name.
    */
    const QString getBioStruct3DObjectName() const;
    /*!
    * @return BioStruct3D Protein Data Bank id.
    */
    const QString getPDBId() const;
    /*!
    * @return Menu containing display actions: renderers, color schemes etc.
    */
    QMenu* getDisplayMenu();
    /*!
    * Records widget active settings (part of UGENE architecture).
    * @return Current widgets settings.
    */
    QVariantMap getState();
    /*!
    * Restores widget settings with given (part of UGENE architecture).
    * @param state Widgets settings.
    */
    void setState(const QVariantMap& state);
    /*!
    * Restores default widget settings (viewMatrix, glRenderer, color scheme)
    */
    void restoreDefaultSettigns();
    /*!
    * @param delta Positive value: zoom in, negative value: zoom out.
    */
    void zoom(float delta);
    void shift(float deltaX, float deltaY);
    /*!
    * Creates 2d vector image of BioStruct3DGLWidget contents using gl2ps.
    */
    void writeImage2DToFile(int format, int options, int nbcol, const char *fileName);

    float getEyesShiftMult() const;
    float getRenderDetailLevel() const;
    void setBackgroundColor(QColor backgroundColor);
    float sumCorrectColors(float a, float b);

signals:
    void si_widgetClosed(BioStruct3DGLWidget* w);

protected:

    /*!
    * QGlWidget virtual function, initializes OpenGL params. See, Qt docs "QGLWidget" for details. 
    */
    void initializeGL();
    /*!
    * QGlWidget virtual function, sets viewport
    * @param width GLWidget width
    * @param height GLWidget height
    */
    void resizeGL(int width, int height);
    /*!
    * QGlWidget virtual function, draw GL scene.  
    */
    void paintGL();
    /*!
    * QWidget virtual function, mouse button down event handler.
    */
    void mousePressEvent(QMouseEvent *event);
    /*!
    * QWidget virtual function, mouse move event handler.
    */
    void mouseMoveEvent(QMouseEvent *event);
    /*!
    * QWidget virtual function, mouse wheel event handler.
    */
    void wheelEvent ( QWheelEvent * event );
    /*!
    * QWidget virtual function, executes context menu.
    */
    void contextMenuEvent(QContextMenuEvent *_event);
    
private:

    /*!
    * Helper function called by all constructors.
    */ 
    void setBioStruct3DColorScheme(BioStruct3DColorScheme* clScheme);
    /*!
    * Sets active renderer.
    */
    void setBioStruct3DRenderer(BioStruct3DGLRenderer* renderer);
    /*!
    * Sets unselected regions shading level
    */
    void setUnselectedShadingLevel(int shading);
    /*!
    * @param pos New light source position. Directional light is being used.
    */
    void setLightPosition(const Vector3D& pos);
    /*!
    * Creates actions for existing GLRenderers, loads default renderer.
    */
    void loadGLRenderers();
    /*!
    * Creates actions for existing ColorSchemes, loads default color scheme.
    */
    void loadColorSchemes();
    void createActions();
    void createMenus();
    void createActiveModelIndexList();
    void saveDefaultSettings();
    BioStruct3DColorScheme* createCustomColorScheme(const QString& name); 
    BioStruct3DGLRenderer* createCustomRenderer(const QString& name);
    int getChainIdForAnnotationObject(AnnotationTableObject* ao); 
    void switchActiveModel(bool forward);
    void connectExternalSignals();

    unsigned int* emptyTextureData;
    GLuint anaglyphRenderTextureLeft, anaglyphRenderTextureRight, tempAnaglyphRenderTexture;

    void ViewOrtho();                                               // Set Up An Ortho View
    void ViewPerspective();                                         // Set Up A Perspective View
    GLuint getEmptyTexture(int textureWidth, int textureHeight);    // Create An Empty Texture

    void draw();
    void drawTexturesAnaglyph(GLuint anaglyphRenderTextureLeft, GLuint anaglyphRenderTextureRight);                                // Draw The Image
    void drawTexture(GLuint anaglyphRenderTexture, int red, int green, int blue, float alpha, bool alphaOnly);                              // Draw The Blurred Image
    void drawAll();
    void drawColoredPlane(int red, int green, int blue, float alpha);

    bool hasGlErrors();

    Vector3D getTrackballMapping(int x, int y);
    static int getSequenceChainId(const DNASequenceObject* seqObj);
    static int getWidgetCount(QString objectName);
    bool isSyncModeOn();

private:
    const QList<const BioStruct3DObject*> biostructs;
    const BioStruct3D& biostruc;
    const Document* biostrucDoc;
    const AnnotatedDNAView* dnaView;
    GLFrameManager* frameManager;
    std::auto_ptr<MolecularSurface> molSurface;
    std::auto_ptr<GLFrame> glFrame;
    std::auto_ptr<BioStruct3DColorScheme> colorScheme;
    std::auto_ptr<BioStruct3DGLRenderer> biostructRenderer;
    std::auto_ptr<MolecularSurfaceRenderer> surfaceRenderer;
    QVariantMap defaultsSettings;
    MolecularSurfaceCalcTask* surfaceCalcTask;
    QString currentColorSchemeName;
    QString currentGLRendererName;
    QString currentModelID;
    QMap<QString, BioStruct3DColorSchemeFactory*> colorSchemeFactoryMap;
    QMap<QString, BioStruct3DGLRendererFactory*> rendererFactoryMap;
    QMap<QString, MolecularSurfaceRendererFactory*> surfaceRendererFactoryMap;
    QMap<const AnnotationTableObject*, int> chainIdCache;
    QList<int> activeModelIndexList;
    GLfloat cameraClipNear, cameraClipFar; //cameraDistance, 

    GLfloat rotAngle, spinAngle;
    GLfloat lightPostion[4];
    Vector3D rotAxis, lastPos;
    QAction *spinAction;
    QAction *settingsAction;
    QAction *closeAction;
    QAction *exportImageAction;
    QAction *setNextModelAction, *setPrevModelAction, *setAllModelsActiveAction;
    QActionGroup *colorSchemeActions;
    QActionGroup *rendererActions;
    QActionGroup *molSurfaceRenderActions;
    QActionGroup *molSurfaceTypeActions;
    QTimer* animationTimer;
    QMenu *selectColorSchemeMenu;
    QMenu *selectRendererMenu;
    QMenu *displayMenu;
    QMenu *modelsMenu;
    QColor backgroundColor;
    QColor selectionColor;

    bool multipleModels;

    QColor leftEyeColor;
    QColor rightEyeColor;

    int renderDetailLevel;
    int unselectedShadingLevel;
    bool anaglyph, anaglyphAvailable, firstResize;
    int eyesShift;

 private slots:
     void sl_selectColorScheme(QAction* action);
     void sl_selectGLRenderer(QAction* action);
     void sl_updateRenderSettings(const QStringList& list);
     void sl_acitvateSpin();
     void sl_onSequenceSelectionChanged(LRegionsSelection* s, const QVector<U2Region>& r, const QVector<U2Region>& a);
     void sl_onAnnotationSelectionChanged(AnnotationSelection* thiz, const QList<Annotation*>& added, const QList<Annotation*>& removed);
     void sl_updateAnnimation();
     void sl_settings();
     void sl_closeWidget();
     void sl_exportImage();
     void sl_setNextModelAcitve();
     void sl_setPrevModelAcitve();
     void sl_setAllModelsActive();
     void sl_showSurface();
     void sl_hideSurface();
     void sl_selectSurfaceRenderer(QAction* surfaceRenderer);
     void sl_onTaskFinished(Task* task);
};

} //namespace


#endif // _U2_OPENGL_WIDGET_H
