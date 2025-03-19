#pragma once
#include <vector>
#include <cmath>
#include <iostream>
class vert{
public:
    double x, y, z;
    vert(){}
    vert(double x, double y, double z) : x(x), y(y), z(z) {}

    //  subtraction
    vert operator-(const vert& other) const {
        return vert(x - other.x, y - other.y, z - other.z);
    }

    // addition
    vert operator+(const vert& other) const {
        return vert(x + other.x, y + other.y, z + other.z);
    }
    // cross product
    vert operator^(const vert& other) const {
        return vert(
            y * other.z - z * other.y,  // x 分量
            z * other.x - x * other.z,  // y 分量
            x * other.y - y * other.x   // z 分量
        );
    }
    double operator* (const vert& other) const {
        return x * other.x + y * other.y + z * other.z;
    }
    vert operator*(const double& other) const {
        return vert(x * other, y * other, z * other);
    }
    vert normalize() const {
        double length = std::sqrt(x * x + y * y + z * z);
        if (length != 0) {
            return vert(x / length, y / length, z / length);
        }
        return vert(x, y, z);
    }
    void print(){
        std::cout<<x<<" "<<y<<" "<<z<<'\n';
    }
    
};
class texture{
public:
    double u, v;
    texture(){}
    texture(double u, double v) : u(u), v(v) {}
};
class triplane{
public:
    int vertexIndex[3];
    int textureIndex[3];
    int normalIndex[3];
    triplane(){}
    triplane(int v1, int v2, int v3, int t1, int t2, int t3): vertexIndex{v1, v2, v3}, textureIndex{t1, t2, t3}{}
    triplane(int v1, int v2, int v3, int t1, int t2, int t3, int n1, int n2, int n3): vertexIndex{v1, v2, v3}, textureIndex{t1, t2, t3}, normalIndex{n1, n2, n3}{}
};

class vec2d{
public:
    int x, y;
    vec2d(){}
    vec2d(int x, int y) : x(x), y(y) {}
};

class vec3d{
public:
    int x, y;
    double z;
    vec3d(){}
    vec3d(int x, int y, double z) : x(x), y(y), z(z) {}
    void print(){
        std::cout<<x<<" "<<y<<" "<<z<<std::endl;
    }
};

class vtxnormal{
public:
    double x, y, z;
    vtxnormal(){}
    vtxnormal(double x, double y, double z) : x(x), y(y), z(z) {}
};

class model{
public:
    std::vector<vert> verts; 
    std::vector<triplane> faces;
    std::vector<texture> textures;
    std::vector<vtxnormal> vnormals;

    model(const char *filename);
    int numVerts(){
        return verts.size();
    }
    int numFaces(){
        return faces.size();
    }
    int numTextures(){
        return textures.size();
    }
    int numVnormals(){
        return vnormals.size();
    }

};