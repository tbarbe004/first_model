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

    meshrenderer m ;

    //m.setRhi(m_r);
    //m.setSampleCount(sampleCount);
    //m.initResources(m_rp);


    free(data);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
