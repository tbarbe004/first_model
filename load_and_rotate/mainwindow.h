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

class Window : public QWindow
{
public:
    Window();
    ~Window();
    QOffscreenSurface *m_fallbackSurface = nullptr;
    QRhi *m_r = nullptr;
    QRhiSwapChain *m_sc = nullptr;
    QRhiRenderBuffer *m_ds = nullptr;
    QRhiRenderPassDescriptor *m_rp = nullptr;
    QRhi::Flags rhiFlags = QRhi::EnableDebugMarkers;
    int sampleCount = 1;
    QRhiSwapChain::Flags scFlags;

};


#endif // MAINWINDOW_H
