#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGuiApplication>
#include <QCommandLineParser>
#include <QWindow>
#include <QPlatformSurfaceEvent>
#include <QElapsedTimer>
#include <QTimer>
#include <QLoggingCategory>

#include <QtGui/private/qshader_p.h>
#include <QFile>
#include <QtGui/private/qrhiprofiler_p.h>
#include <QtGui/private/qrhinull_p.h>

#ifndef QT_NO_OPENGL
#include <QtGui/private/qrhigles2_p.h>
#include <QOffscreenSurface>
#endif

#include "meshrenderer.h"

struct {
    meshrenderer meshRenderer;
    QSize lastOutputSize;
    int frameCount = 0;
    QFile profOut;
    float *data;
} d;

class Window : public QWindow
{
public:
    Window(float *vertexData);
    ~Window();
    void render();
    void resizeSwapChain();
    void releaseSwapChain();
    void customRender();
    QOffscreenSurface *m_fallbackSurface = nullptr;

    bool m_notExposed = false;
    bool m_newlyExposed = false;

    QRhi *m_r = nullptr;
    bool m_hasSwapChain = false;
    QRhiSwapChain *m_sc = nullptr;
    QRhiRenderBuffer *m_ds = nullptr;
    QRhiRenderPassDescriptor *m_rp = nullptr;

    QRhi::Flags rhiFlags = QRhi::EnableDebugMarkers;
    QRhi::BeginFrameFlags beginFrameFlags;
    QRhi::EndFrameFlags endFrameFlags;
    int sampleCount = 1;
    QRhiSwapChain::Flags scFlags;

    QElapsedTimer m_timer;
    int m_frameCount;

    QMatrix4x4 m_proj;

    QColor m_clearColor;
};


#endif // MAINWINDOW_H
