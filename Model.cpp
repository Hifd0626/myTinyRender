#include<fstream>
#include "Model.h"
#include "tgaimage.h"
#include <iostream>
#include <vector>
#include <sstream>

model::model(const char *filename){
    std::ifstream file;
    file.open(filename);
    if(file.fail()){
        std::cerr << "Unable to open file" << std::endl;
        return;
    }
    std::string line;
    while(!file.eof()){
        std::getline(file, line);
        std::istringstream iss(line.c_str());
        char junk;
        if(!line.compare(0, 2, "v ")){
            iss >> junk;
            double x, y, z;
            iss>>x>>y>>z;
            verts.emplace_back(x,y,z);
        }
        else if(!line.compare(0, 2, "f ")){
            int vertexIndex[3], textureIndex[3];
            iss >> junk;
            int first, second, third;
            for(int i=0;i<3;i++){
                iss >> first >> junk >> second >> junk >> third;
                vertexIndex[i] = --first;
                textureIndex[i] = --second;
            }
            faces.emplace_back(vertexIndex[0], vertexIndex[1], vertexIndex[2], textureIndex[0], textureIndex[1], textureIndex[2]);
        }
        else if(!line.compare(0, 3, "vt ")){
            iss >>junk>>junk;
            double u, v;
            iss>>u>>v;
            textures.emplace_back(u, v);
        }
    }
}