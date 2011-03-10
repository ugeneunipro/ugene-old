#include <U2Core/Vector3D.h>

#include "GLFrameManager.h"
#include <U2Core/Log.h>

namespace U2 {

const GLfloat GLFrame::DEFAULT_ZOOM = 45.0f;
//const Vector3D GLFrame::DEFAULT_CAMERA = Vector3D(0, 0, 0);

///////////////////////////////////////////////////////////////////////////////////////////
/// Matrix4x4

Matrix4x4::Matrix4x4()
{
    m[0] = 0; m[1] = 0; m[2] = 0; m[3] = 0;
    m[4] = 0; m[5] = 0; m[6] = 0; m[7] = 0;
    m[8] = 0; m[9] = 0; m[10] = 0; m[11] = 0;
    m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 0;
}

void Matrix4x4::loadIdentity()
{
    m[0] = 1; m[1] = 0; m[2] = 0; m[3] = 0;
    m[4] = 0; m[5] = 1; m[6] = 0; m[7] = 0;
    m[8] = 0; m[9] = 0; m[10] = 1; m[11] = 0;
    m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
}

float& Matrix4x4::operator[]( unsigned int i )
{
    static float err = 0.0;
    if (i < 16)
        return m[i];
    else  {
        qCritical("Vector operator [] access out of range : %d", i);
        return err;
    }
}

Matrix4x4& Matrix4x4::operator=( const Matrix4x4& matrix )
{
    if (this == &matrix) { 
        return *this;
    }
    memcpy((void*) m, (void*) matrix.m, sizeof(matrix.m) ); 
    return *this;

}

void Matrix4x4::load( QVariantList values )
{
    Q_ASSERT(values.size() == 16);
    for (int i = 0; i < 16; ++i) {
        m[i] = values.at(i).value<float>(); 
    }

}

QVariantList Matrix4x4::store()
{
    QVariantList values;
    for (int i = 0; i < 16; ++i ) {
        values.append(QVariant::fromValue(m[i]));
    }
    return values;
}

///////////////////////////////////////////////////////////////////////////////////////////
/// GLFrame

GLFrame::GLFrame(QGLWidget* widget, float cameraNear, float cameraFar, float cameraZ) : glWidget(widget), synchLock(false)
{
    zoomFactor = DEFAULT_ZOOM;
    rotMatrix.loadIdentity();
    cameraClipNear = cameraNear;
    cameraClipFar = cameraFar;

    cameraPosition.set(0.0f, 0.0f, cameraZ);

    // Set view settings
    //float scaleFactor = 2.5;
}

void GLFrame::performZoom( float delta )
{
    static const float maxZoom = 150.0;
    static const float minZoom = 2.0;
    zoomFactor += delta;
    if (zoomFactor < minZoom) {
        zoomFactor = minZoom;
        return;
    }

    if (zoomFactor > maxZoom) {
        zoomFactor = maxZoom;
        return;
    }
}

void GLFrame::performShift(float deltaX, float deltaY)
{
    static float defaultWidth=1092;
    Vector3D shiftVector;
    float frameFactor = glWidget->width()/defaultWidth;

    shiftVector.set(deltaX*zoomFactor*frameFactor, deltaY*zoomFactor*frameFactor, 0);
    cameraPosition+=shiftVector;
}

const Vector3D GLFrame::getCameraPosition()const
{
    return cameraPosition;
}

void GLFrame::setCameraPosition(float x, float y, float z)
{
    cameraPosition.set(x, y, z);
}

void GLFrame::rotateCamera(const Vector3D& rotAxis, float rotAngle )
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(rotAngle, rotAxis.x, rotAxis.y, rotAxis.z );
    glMultMatrixf(rotMatrix.getData());
    glGetFloatv( GL_MODELVIEW_MATRIX, rotMatrix.getData());
}

#define ZOOM_FACTOR_ID "ZOOM_FACTOR"
#define CAMERA_STATE_POSITION_X_ID "CAMERA_STATE_POSITION_X"
#define CAMERA_STATE_POSITION_Y_ID "CAMERA_STATE_POSITION_Y"
#define ROTATION_MATRIX_ID "ROTATION_MATRIX"

void GLFrame::setState( const QVariantMap& state )
{
    cameraPosition.x = state.value(CAMERA_STATE_POSITION_X_ID, 0).value<float>();
    cameraPosition.y = state.value(CAMERA_STATE_POSITION_Y_ID, 0).value<float>();

    zoomFactor = state.value(ZOOM_FACTOR_ID, DEFAULT_ZOOM).value<float>();
    QVariantList rotML = state.value(ROTATION_MATRIX_ID).value<QVariantList>();
    if (!rotML.isEmpty()) {
             rotMatrix.load(rotML);
    }
}

void GLFrame::writeStateToMap( QVariantMap& state )
{
    state[CAMERA_STATE_POSITION_X_ID] = QVariant::fromValue(cameraPosition.x);
    state[CAMERA_STATE_POSITION_Y_ID] = QVariant::fromValue(cameraPosition.y);

    state[ZOOM_FACTOR_ID] = QVariant::fromValue(zoomFactor);
    state[ROTATION_MATRIX_ID] = rotMatrix.store();
}


void GLFrame::updateViewPort( int width, int height )
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Set view settings
    GLfloat aspect = GLfloat(width) / height;
    gluPerspective(zoomFactor, aspect, cameraClipNear, cameraClipFar);
}

void GLFrame::updateViewPort()
{
    updateViewPort(glWidget->width(), glWidget->height());
}

GLFrameManager::~GLFrameManager()
{

}

void GLFrameManager::addGLFrame( GLFrame* glFrame)
{
    widgetFrameMap.insert(glFrame->getGLWidget(), glFrame);
}

GLFrame* GLFrameManager::getGLWidgetFrame( QGLWidget* widget )
{
    if (widgetFrameMap.contains(widget)) {
        return widgetFrameMap.value(widget);
    } else  {
        return NULL;
    }

}

QList<GLFrame*> GLFrameManager::getGLFrames()
{
    return widgetFrameMap.values();
}

void GLFrameManager::setSyncLock( bool lockOn, QGLWidget* syncWidget )
{
    syncLock = lockOn;
    if (lockOn) {
        GLFrame* syncFrame = getGLWidgetFrame(syncWidget);
        QVariantMap state;
        syncFrame->writeStateToMap(state);
        foreach(GLFrame* frame, widgetFrameMap.values()) {
            if (frame != syncFrame) {
                frame->makeCurrent();
                frame->setState(state);
                frame->updateViewPort();
                frame->updateGL();
            }
        }   
    }
}

void GLFrameManager::removeGLWidgetFrame( QGLWidget* widget )
{
    Q_ASSERT(widgetFrameMap.contains(widget));
    widgetFrameMap.remove(widget);
}

QList<GLFrame*> GLFrameManager::getActiveGLFrameList( GLFrame* currentFrame, bool syncModeOn )
{
    if (syncModeOn) {
        return widgetFrameMap.values();
    } else {
        QList<GLFrame*> lst;
        lst.append(currentFrame);
        return lst;
    }
}
} //namespace
