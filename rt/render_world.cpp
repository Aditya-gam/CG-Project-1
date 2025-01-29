#include "render_world.h"
#include "flat_shader.h"
#include "object.h"
#include "light.h"
#include "ray.h"

extern bool enable_acceleration;

Render_World::~Render_World()
{
    for (auto a : all_objects) delete a;
    for (auto a : all_shaders) delete a;
    for (auto a : all_colors) delete a;
    for (auto a : lights) delete a;
}

// Find and return the Hit structure for the closest intersection. Ensure that hit.dist >= small_t.
std::pair<Shaded_Object, Hit> Render_World::Closest_Intersection(const Ray& ray) const
{
    // Pixel_Print("Finding closest intersection for ray.");
    // Debug_Ray("Ray", ray);

    Hit closest_hit;
    // Initialize distance to infinity so that any valid intersection replaces it
    closest_hit.dist = std::numeric_limits<double>::infinity(); 
    Shaded_Object closest_object;

    // Iterate through all objects to find the closest intersection
    for (const auto& obj : objects)
    {
        Hit hit = obj.object->Intersection(ray, -1); // Intersect with all parts (part=-1)
        if (hit.Valid() && hit.dist < closest_hit.dist && hit.dist >= small_t)
        {
            closest_hit = hit;
            closest_object = obj;
            // Pixel_Print("Updated closest hit: ", "Object: ", obj.object->name, 
                        // ", Distance: ", closest_hit.dist);
        }
    }

    // Now decide whether we found an intersection
    if (closest_object.object)
    {
        // Here is where we could include extra debugging logic for meshes:
        // For example, computing UV coordinates if the object has a mesh structure.
        //
        // The following snippet shows an example of how you might retrieve
        // per-vertex UVs and combine them using barycentric coordinates:
        //
        // ---------------------------------------------------------------
        // double uv_x = 0.0, uv_y = 0.0;
        // if (!closest_object.object->uvs.empty() && closest_hit.triangle >= 0)
        // {
        //     // Suppose 'triangles' is a vector of ivec3, each holding 3 vertex indices
        //     ivec3 e = closest_object.object->triangles[closest_hit.triangle];
        //     // Suppose 'uvs' is a vector of vec2 storing UV coords for each vertex
        //     vec2 uvA = closest_object.object->uvs[e[0]];
        //     vec2 uvB = closest_object.object->uvs[e[1]];
        //     vec2 uvC = closest_object.object->uvs[e[2]];
        
        //     // Suppose 'alpha', 'beta', 'gamma' are barycentric coordinates set by Intersection
        //     uv_x = closest_hit.alpha * uvA[0] 
        //          + closest_hit.beta  * uvB[0] 
        //          + closest_hit.gamma * uvC[0];
        //     uv_y = closest_hit.alpha * uvA[1] 
        //          + closest_hit.beta  * uvB[1] 
        //          + closest_hit.gamma * uvC[1];
        // }
        
        // Pixel_Print("intersect test with M; hit: (dist: ", closest_hit.dist,
        //             "; triangle: ", closest_hit.triangle, 
        //             "; uv: (", uv_x, " ", uv_y, "))");
        // ---------------------------------------------------------------
        
        // For now, we just print a simpler message:
        // Pixel_Print("Found intersection with object: ", closest_object.object->name,
                    // "; dist: ", closest_hit.dist);
    }
    else
    {
        // No intersection found
        // Pixel_Print("No intersection found.");
    }

    return {closest_object, closest_hit};
}


// Set up the initial view ray and call Cast_Ray
void Render_World::Render_Pixel(const ivec2& pixel_index)
{
    // Pixel_Print("Rendering pixel: (", pixel_index[0], " ", pixel_index[1], ")");

    Ray ray;
    ray.endpoint = camera.position; // Camera position as the ray origin
    ray.direction = (camera.World_Position(pixel_index) - camera.position).normalized(); // Direction toward the pixel

    vec3 color = Cast_Ray(ray, 1); // Cast ray with recursion depth = 1
    camera.Set_Pixel(pixel_index, Pixel_Color(color)); // Set the pixel color
    // Pixel_Print("Pixel color: ", Vec_To_String(color));
}

void Render_World::Render()
{
    for (int j = 0; j < camera.number_pixels[1]; j++)
    {
        for (int i = 0; i < camera.number_pixels[0]; i++)
        {
            Render_Pixel(ivec2(i, j)); // Render each pixel
        }
    }
}

// Cast ray and return the color of the closest intersected surface point,
// or the background color if there is no object intersection
vec3 Render_World::Cast_Ray(const Ray& ray, int recursion_depth) const
{
    // Pixel_Print("Casting ray at recursion depth: ", recursion_depth);
    // Debug_Ray("Ray", ray);

    if (recursion_depth > recursion_depth_limit)
    {
        // Pixel_Print("Recursion depth limit exceeded. Returning black.");
        return vec3(0, 0, 0); // Return black if recursion depth exceeds the limit
    }

    auto [closest_object, closest_hit] = Closest_Intersection(ray);

    if (closest_object.object)
    {
        // Calculate the intersection point and normal
        vec3 intersection_point = ray.Point(closest_hit.dist);
        vec3 normal = closest_object.object->Normal(ray, closest_hit);

        // Pixel_Print("Intersection found at: ", Vec_To_String(intersection_point));
        // Pixel_Print("Normal at intersection: ", Vec_To_String(normal));

        // Shade the surface using the object's shader
        vec3 shaded_color = closest_object.shader->Shade_Surface(*this, ray, closest_hit, intersection_point, normal, recursion_depth);
        // Pixel_Print("Final shaded color: ", Vec_To_String(shaded_color));

        return shaded_color;
    }
    else if (background_shader)
    {
        // Pixel_Print("No intersection. Using background shader.");
        return background_shader->Shade_Surface(*this, ray, {}, {}, {}, recursion_depth);
    }

    // Pixel_Print("No intersection and no background shader. Returning black.");
    return vec3(0, 0, 0); // Return black if no object and no background shader
}
