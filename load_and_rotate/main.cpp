#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "tiny_obj_loader.h"
#include "mainwindow.h"
#include "meshrenderer.h"
#include <iostream>
#include <QApplication>

float* attrib_to_data(tinyobj::ObjReader reader, std::string inputfile, tinyobj::ObjReaderConfig reader_config){
    if (!reader.ParseFromFile(inputfile, reader_config)) {
      if (!reader.Error().empty()) {
          std::cerr << "TinyObjReader: " << reader.Error();
      }
      exit(1);
    }

    if (!reader.Warning().empty()) {
      std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();

    unsigned long data_length = 0;

    for (size_t s = 0; s < shapes.size(); s++) {
      size_t index_offset = 0;
      for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
        size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

        // Loop over vertices in the face.
        for (size_t v = 0; v < fv; v++) {

          tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
          data_length += 3;
          if (idx.normal_index >= 0) {
              data_length += 3;

          }

          if (idx.texcoord_index >= 0) {
              data_length += 2;
          }

         }
        index_offset += fv;
      }
    }

    float* data = (float *) malloc(data_length * sizeof(float));

    int cursor = 0;

    for (size_t s = 0; s < shapes.size(); s++) {
      // Loop over faces(polygon)
      size_t index_offset = 0;
      for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
        size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

        // Loop over vertices in the face.
        for (size_t v = 0; v < fv; v++) {
          // access to vertex
          tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
          data[cursor] = attrib.vertices[3*size_t(idx.vertex_index)+0];
          data[cursor + 1] = attrib.vertices[3*size_t(idx.vertex_index)+1];
          data[cursor + 2] = attrib.vertices[3*size_t(idx.vertex_index)+2];

          cursor += 3;

          // Check if `normal_index` is zero or positive. negative = no normal data
          /*if (idx.normal_index >= 0) {
            data[cursor] = attrib.normals[3*size_t(idx.normal_index)+0];
            data[cursor + 1] = attrib.normals[3*size_t(idx.normal_index)+1];
            data[cursor + 2] = attrib.normals[3*size_t(idx.normal_index)+2];

            cursor += 3;
          }*/

          // Check if `texcoord_index` is zero or positive. negative = no texcoord data
          //if (idx.texcoord_index >= 0) {
          data[cursor] = attrib.texcoords[2*size_t(idx.texcoord_index)+0];
          data[cursor + 1] = attrib.texcoords[2*size_t(idx.texcoord_index)+1];

          cursor += 2;
          //}

        }
        index_offset += fv;

      }
    }


    return data;
}

