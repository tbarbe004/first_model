#include "mainwindow.h"

Window::Window(float *vertexData)
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

    d.data = vertexData;
    d.meshRenderer.setRhi(m_r);
    d.meshRenderer.setSampleCount(sampleCount);
    d.meshRenderer.initResources(m_rp, sizeof(vertexData));
}

Window::~Window()
{
    d.meshRenderer.releaseResources();

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

void Window::resizeSwapChain()
{
    m_hasSwapChain = m_sc->buildOrResize(); // also handles m_ds

    m_frameCount = 0;
    m_timer.restart();

    const QSize outputSize = m_sc->currentPixelSize();
    m_proj = m_r->clipSpaceCorrMatrix();
    m_proj.perspective(45.0f, outputSize.width() / (float) outputSize.height(), 0.01f, 1000.0f);
    m_proj.translate(0, 0, -4);
}

void Window::releaseSwapChain()
{
    if (m_hasSwapChain) {
        m_hasSwapChain = false;
        m_sc->release();
    }
}

void Window::render()
{
    if (!m_hasSwapChain || m_notExposed)
        return;

    // If the window got resized or got newly exposed, resize the swapchain.
    // (the newly-exposed case is not actually required by some
    // platforms/backends, but f.ex. Vulkan on Windows seems to need it)
    if (m_sc->currentPixelSize() != m_sc->surfacePixelSize() || m_newlyExposed) {
        resizeSwapChain();
        if (!m_hasSwapChain)
            return;
        m_newlyExposed = false;
    }

    // Start a new frame. This is where we block when too far ahead of
    // GPU/present, and that's what throttles the thread to the refresh rate.
    // (except for OpenGL where it happens either in endFrame or somewhere else
    // depending on the GL implementation)
    QRhi::FrameOpResult r = m_r->beginFrame(m_sc, beginFrameFlags);
    if (r == QRhi::FrameOpSwapChainOutOfDate) {
        resizeSwapChain();
        if (!m_hasSwapChain)
            return;
        r = m_r->beginFrame(m_sc);
    }
    if (r != QRhi::FrameOpSuccess) {
        requestUpdate();
        return;
    }

    m_frameCount += 1;
    if (m_timer.elapsed() > 1000) {
        if (rhiFlags.testFlag(QRhi::EnableProfiling)) {
            const QRhiProfiler::CpuTime ff = m_r->profiler()->frameToFrameTimes(m_sc);
            const QRhiProfiler::CpuTime be = m_r->profiler()->frameBuildTimes(m_sc);
            const QRhiProfiler::GpuTime gp = m_r->profiler()->gpuFrameTimes(m_sc);
            if (m_r->isFeatureSupported(QRhi::Timestamps)) {
                qDebug("ca. %d fps. "
                       "frame-to-frame: min %lld max %lld avg %f. "
                       "frame build: min %lld max %lld avg %f. "
                       "gpu frame time: min %f max %f avg %f",
                       m_frameCount,
                       ff.minTime, ff.maxTime, ff.avgTime,
                       be.minTime, be.maxTime, be.avgTime,
                       gp.minTime, gp.maxTime, gp.avgTime);
            } else {
                qDebug("ca. %d fps. "
                       "frame-to-frame: min %lld max %lld avg %f. "
                       "frame build: min %lld max %lld avg %f. ",
                       m_frameCount,
                       ff.minTime, ff.maxTime, ff.avgTime,
                       be.minTime, be.maxTime, be.avgTime);
            }
        } else {
            qDebug("ca. %d fps", m_frameCount);
        }

        m_timer.restart();
        m_frameCount = 0;
    }

    customRender();

    m_r->endFrame(m_sc, endFrameFlags);

    if (!scFlags.testFlag(QRhiSwapChain::NoVSync))
        requestUpdate();
    else // try prevent all delays when NoVSync
        QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
}

void Window::customRender()
{
    const QSize outputSize = m_sc->currentPixelSize();
    QRhiCommandBuffer *cb = m_sc->currentFrameCommandBuffer();

    if (outputSize != d.lastOutputSize) {
            d.meshRenderer.resize(outputSize);
        d.lastOutputSize = outputSize;
    }

    QRhiResourceUpdateBatch *u = m_r->nextResourceUpdateBatch();

    d.meshRenderer.queueResourceUpdates(u, d.data);

    cb->beginPass(m_sc->currentFrameRenderTarget(), m_clearColor, { 1.0f, 0 }, u);

    cb->debugMarkBegin(QByteArrayLiteral("Cube"));
    d.meshRenderer.queueDraw(cb, outputSize, sizeof(d.data));
    cb->debugMarkEnd();

    QRhiResourceUpdateBatch *passEndUpdates = nullptr;

    cb->endPass(passEndUpdates);

    d.frameCount += 1;
}
