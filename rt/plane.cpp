#include "plane.h"
#include "hit.h"
#include "ray.h"
#include <limits>
#include <cmath>

Plane::Plane(const Parse* parse, std::istream& in)
{
    in >> name >> x >> normal;
    normal = normal.normalized();
}

// Intersect with the plane. The plane's normal points outside.
Hit Plane::Intersection(const Ray& ray, int part) const
{
    // Pixel_Print("Intersect test with ", name); // Debugging: Checking intersection
    // Debug_Ray("Ray", ray); // Print ray information

    Hit hit;
    hit.dist = -1; // Initialize to indicate no intersection
    hit.triangle = part; // For compatibility with meshes

    // Calculate the denominator of the intersection equation
    double denominator = dot(ray.direction, normal);

    // Check if the ray is not parallel to the plane
    if (std::abs(denominator) > small_t)
    {
        // Compute the distance t along the ray
        double t = dot(x - ray.endpoint, normal) / denominator;

        // Check if the intersection is valid
        if (t > small_t) // Ensure t is strictly greater than small_t to avoid self-intersection
        {
            hit.dist = t; // Set the intersection distance
            // Pixel_Print("Intersection found at distance: ", t);
        }
    }

    if (hit.dist < 0)
    {
        // Pixel_Print("No intersection with ", name);
    }

    return hit; // Return the hit, valid or not
}

vec3 Plane::Normal(const Ray& ray, const Hit& hit) const
{
    return normal; // The normal of the plane is constant
}

std::pair<Box, bool> Plane::Bounding_Box(int part) const
{
    Box b;
    b.Make_Full(); // Planes are infinite; they fill the entire space
    return {b, true};
}
