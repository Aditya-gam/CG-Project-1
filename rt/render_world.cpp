#include "render_world.h"
#include "flat_shader.h"
#include "object.h"
#include "light.h"
#include "ray.h"

//#include <iostream>
//using namespace std;

extern bool disable_hierarchy;

Render_World::Render_World()
    :background_shader(0),ambient_intensity(0),enable_shadows(true),
    recursion_depth_limit(3)
{}

Render_World::~Render_World()
{
    delete background_shader;
    for(size_t i=0;i<objects.size();i++) delete objects[i];
    for(size_t i=0;i<lights.size();i++) delete lights[i];
}

// Find and return the Hit structure for the closest intersection.  Be careful
// to ensure that hit.dist>=small_t.
Hit Render_World::Closest_Intersection(const Ray& ray)
{
    TODO;
    return {};
}

// set up the initial view ray and call
void Render_World::Render_Pixel(const ivec2& pixel_index)
//
// This function sets up the initial view ray for the pixel at 'pixel_index'
// in screen coordinates, then computes the color by casting that ray into
// the scene. Finally, it sets the color in the camera's pixel buffer.
//
// Parameters:
//   pixel_index: the 2D pixel coordinate (in integer screen space) 
//
// Returns:
//   (void) - color is directly written to the camera's pixel buffer
{
    // Create a Ray object that will be sent through the scene.
    Ray ray;

    // The ray's endpoint is the camera's position in world space.
    ray.endpoint = camera.position;

    // Compute the direction by finding the world-space position corresponding to pixel_index, subtracting the camera position, and normalizing the result to get a unit direction vector.
    vec3 pixel_world_pos = camera.World_Position(pixel_index);
    ray.direction = (pixel_world_pos - ray.endpoint).normalized();

    // 4. Cast the ray into the scene, passing recursion depth = 1
    //    (often used for bounding the reflection/refraction recursion).
    vec3 color = Cast_Ray(ray, 1);

    // 5. Convert the computed color into the appropriate pixel format
    //    and set that pixel in the camera buffer.
    camera.Set_Pixel(pixel_index, Pixel_Color(color));
}


void Render_World::Render()
{
    for(int j=0;j<camera.number_pixels[1];j++)
        for(int i=0;i<camera.number_pixels[0];i++)
            Render_Pixel(ivec2(i,j));
}

// cast ray and return the color of the closest intersected surface point,
// or the background color if there is no object intersection
vec3 Render_World::Cast_Ray(const Ray& ray,int recursion_depth)
{
    vec3 color;
    Hit closest_hit = Closest_Intersection(ray);

    if(closest_hit.object) {
        vec3 intersection_point = ray.Point(closest_hit.dist);
        color = closest_hit.object->material_shader->Shade_Surface(ray, intersection_point, closest_hit.object->Normal(intersection_point, closest_hit.part), recursion_depth);
    }
    else {
        color = background_shader->Shade_Surface(ray, ray.direction, ray.direction, recursion_depth);
    }

    return color;
}
