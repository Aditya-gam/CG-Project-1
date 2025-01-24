#include "sphere.h"
#include "ray.h"
#include <cmath>
#include <limits>

Sphere::Sphere(const Parse* parse, std::istream& in)
{
    in >> name >> center >> radius;
}

Hit Sphere::Intersection(const Ray& ray, int part) const
{
    vec3 oc = ray.endpoint - center;
    double a = dot(ray.direction, ray.direction);
    double b = 2 * dot(ray.direction, oc);
    double c = dot(oc, oc) - radius * radius;

    double discriminant = b * b - 4 * a * c;

    Hit hit;

    if (discriminant >= 0)
    {
        double sqrt_discriminant = sqrt(discriminant);
        double t1 = (-b - sqrt_discriminant) / (2 * a);
        double t2 = (-b + sqrt_discriminant) / (2 * a);

        if (t1 >= small_t && t2 >= small_t)
        {
            hit.dist = std::min(t1, t2);
            hit.triangle = part; // Using part for compatibility with meshes
        }
        else if (t1 >= small_t)
        {
            hit.dist = t1;
            hit.triangle = part;
        }
        else if (t2 >= small_t)
        {
            hit.dist = t2;
            hit.triangle = part;
        }
    }

    return hit; // If no valid intersection, hit.dist will remain -1
}


vec3 Sphere::Normal(const Ray& ray, const Hit& hit) const
{
    vec3 intersection_point = ray.Point(hit.dist);
    return (intersection_point - center).normalized();
}

std::pair<Box, bool> Sphere::Bounding_Box(int part) const
{
    vec3 min_corner = center - vec3(radius, radius, radius);
    vec3 max_corner = center + vec3(radius, radius, radius);
    return {{min_corner, max_corner}, true};
}
