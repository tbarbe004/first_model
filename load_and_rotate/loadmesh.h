#ifndef LOADMESH_H
#define LOADMESH_H
#include "tiny_obj_loader.h"
#include <iostream>
#include <QApplication>

float* attrib_to_data(tinyobj::ObjReader reader, std::string inputfile, tinyobj::ObjReaderConfig reader_config);

#endif // LOADMESH_H
