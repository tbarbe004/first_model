#include "meshrenderer.h"
#include <QFile>
#include <QtGui/private/qshader_p.h>

//#define VBUF_IS_DYNAMIC

static QShader getShader(const QString &name)
{
    QFile f(name);
    if (f.open(QIODevice::ReadOnly))
        return QShader::fromSerialized(f.readAll());

    return QShader();
}

void meshrenderer::initResources(QRhiRenderPassDescriptor *rp, float *vertexData)
{
#ifdef VBUF_IS_DYNAMIC
    m_vbuf = m_r->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::VertexBuffer, sizeof(vertexData));
#else
    m_vbuf = m_r->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(vertexData));
#endif
    m_vbuf->setName(QByteArrayLiteral("Triangle vbuf"));
    m_vbuf->build();
    m_vbufReady = false;

    m_ubuf = m_r->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 68);
    m_ubuf->setName(QByteArrayLiteral("Triangle ubuf"));
    m_ubuf->build();

    m_srb = m_r->newShaderResourceBindings();
    m_srb->setBindings({
        QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage, m_ubuf)
    });
    m_srb->build();

    m_ps = m_r->newGraphicsPipeline();

    QRhiGraphicsPipeline::TargetBlend premulAlphaBlend; // convenient defaults...
    premulAlphaBlend.enable = true;
    QVarLengthArray<QRhiGraphicsPipeline::TargetBlend, 4> rtblends;
    for (int i = 0; i < m_colorAttCount; ++i)
        rtblends << premulAlphaBlend;

    m_ps->setTargetBlends(rtblends.cbegin(), rtblends.cend());
    m_ps->setSampleCount(m_sampleCount);

    if (m_depthWrite) { // TriangleOnCube may want to exercise this
        m_ps->setDepthTest(true);
        m_ps->setDepthOp(QRhiGraphicsPipeline::Always);
        m_ps->setDepthWrite(true);
    }

    QShader vs = getShader(QLatin1String(":/color.vert.qsb"));
    Q_ASSERT(vs.isValid());
    QShader fs = getShader(QLatin1String(":/color.frag.qsb"));
    Q_ASSERT(fs.isValid());
    m_ps->setShaderStages({
        { QRhiShaderStage::Vertex, vs },
        { QRhiShaderStage::Fragment, fs }
    });

    QRhiVertexInputLayout inputLayout;
    inputLayout.setBindings({
        { 7 * sizeof(float) }
    });
    inputLayout.setAttributes({
        { 0, 0, QRhiVertexInputAttribute::Float2, 0 },
        { 0, 1, QRhiVertexInputAttribute::Float3, 2 * sizeof(float) }
    });

    m_ps->setVertexInputLayout(inputLayout);
    m_ps->setShaderResourceBindings(m_srb);
    m_ps->setRenderPassDescriptor(rp);

    m_ps->build();
}

void meshrenderer::queueDraw(QRhiCommandBuffer *cb, const QSize &outputSizeInPixels)
{
    cb->setGraphicsPipeline(m_ps);
    cb->setViewport(QRhiViewport(0, 0, outputSizeInPixels.width(), outputSizeInPixels.height()));
    cb->setShaderResources();
    const QRhiCommandBuffer::VertexInput vbufBinding(m_vbuf, 0);
    cb->setVertexInput(0, 1, &vbufBinding);
    cb->draw(3);
}
