/*_____________________________________________
|
|   CS 535 Programming Assignment 3
|
|       Written by Mark Noblin
|
|           Fall 2017         
|
|______________________________________________*/


#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>

using namespace std;


#define MAX_RAY_DISTANCE 5
struct Vec3{

    float x;
    float y;
    float z;

    Vec3() : x(0.0), y(0.0), z(0.0) {}
    Vec3(float xi, float yi, float zi) : x(xi), y(yi), z(zi) {}

    Vec3 operator + (const Vec3& v) const { return Vec3(x+v.x, y+v.y, z+v.z); }
    Vec3 operator - (const Vec3& v) const { return Vec3(x-v.x, y-v.y, z-v.z); }
    Vec3 operator * (const Vec3& v) const { return Vec3(x * v.x, y * v.y, z * v.z); }
    Vec3 operator * (float d) const { return Vec3(x*d, y*d, z*d); }
    Vec3 operator / (float d) const { return Vec3(x/d, y/d, z/d); }
    Vec3& normalize() {
        float nor2 = x*x + y*y + z*z;
        if(nor2 > 0){
            float invNor = 1 / sqrt(nor2);
            x *= invNor, y *= invNor, z *= invNor;
        }
        return *this;
    }
};

inline float dot(const Vec3& a, const Vec3& b) {
  return (a.x*b.x + a.y*b.y + a.z*b.z);
}

struct Ray{

    Vec3 orig;
    Vec3 dir;

    Ray(const Vec3& o, const Vec3& d) : orig(o), dir(d) {}

};

struct Sphere{

    Vec3 center;
    float radius;
    Vec3 color;
    float transparency;
    float reflection;
    Vec3 emission;

    Sphere(const Vec3& c, float r, const Vec3& col, float trans, float ref, const Vec3& emiss) : center(c), radius(r), color(col), transparency(trans), reflection(ref), emission(emiss) {}

    bool Intersect(const Ray& ray, float &t0, float &t1) const {
        Vec3 l = center - ray.orig;
        float tca = dot(l, ray.dir);
        if(tca < 0) return false;
        float d2 = dot(l, l) - tca * tca;
        if(d2 > (radius*radius)) return false;
        float thc = sqrt((radius*radius) - d2);
        t0 = tca - thc;
        t1 = tca + thc;

        return true;
    }

};

float mix( const float &a, const float &b, const float &mix)
{
    return b * mix + a * (1 - mix);
}

const Vec3 RayTrace(Ray& ray, const vector<Sphere> &spheres, const int &depth)
{

    float tnear = 100000000.0;
    const Sphere* sphere = NULL;

    for( unsigned i = 0; i < spheres.size(); i++)
    {
        float t0 = 100000000.0, t1 = 100000000.0;
        if(spheres[i].Intersect( ray, t0, t1 ))
        {
            if (t0 < 0) t0 = t1;
            if(t0 < tnear)
            {
                tnear = t0;
                sphere = &spheres[i];
            }
        }
    }

    if(!sphere) return Vec3(2.0, 2.0, 2.0);

    Vec3 surfaceColor = Vec3(0.0, 0.0, 0.0);
    Vec3 intPoint = ray.orig + ray.dir * tnear;
    Vec3 normHit = intPoint - sphere->center;
    normHit.normalize();


    float bias = 1e-4;
    bool inside = false;

    if(dot(ray.dir, normHit) > 0){ normHit = (normHit * -1.0f); inside = true; }
        
    if((sphere->transparency > 0 || sphere->reflection > 0) && depth < MAX_RAY_DISTANCE)
    {
        float facingRatio = dot(ray.dir*-1.0f, normHit);
        float fresnel = mix( pow( 1 - facingRatio, 3), 1, 0.1);

        Vec3 reflectDir = ray.dir - normHit * 2 * dot(ray.dir, normHit);
        reflectDir.normalize();
        Ray reflectRay = Ray(intPoint + normHit * bias, reflectDir);
        Vec3 reflection = RayTrace(reflectRay, spheres, depth + 1);
        Vec3 refraction = Vec3(0.0, 0.0, 0.0);

        if(sphere->transparency)
        {
            float ior = 1.1, eta = (inside) ? ior : 1/ior;

            float cosi = -1 * dot(normHit, ray.dir);
            float k = 1 - eta*eta * (1 - cosi*cosi);
            Vec3 refractDir = ray.dir * eta + normHit * (eta * cosi - sqrt(k));
            refractDir.normalize();
            Ray refractRay = Ray(intPoint - normHit * bias, refractDir);
            refraction = RayTrace(refractRay, spheres, depth + 1);

        }

        surfaceColor = (reflection * fresnel + refraction * (1-fresnel) * sphere->transparency) * sphere->color ;

    } else {
       
        for( unsigned i = 0; i < spheres.size(); i++) 
        {
            if(spheres[i].emission.x > 0) 
            {
                Vec3 transmission = Vec3(1.0f, 1.0f, 1.0f);
                Vec3 lightDirection = spheres[i].center - intPoint;
                lightDirection.normalize();

                for(unsigned j = 0; j < spheres.size(); j++)
                {
                    if( i != j )
                    {
                        float t0, t1;
                        Ray lightRay = Ray(intPoint + normHit * bias, lightDirection);
                        if(spheres[j].Intersect(lightRay, t0, t1)) 
                        {
                            transmission = Vec3(0.0f, 0.0f, 0.0f);
                            break;
                        }
                    }
                }
                surfaceColor = surfaceColor + (sphere->color * transmission * std::max(float(0), dot(normHit, lightDirection)) * spheres[i].emission);

            }
        }

    }

    return surfaceColor + sphere->emission;
}

