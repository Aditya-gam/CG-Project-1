#include "sphere.h"
#include "ray.h"
#include <cmath>
#include <limits>

/**
 * Constructor for Sphere. Parses its name, center (vec3), and radius (double)
 * from the input stream.
 *
 * Example input line for a sphere:
 *     sphere SphereName 0.1 0.1 0.3 2.5
 * which sets the sphere's center to (0.1,0.1,0.3) and radius to 2.5.
 */
Sphere::Sphere(const Parse* parse, std::istream& in)
{
    in >> name >> center >> radius;
}

/**
 * Computes the intersection between this sphere and the given ray.
 * Returns a Hit with dist < 0 if no intersection occurs in [small_t, +∞).
 * Otherwise, dist is set to the nearest valid intersection along the ray.
 *
 * The formula for a sphere of center C and radius R:
 *    Let oc = ray.endpoint - C
 *    a = dot(ray.direction, ray.direction)
 *    b = 2 * dot(ray.direction, oc)
 *    c = dot(oc, oc) - R^2
 *
 * Then we solve the quadratic:
 *    a*t^2 + b*t + c = 0
 * for t >= small_t.
 */
Hit Sphere::Intersection(const Ray& ray, int part) const
{
    Hit hit;
    hit.dist = -1; // Default to "no intersection"
    hit.triangle = part; // Used for mesh indexing, but here just store part

    vec3 oc = ray.endpoint - center;
    double a = dot(ray.direction, ray.direction);
    double b = 2.0 * dot(ray.direction, oc);
    double c = dot(oc, oc) - radius * radius;

    double discriminant = b * b - 4.0 * a * c;
    if (discriminant >= 0.0)
    {
        double sqrt_discriminant = std::sqrt(discriminant);
        double t1 = (-b - sqrt_discriminant) / (2.0 * a);
        double t2 = (-b + sqrt_discriminant) / (2.0 * a);

        // We want the smallest positive t >= small_t
        bool valid_t1 = (t1 >= small_t);
        bool valid_t2 = (t2 >= small_t);

        if (valid_t1 && valid_t2)
        {
            // Both intersections are valid; pick the nearer one
            hit.dist = std::min(t1, t2);
        }
        else if (valid_t1)
        {
            hit.dist = t1;
        }
        else if (valid_t2)
        {
            hit.dist = t2;
        }
    }

    // If neither t1 nor t2 was valid, hit.dist remains -1 (no intersection)
    return hit;
}

/**
 * Computes the surface normal of the sphere at the intersection given by hit.
 * Intersection point P = ray.endpoint + ray.direction * hit.dist
 * Normal N = (P - center).normalized()
 */
vec3 Sphere::Normal(const Ray& ray, const Hit& hit) const
{
    vec3 intersection_point = ray.Point(hit.dist);
    return (intersection_point - center).normalized();
}

/**
 * Returns a bounding box that tightly encloses this sphere.
 * Because the sphere is finite, is_infinite = false.
 */
std::pair<Box,bool> Sphere::Bounding_Box(int part) const
{
    // Spheres are finite. Compute min/max corners.
    vec3 min_corner = center - vec3(radius, radius, radius);
    vec3 max_corner = center + vec3(radius, radius, radius);

    // Mark is_infinite=false for finite shapes (this is the critical fix!)
    return { { min_corner, max_corner }, false };
}