/*int main(int argc, char **argv)
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    QLoggingCategory::setFilterRules(QLatin1String("qt.rhi.*=true"));

    // Defaults.
#if defined(Q_OS_WIN)
    graphicsApi = D3D11;
#elif defined(Q_OS_MACOS) || defined(Q_OS_IOS)
    graphicsApi = Metal;
#elif QT_CONFIG(vulkan)
    graphicsApi = Vulkan;
#else
    graphicsApi = OpenGL;
#endif

    // Allow overriding via the command line.
    QCommandLineParser cmdLineParser;
    cmdLineParser.addHelpOption();
    QCommandLineOption nullOption({ "n", "null" }, QLatin1String("Null"));
    cmdLineParser.addOption(nullOption);
    QCommandLineOption glOption({ "g", "opengl" }, QLatin1String("OpenGL (2.x)"));
    cmdLineParser.addOption(glOption);
    QCommandLineOption vkOption({ "v", "vulkan" }, QLatin1String("Vulkan"));
    cmdLineParser.addOption(vkOption);
    QCommandLineOption d3dOption({ "d", "d3d11" }, QLatin1String("Direct3D 11"));
    cmdLineParser.addOption(d3dOption);
    QCommandLineOption mtlOption({ "m", "metal" }, QLatin1String("Metal"));
    cmdLineParser.addOption(mtlOption);
    // Testing cleanup both with QWindow::close() (hitting X or Alt-F4) and
    // QCoreApplication::quit() (e.g. what a menu widget would do) is important.
    // Use this parameter for the latter.
    QCommandLineOption sdOption({ "s", "self-destruct" }, QLatin1String("Self-destruct after 5 seconds."));
    cmdLineParser.addOption(sdOption);
    // Attempt testing device lost situations on D3D at least.
    QCommandLineOption tdrOption(QLatin1String("curse"), QLatin1String("Curse the graphics device. "
                                                        "(generate a device reset every <count> frames when on D3D11)"),
                                 QLatin1String("count"));
    cmdLineParser.addOption(tdrOption);
    // Allow testing preferring the software adapter (D3D).
    QCommandLineOption swOption(QLatin1String("software"), QLatin1String("Prefer a software renderer when choosing the adapter. "
                                                                         "Only applicable with some APIs and platforms."));
    cmdLineParser.addOption(swOption);
    // Allow testing having a semi-transparent window.
    QCommandLineOption transparentOption(QLatin1String("transparent"), QLatin1String("Make background transparent"));
    cmdLineParser.addOption(transparentOption);

    cmdLineParser.process(app);
    if (cmdLineParser.isSet(nullOption))
        graphicsApi = Null;
    if (cmdLineParser.isSet(glOption))
        graphicsApi = OpenGL;
    if (cmdLineParser.isSet(vkOption))
        graphicsApi = Vulkan;
    if (cmdLineParser.isSet(d3dOption))
        graphicsApi = D3D11;
    if (cmdLineParser.isSet(mtlOption))
        graphicsApi = Metal;

    qDebug("Selected graphics API is %s", qPrintable(graphicsApiName()));
    qDebug("This is a multi-api example, use command line arguments to override:\n%s", qPrintable(cmdLineParser.helpText()));

    if (cmdLineParser.isSet(transparentOption)) {
        transparentBackground = true;
        scFlags |= QRhiSwapChain::SurfaceHasPreMulAlpha;
    }

#ifdef EXAMPLEFW_PREINIT
    void preInit();
    preInit();
#endif

    // OpenGL specifics.
    QSurfaceFormat fmt;
    fmt.setDepthBufferSize(24);
    fmt.setStencilBufferSize(8);
    if (sampleCount > 1)
        fmt.setSamples(sampleCount);
    if (scFlags.testFlag(QRhiSwapChain::NoVSync))
        fmt.setSwapInterval(0);
    if (scFlags.testFlag(QRhiSwapChain::sRGB))
        fmt.setColorSpace(QSurfaceFormat::sRGBColorSpace);
    // Exception: The alpha size is not necessarily OpenGL specific.
    if (transparentBackground)
        fmt.setAlphaBufferSize(8);
    QSurfaceFormat::setDefaultFormat(fmt);



    if (cmdLineParser.isSet(tdrOption))
        framesUntilTdr = cmdLineParser.value(tdrOption).toInt();

    if (cmdLineParser.isSet(swOption))
        rhiFlags |= QRhi::PreferSoftwareRenderer;

    // Create and show the window.
    Window w;

#if QT_CONFIG(vulkan)
    //if (graphicsApi == Vulkan)
        //1w.setVulkanInstance(&inst);
#endif


    std::string inputfile = "ponte.obj";
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = "./"; // Path to material files

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(inputfile, reader_config)) {
      if (!reader.Error().empty()) {
          std::cerr << "TinyObjReader: " << reader.Error();
      }
      exit(1);
    }

    if (!reader.Warning().empty()) {
      std::cout << "TinyObjReader: " << reader.Warning();
    }

    float * data = attrib_to_data(reader, inputfile, reader_config);

    std::cout << "data : " << data ;

    //auto& attrib = reader.GetAttrib();
    //auto& shapes = reader.GetShapes();
    //auto& materials = reader.GetMaterials();

    #ifdef PROFILE_TO_FILE
        m_r->profiler()->setDevice(&d.profOut);
    #endif

    meshrenderer m ;

    m.setRhi(w.m_r);
    m.setSampleCount(sampleCount);
    m.initResources(w.m_rp, data);

    free(data);


    w.resize(1280, 720);
    w.setTitle(QCoreApplication::applicationName() + QLatin1String(" - ") + graphicsApiName());
    w.show();

    if (cmdLineParser.isSet(sdOption))
        QTimer::singleShot(5000, qGuiApp, SLOT(quit()));

    int ret = app.exec();

    // Window::event() will not get invoked when the
    // PlatformSurfaceAboutToBeDestroyed event is sent during the QWindow
    // destruction. That happens only when exiting via app::quit() instead of
    // the more common QWindow::close(). Take care of it: if the
    // QPlatformWindow is still around (there was no close() yet), get rid of
    // the swapchain while it's not too late.
    if (w.handle())
        w.releaseSwapChain();

    return ret;
}*/


int main(int argc, char *argv[])
{

    std::string inputfile = "ponte.obj";
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = "./"; // Path to material files

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(inputfile, reader_config)) {
      if (!reader.Error().empty()) {
          std::cerr << "TinyObjReader: " << reader.Error();
      }
      exit(1);
    }

    if (!reader.Warning().empty()) {
      std::cout << "TinyObjReader: " << reader.Warning();
    }

    float * data = attrib_to_data(reader, inputfile, reader_config);

    std::cout << "data : " << data ;

    //auto& attrib = reader.GetAttrib();
    //auto& shapes = reader.GetShapes();
    //auto& materials = reader.GetMaterials();

    #ifdef PROFILE_TO_FILE
        m_r->profiler()->setDevice(&d.profOut);
    #endif
    QGuiApplication a(argc, argv);
    Window w(data);



    //free(data);

    w.resize(1280, 720);
    w.setTitle(QCoreApplication::applicationName());
    w.show();
    return a.exec();
}
