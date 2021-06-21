#ifndef MESHRENDERER_H
#define MESHRENDERER_H
#include <QtGui/private/qrhi_p.h>
#include "loadmesh.h"

class meshrenderer
{
public:
    void setRhi(QRhi *r) { m_r = r; }
    void setSampleCount(int samples) { m_sampleCount = samples; }
    void setTranslation(const QVector3D &v) { m_translation = v; }
    void initResources(QRhiRenderPassDescriptor *rp);
    void releaseResources();
    void resize(const QSize &pixelSize);
    void queueResourceUpdates(QRhiResourceUpdateBatch *resourceUpdates);
    void queueDraw(QRhiCommandBuffer *cb, const QSize &outputSizeInPixels);

private:
    QRhi *m_r;

    QRhiBuffer *m_vbuf = nullptr;
    bool m_vbufReady = false;
    QRhiBuffer *m_ubuf = nullptr;
    QRhiBuffer *m_scale = nullptr;
    QImage m_image;
    QRhiTexture *m_tex = nullptr;
    QRhiSampler *m_sampler = nullptr;
    QRhiShaderResourceBindings *m_srb = nullptr;
    QRhiGraphicsPipeline *m_ps = nullptr;

    QVector3D m_translation;
    QMatrix4x4 m_proj;
    float m_rotation = 0;
    int m_sampleCount = 1; // no MSAA by default
};

#endif // MESHRENDERER_H
