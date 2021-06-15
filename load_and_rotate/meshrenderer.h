#ifndef MESHRENDERER_H
#define MESHRENDERER_H
#include <QtGui/private/qrhi_p.h>

class meshrenderer
{
public:
    void initResources(QRhiRenderPassDescriptor *rp, float *vertexData);
    void queueDraw(QRhiCommandBuffer *cb, const QSize &outputSizeInPixels);

private:
    QRhi *m_r;

    QRhiBuffer *m_vbuf = nullptr;
    bool m_vbufReady = false;
    QRhiBuffer *m_ubuf = nullptr;
    QRhiShaderResourceBindings *m_srb = nullptr;
    QRhiGraphicsPipeline *m_ps = nullptr;

    bool m_depthWrite = false;
    int m_colorAttCount = 1;
    int m_sampleCount = 1; // no MSAA by default
};

#endif // MESHRENDERER_H
