#include "tgaimage.h"
#include "model.h"
#include <iostream>

const TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
const TGAColor green   = {  0, 255,   0, 255};
const TGAColor red     = {  0,   0, 255, 255};
const TGAColor blue    = {255, 128,  64, 255};
const TGAColor yellow  = {  0, 200, 255, 255};

TGAImage textureImage;


constexpr int width  = 500;
constexpr int height = 500;


void line(TGAImage &framebuffer,int ax, int ay, int bx, int by,  TGAColor color) {
    bool steep = std::abs(ax-bx) < std::abs(ay-by);
    if (steep) { // if the line is steep, we transpose the image
        std::swap(ax, ay);
        std::swap(bx, by);
    }
    if (ax>bx) { // make it left−to−right
        std::swap(ax, bx);
        std::swap(ay, by);
    }
    int y = ay;
    int ierror = 0;
    for (int x=ax; x<=bx; x++) {
        if (steep) // if transposed, de−transpose
            framebuffer.set(y, x, color);
        else
            framebuffer.set(x, y, color);
        ierror += 2 * std::abs(by-ay);
        if (ierror > bx - ax) {
            y += by > ay ? 1 : -1;
            ierror -= 2 * (bx-ax);
        }
    }
}
double signed_triangle_area(const vec3d &a, const vec3d &b, const vec3d &c) {
    return .5*((b.y-a.y)*(b.x+a.x) + (c.y-b.y)*(c.x+b.x) + (a.y-c.y)*(a.x+c.x));
}

std::vector<double> zbuffer(width*height, std::numeric_limits<double>::min());

void triangle(const vec3d &a, const vec3d &b, const vec3d &c,texture* texplane ,TGAImage &framebuffer, TGAColor color) {
    int bbminx = std::min(std::min(a.x, b.x), c.x); // bounding box for the triangle
    int bbminy = std::min(std::min(a.y, b.y), c.y); // defined by its top left and bottom right corners
    int bbmaxx = std::max(std::max(a.x, b.x), c.x);
    int bbmaxy = std::max(std::max(a.y, b.y), c.y);
    double total_area = signed_triangle_area(a,b,c);
	if(total_area<1)
		return;

#pragma omp parallel for
    for (int x=bbminx; x<=bbmaxx; x++) {
        for (int y=bbminy; y<=bbmaxy; y++) {
			vec3d p(x,y,0.);
            double alpha = signed_triangle_area(p,b,c) / total_area;
            double beta  = signed_triangle_area(p,c,a) / total_area;
            double gamma = signed_triangle_area(p,a,b) / total_area;
            if (alpha<0 || beta<0 || gamma<0) continue; // negative barycentric coordinate => the pixel is outside the triangle
			//framebuffer.set(x, y, color);

			p.z = alpha*a.z + beta*b.z + gamma*c.z;
            
            texture tex = {alpha*texplane[0].u + beta*texplane[1].u + gamma*texplane[2].u, alpha*texplane[0].v + beta*texplane[1].v + gamma*texplane[2].v};
           
            TGAColor curColr=textureImage.get(tex.u*textureImage.get_width(), tex.v*textureImage.get_height());
			int idx = x + y*width;
			if(zbuffer[idx]<p.z){
				zbuffer[idx] = p.z;	
            	framebuffer.set(x, y, curColr);
			}
    	}
	}
}

vec3d project(const vert &v){
	return vec3d((v.x+1.)*width/2., (v.y+1.)*height/2.,v.z);
}

int main(int argc, char** argv) {
	TGAImage image(width, height, TGAImage::RGB);
    textureImage.read_tga_file("./texture.tga");
    textureImage.flip_vertically();
	model m("./my.obj");

// light
const vert light={0.,0.,-1};

	for(auto & face: m.faces){
		auto [index1, index2, index3] = face.vertexIndex;
        auto [texIndex1, texIndex2, texIndex3] = face.textureIndex;
		vert beforeProject[3] = {m.verts[index1], m.verts[index2], m.verts[index3]};
        texture texplane[3]={m.textures[texIndex1], m.textures[texIndex2], m.textures[texIndex3]};
        
		vec3d a = project(beforeProject[0]);
		vec3d b = project(beforeProject[1]);
		vec3d c = project(beforeProject[2]);
		vert n = (beforeProject[2]-beforeProject[0])^(beforeProject[1]-beforeProject[0]);
		n=n.normalize();
		double intensity = n*light;
		TGAColor color = TGAColor(255*intensity, 255*intensity, 255*intensity, 255);
		if(intensity>0){
        	triangle(a,b,c,texplane,image,color);
		}
	}

	image.flip_vertically(); 
	image.write_tga_file("output.tga");
	return 0;
}

