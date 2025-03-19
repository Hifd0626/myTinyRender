#include "tgaimage.h"
#include "model.h"
#include <iostream>
#include "Matrix.h"
#include <algorithm>
const TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
const TGAColor green   = {  0, 255,   0, 255};
const TGAColor red     = {  0,   0, 255, 255};
const TGAColor blue    = {255, 128,  64, 255};
const TGAColor yellow  = {  0, 200, 255, 255};

TGAImage textureImage;
constexpr int width  = 800;
constexpr int height = 800;
constexpr int depth  = 255;
const vert light={0.,0.,-1.};
const vert camera={0,0,3};

Matrix viewPort(int x,int y,int w,int h){
    Matrix m(4,4);
    m.identity();
    m[0][3] = x+w/2.f;
    m[1][3] = y+h/2.f;
    m[2][3] = depth/2.f;
    m[0][0] = w/2.f;
    m[1][1] = h/2.f;
    m[2][2] = depth/2.f;
    return m;

}
Matrix rotationY(float angle) {
    Matrix rot(4, 4);
    rot.identity();
    rot[0][0] = cos(angle);
    rot[0][2] = sin(angle);
    rot[2][0] = -sin(angle);
    rot[2][2] = cos(angle);
    return rot;
}
Matrix vert2Matrix(const vert &v){
    Matrix m(4,1);
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
    m[3][0] = 1.f;
    return m;
}
vert matrix2vert(Matrix  m){
    return vert(m[0][0]/m[3][0], m[1][0]/m[3][0], m[2][0]/m[3][0]);
}
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


void triangle(const vec3d &a, const vec3d &b, const vec3d &c,texture* texplane, vtxnormal * normplane,TGAImage &framebuffer, double intensity,TGAColor& color) {
    int bbminx = std::min(std::min(a.x, b.x), c.x); // bounding box for the triangle
    int bbminy = std::min(std::min(a.y, b.y), c.y); // defined by its top left and bottom right corners
    int bbmaxx = std::max(std::max(a.x, b.x), c.x);
    int bbmaxy = std::max(std::max(a.y, b.y), c.y);
    double total_area = signed_triangle_area(a,b,c);
	if(total_area<1)
		return;
    vert vtxnrm0=vert(normplane[0].x,normplane[0].y,normplane[0].z);
    vert vtxnrm1=vert(normplane[1].x,normplane[1].y,normplane[1].z);
    vert vtxnrm2=vert(normplane[2].x,normplane[2].y,normplane[2].z);
    vtxnrm0.normalize();
    vtxnrm1.normalize();
    vtxnrm2.normalize();
 
#pragma omp parallel for

    for (int x=bbminx; x<=bbmaxx; x++) {
        for (int y=bbminy; y<=bbmaxy; y++) {
			vec3d p(x,y,0.);
            double alpha = signed_triangle_area(p,b,c) / total_area;
            double beta  = signed_triangle_area(p,c,a) / total_area;
            double gamma = signed_triangle_area(p,a,b) / total_area;
            if (alpha<0 || beta<0 || gamma<0) 
                continue; 
			
			p.z = alpha*a.z + beta*b.z + gamma*c.z;
            
            
            vert interpolated_normal = vtxnrm0*alpha + vtxnrm1*beta + vtxnrm2*gamma;
            interpolated_normal.normalize();  // 归一化
            intensity = -(interpolated_normal*light);
            
            intensity = std::max(0.,intensity);
            intensity = std::min(1.,intensity);
            
            if(intensity<0.)   
                continue;

            texture tex = {alpha*texplane[0].u + beta*texplane[1].u + gamma*texplane[2].u, alpha*texplane[0].v + beta*texplane[1].v + gamma*texplane[2].v};
            TGAColor curColr=textureImage.get(tex.u*textureImage.get_width(), tex.v*textureImage.get_height());
            for(int i=0;i<3;++i){
                curColr.raw[i] = abs(curColr.raw[i]*intensity);
            }
            // for(int i=0;i<3;++i){
            //     color.raw[i] = color.raw[i]*intensity;
            // }
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

vec3d vert2v3d(const vert &v){
    return vec3d(v.x,v.y,v.z);
}
TGAImage txt[2];

int main(int argc, char** argv) {
	TGAImage image(width, height, TGAImage::RGB);
    // textureImage.read_tga_file("./obj/african_head/african_head_eye_inner_diffuse.tga");
    textureImage.flip_vertically();
	//model m("./obj/african_head/african_head_eye_inner.obj");
    model m1=model("./obj/african_head/african_head.obj");
    model m2=model("./obj/african_head/african_head_eye_inner.obj");
    txt[0].read_tga_file("./obj/african_head/african_head_diffuse.tga");
    txt[1].read_tga_file("./obj/african_head/african_head_eye_inner_diffuse.tga");
    model m3=model("./obj/Nezha/Nezha.obj");   
    //txt[0].read_tga_file("./obj/Nezha/Nezha.tga"); 
    model mm[]={m1,m2};
    Matrix projection=Matrix(4,4);
    projection.identity();
    projection[3][2] = -1.f/(camera.z);
    Matrix view = viewPort(width/8, height/8, width*3/4, height*3/4);
    Matrix rotation = rotationY(0.5);
    Matrix modelView = view*projection*rotation;
    // modelView.printMatrix();
    light.normalize();
    int cnt=0;
    for(auto &m:mm){
        textureImage=txt[cnt++];
        textureImage.flip_vertically();

        for(auto & face: m.faces){
            auto [index1, index2, index3] = face.vertexIndex;
            auto [texIndex1, texIndex2, texIndex3] = face.textureIndex;
            auto [normalIndex1, normalIndex2, normalIndex3] = face.normalIndex;
            vert beforeProject[3] = {m.verts[index1], m.verts[index2], m.verts[index3]};
            texture texplane[3]={m.textures[texIndex1], m.textures[texIndex2], m.textures[texIndex3]};    
            vtxnormal  normalplane[3]={m.vnormals[normalIndex1], m.vnormals[normalIndex2], m.vnormals[normalIndex3]};

            vert a1 = matrix2vert(modelView*vert2Matrix(beforeProject[0]));
            vert b1 = matrix2vert(modelView*vert2Matrix(beforeProject[1]));
            vert c1 = matrix2vert(modelView*vert2Matrix(beforeProject[2]));
            vec3d a= vert2v3d(a1);
            vec3d b= vert2v3d(b1);
            vec3d c= vert2v3d(c1);

            vert n = (beforeProject[2]-beforeProject[0])^(beforeProject[1]-beforeProject[0]);
            n=n.normalize();
            double intensity = n*light;
            TGAColor color = TGAColor(255, 255, 255, 255);
            if(intensity>0){
                triangle(a,b,c,texplane,normalplane,image,intensity,color);
            }
        }
    }
	image.flip_vertically(); 
	image.write_tga_file("output.tga");
	return 0;
}