void render(const vector<Sphere> &spheres)
{
    unsigned width = 640, height = 480;
    Vec3 *image = new Vec3[width * height]; 
    Vec3 *pixel = image;
    float invWidth = 1 / float(width), invHeight = 1/ float(height);
    float fov = 30, aspect = width/float(height);
    float angle = tan(3.14159 * 0.5 * fov / 180.);

    for( unsigned y = 0; y < height; y++) {
        for(unsigned x = 0; x < width; x++, pixel++) {
            float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspect;
            float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
            Vec3 raydir(xx, yy, -1);
            raydir.normalize();
            Ray ray = Ray(Vec3(0.0, 0.0, 10.0), raydir);
            *pixel = RayTrace( ray, spheres, 0);
        }
    }

    std::ofstream ofs("./535RayTrace.ppm");
    ofs << "P6\n" << width << " " << height << "\n255\n";
    for(unsigned i = 0; i < width * height; i++){
        ofs << (unsigned char)(std::min(float(1), image[i].x) * 255) <<
               (unsigned char)(std::min(float(1), image[i].y) * 255) <<
               (unsigned char)(std::min(float(1), image[i].z) * 255);
    }
    ofs.close();
    delete [] image;
}

int main(int argc, char **argv)
{

    srand48(13);
    vector<Sphere> spheres;
    spheres.push_back(Sphere(Vec3(0.0, -10004, -20), 10000, Vec3(0.20, 0.20, 0.20), 1, 0.0, Vec3()));
    spheres.push_back(Sphere(Vec3(0.0, 0, -20), 4, Vec3(1.00, 0.32, 0.36), 1, 0.5, Vec3()));
    spheres.push_back(Sphere(Vec3(-5.0, 3, -30), 2, Vec3(0.90, 0.76, 0.46), 1, 0.0, Vec3()));
    spheres.push_back(Sphere(Vec3(5.0, 0, -25), 3, Vec3(0.65, 0.77, 0.97), 1, 0.0, Vec3()));
    spheres.push_back(Sphere(Vec3(-5.0, 0, -25), 3, Vec3(0.50, 0.50, 0.50), 1, 0.5, Vec3()));
    // light
    spheres.push_back(Sphere(Vec3( 3.0,     20, -30),     3, Vec3(1.00, 1.00, 1.00), 0, 0.0, Vec3(0.25, 0.25, 0.25)));
    spheres.push_back(Sphere(Vec3(5.0, 20, -10), 2, Vec3(1.00, 1.00, 1.00), 0, 0.0, Vec3(0.5, 0.5, 0.5)));

    render(spheres);

    return 0;
}

