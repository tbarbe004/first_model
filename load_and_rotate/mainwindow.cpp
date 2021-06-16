#include "mainwindow.h"

Window::Window()
{
    m_fallbackSurface = QRhiGles2InitParams::newFallbackSurface();
    QRhiGles2InitParams params;
    params.fallbackSurface = m_fallbackSurface;
    params.window = this;
    m_r = QRhi::create(QRhi::OpenGLES2, &params, rhiFlags);

    m_sc = m_r->newSwapChain();
    // allow depth-stencil, although we do not actually enable depth test/write for the triangle
    m_ds = m_r->newRenderBuffer(QRhiRenderBuffer::DepthStencil,
                                QSize(), // no need to set the size here, due to UsedWithSwapChainOnly
                                sampleCount,
                                QRhiRenderBuffer::UsedWithSwapChainOnly);
    m_sc->setWindow(this);
    m_sc->setDepthStencil(m_ds);
    m_sc->setSampleCount(sampleCount);
    m_sc->setFlags(scFlags);
    m_rp = m_sc->newCompatibleRenderPassDescriptor();
    m_sc->setRenderPassDescriptor(m_rp);
}

Window::~Window()
{
    delete m_rp;
    m_rp = nullptr;

    delete m_ds;
    m_ds = nullptr;

    delete m_sc;
    m_sc = nullptr;

    delete m_r;
    m_r = nullptr;

    delete m_fallbackSurface;
    m_fallbackSurface = nullptr;
}
