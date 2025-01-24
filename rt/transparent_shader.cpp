#include "transparent_shader.h"
#include "parse.h"
#include "ray.h"
#include "render_world.h"
#include <cmath>
#include <cassert>

Transparent_Shader::
Transparent_Shader(const Parse* parse, std::istream& in)
{
    in >> name >> index_of_refraction >> opacity;
    shader = parse->Get_Shader(in);
    assert(index_of_refraction >= 1.0);
}

vec3 Transparent_Shader::
Shade_Surface(const Render_World& render_world, const Ray& ray, const Hit& hit,
    const vec3& intersection_point, const vec3& normal, int recursion_depth) const
{
    if (recursion_depth > render_world.recursion_depth_limit)
        return vec3(0, 0, 0); // Terminate recursion if depth exceeds limit

    // Compute the base color using the underlying shader
    vec3 base_color = shader->Shade_Surface(render_world, ray, hit, intersection_point, normal, recursion_depth);

    // Determine if the ray is entering or leaving the object
    double n1 = 1.0; // Refractive index of air
    double n2 = index_of_refraction; // Refractive index of the object
    vec3 adjusted_normal = normal;

    if (dot(ray.direction, normal) > 0) // Leaving the object
    {
        std::swap(n1, n2);
        adjusted_normal = -normal;
    }

    // Compute the refraction direction using Snell's law
    double n_ratio = n1 / n2;
    double cos_theta_i = -dot(adjusted_normal, ray.direction);
    double sin2_theta_t = n_ratio * n_ratio * (1.0 - cos_theta_i * cos_theta_i);

    vec3 refracted_direction;
    bool total_internal_reflection = false;

    if (sin2_theta_t > 1.0) // Total internal reflection
    {
        total_internal_reflection = true;
    }
    else
    {
        double cos_theta_t = std::sqrt(1.0 - sin2_theta_t);
        refracted_direction = n_ratio * ray.direction + (n_ratio * cos_theta_i - cos_theta_t) * adjusted_normal;
    }

    // Compute the reflection direction
    vec3 reflected_direction = ray.direction - 2 * dot(ray.direction, adjusted_normal) * adjusted_normal;

    // Schlick approximation for reflectivity
    double r0 = pow((n1 - n2) / (n1 + n2), 2);
    double reflectivity = r0 + (1 - r0) * pow(1 - std::abs(cos_theta_i), 5);

    // Cast reflection ray
    Ray reflected_ray(intersection_point + adjusted_normal * 1e-6, reflected_direction);
    vec3 reflected_color = render_world.Cast_Ray(reflected_ray, recursion_depth + 1);

    // Cast refraction ray if no total internal reflection
    vec3 refracted_color(0, 0, 0);
    if (!total_internal_reflection)
    {
        Ray refracted_ray(intersection_point - adjusted_normal * 1e-6, refracted_direction);
        refracted_color = render_world.Cast_Ray(refracted_ray, recursion_depth + 1);
    }

    // Combine results using opacity and reflectivity
    vec3 color = (1 - opacity) * base_color +
                 opacity * (reflectivity * reflected_color + (1 - reflectivity) * refracted_color);

    return color;
}
