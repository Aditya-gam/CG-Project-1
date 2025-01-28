#include "transparent_shader.h"
#include "parse.h"
#include "ray.h"
#include "render_world.h"
#include <cmath>
#include <cassert>

Transparent_Shader::Transparent_Shader(const Parse* parse, std::istream& in)
{
    in >> name >> index_of_refraction >> opacity;
    shader = parse->Get_Shader(in);
    assert(index_of_refraction >= 1.0);
}

// vec3 Transparent_Shader::Shade_Surface(const Render_World& render_world, const Ray& ray, const Hit& hit,
//                                        const vec3& intersection_point, const vec3& normal, int recursion_depth) const
// {
//     Pixel_Print("Shading surface with Transparent_Shader");
//     Pixel_Print("Recursion depth: ", recursion_depth);

//     if (recursion_depth > render_world.recursion_depth_limit)
//     {
//         Pixel_Print("Recursion depth exceeded. Returning black color.");
//         return vec3(0, 0, 0); // Terminate recursion if depth exceeds limit
//     }

//     // Compute the base color using the underlying shader
//     vec3 base_color = shader->Shade_Surface(render_world, ray, hit, intersection_point, normal, recursion_depth);
//     Pixel_Print("Base color: ", Vec_To_String(base_color));

//     // Determine if the ray is entering or leaving the object
//     double n1 = 1.0;               // Refractive index of air
//     double n2 = index_of_refraction; // Refractive index of the object
//     vec3 adjusted_normal = normal;

//     if (dot(ray.direction, normal) > 0) // Leaving the object
//     {
//         Pixel_Print("Ray is leaving the object.");
//         std::swap(n1, n2);
//         adjusted_normal = -normal;
//     }
//     else
//     {
//         Pixel_Print("Ray is entering the object.");
//     }

//     Pixel_Print("n1 (outer): ", n1, ", n2 (inner): ", n2);
//     Pixel_Print("Adjusted normal: ", Vec_To_String(adjusted_normal));

//     // Compute the refraction direction using Snell's law
//     double n_ratio = n1 / n2;
//     double cos_theta_i = -dot(adjusted_normal, ray.direction);
//     double sin2_theta_t = n_ratio * n_ratio * (1.0 - cos_theta_i * cos_theta_i);

//     Pixel_Print("n_ratio: ", n_ratio);
//     Pixel_Print("cos_theta_i: ", cos_theta_i);
//     Pixel_Print("sin^2(theta_t): ", sin2_theta_t);

//     vec3 refracted_direction;
//     bool total_internal_reflection = false;

//     if (sin2_theta_t > 1.0) // Total internal reflection
//     {
//         total_internal_reflection = true;
//         Pixel_Print("Total internal reflection occurred.");
//     }
//     else
//     {
//         double cos_theta_t = std::sqrt(1.0 - sin2_theta_t);
//         refracted_direction = n_ratio * ray.direction + (n_ratio * cos_theta_i - cos_theta_t) * adjusted_normal;
//         Pixel_Print("Refracted direction: ", Vec_To_String(refracted_direction));
//     }

//     // Compute the reflection direction
//     vec3 reflected_direction = ray.direction - 2 * dot(ray.direction, adjusted_normal) * adjusted_normal;
//     Pixel_Print("Reflected direction: ", Vec_To_String(reflected_direction));

//     // Schlick approximation for reflectivity
//     double r0 = pow((n1 - n2) / (n1 + n2), 2);
//     double reflectivity = r0 + (1 - r0) * pow(1 - std::abs(cos_theta_i), 5);
//     Pixel_Print("Reflectivity (Schlick approximation): ", reflectivity);

//     // Cast reflection ray
//     Ray reflected_ray(intersection_point + adjusted_normal * 1e-6, reflected_direction);
//     Pixel_Print("Casting reflection ray.");
//     Debug_Ray("Reflection ray", reflected_ray);
//     vec3 reflected_color = render_world.Cast_Ray(reflected_ray, recursion_depth + 1);
//     Pixel_Print("Reflected color: ", Vec_To_String(reflected_color));

//     // Cast refraction ray if no total internal reflection
//     vec3 refracted_color(0, 0, 0);
//     if (!total_internal_reflection)
//     {
//         Ray refracted_ray(intersection_point - adjusted_normal * 1e-6, refracted_direction);
//         Pixel_Print("Casting refraction ray.");
//         Debug_Ray("Refraction ray", refracted_ray);
//         refracted_color = render_world.Cast_Ray(refracted_ray, recursion_depth + 1);
//         Pixel_Print("Refracted color: ", Vec_To_String(refracted_color));
//     }

//     // Combine results using opacity and reflectivity
//     vec3 combined_color = reflectivity * reflected_color + (1 - reflectivity) * refracted_color;
//     vec3 final_color = (1 - opacity) * base_color + opacity * combined_color;

//     Pixel_Print("Combined color (reflectivity + refraction): ", Vec_To_String(combined_color));
//     Pixel_Print("Final color (opacity blend): ", Vec_To_String(final_color));

//     return final_color;
// }


vec3 Transparent_Shader::Shade_Surface(const Render_World& render_world, const Ray& ray, const Hit& hit,
                                       const vec3& intersection_point, const vec3& normal, int recursion_depth) const
{
    Pixel_Print("Shading surface with Transparent_Shader");
    Pixel_Print("Recursion depth: ", recursion_depth);

    if (recursion_depth > render_world.recursion_depth_limit)
    {
        Pixel_Print("Recursion depth exceeded. Returning black color.");
        return vec3(0, 0, 0); // Terminate recursion if depth exceeds limit
    }

    // Compute the base color using the underlying shader
    vec3 base_color = shader->Shade_Surface(render_world, ray, hit, intersection_point, normal, recursion_depth);
    Pixel_Print("Base color: ", Vec_To_String(base_color));

    // Determine if the ray is entering or leaving the object
    double n1 = 1.0;               // Refractive index of air
    double n2 = index_of_refraction; // Refractive index of the object
    vec3 adjusted_normal = normal;

    if (dot(ray.direction, normal) > 0) // Leaving the object
    {
        Pixel_Print("Ray is leaving the object.");
        std::swap(n1, n2);
        adjusted_normal = -normal;
    }
    else
    {
        Pixel_Print("Ray is entering the object.");
    }

    Pixel_Print("n1 (outer): ", n1, ", n2 (inner): ", n2);
    Pixel_Print("Adjusted normal: ", Vec_To_String(adjusted_normal));

    // Compute the refraction direction using Snell's law
    double n_ratio = n1 / n2;
    double cos_theta_i = -dot(adjusted_normal, ray.direction);
    double sin2_theta_t = n_ratio * n_ratio * (1.0 - cos_theta_i * cos_theta_i);

    Pixel_Print("n_ratio: ", n_ratio);
    Pixel_Print("cos_theta_i: ", cos_theta_i);
    Pixel_Print("sin^2(theta_t): ", sin2_theta_t);

    vec3 refracted_direction;
    bool total_internal_reflection = false;

    if (sin2_theta_t > 1.0) // Total internal reflection
    {
        total_internal_reflection = true;
        Pixel_Print("Total internal reflection occurred.");
    }
    else
    {
        double cos_theta_t = std::sqrt(1.0 - sin2_theta_t);
        refracted_direction = n_ratio * ray.direction + (n_ratio * cos_theta_i - cos_theta_t) * adjusted_normal;
        Pixel_Print("Refracted direction: ", Vec_To_String(refracted_direction));
    }

    // Compute the reflection direction
    vec3 reflected_direction = ray.direction - 2 * dot(ray.direction, adjusted_normal) * adjusted_normal;
    Pixel_Print("Reflected direction: ", Vec_To_String(reflected_direction));

    // Schlick approximation for reflectivity
    double r0 = pow((n1 - n2) / (n1 + n2), 2);
    double reflectivity = r0 + (1 - r0) * pow(1 - std::abs(cos_theta_i), 5);
    Pixel_Print("Reflectivity (Schlick approximation): ", reflectivity);

    // Cast reflection ray
    Ray reflected_ray(intersection_point + adjusted_normal * 1e-6, reflected_direction);
    Pixel_Print("Casting reflection ray.");
    Debug_Ray("Reflection ray", reflected_ray);
    vec3 reflected_color = render_world.Cast_Ray(reflected_ray, recursion_depth + 1);
    Pixel_Print("Reflected color: ", Vec_To_String(reflected_color));

    // Cast refraction ray if no total internal reflection
    vec3 refracted_color(0, 0, 0);
    if (!total_internal_reflection)
    {
        Ray refracted_ray(intersection_point - adjusted_normal * 1e-6, refracted_direction);
        Pixel_Print("Casting refraction ray.");
        Debug_Ray("Refraction ray", refracted_ray);
        refracted_color = render_world.Cast_Ray(refracted_ray, recursion_depth + 1);
        Pixel_Print("Refracted color: ", Vec_To_String(refracted_color));
    }

    // Combine reflection and refraction using reflectivity
    vec3 combined_reflection_refraction = reflectivity * reflected_color +
                                          (1 - reflectivity) * refracted_color;

    // Combine with base color using opacity
    vec3 final_color = (1 - opacity) * base_color + opacity * combined_reflection_refraction;

    Pixel_Print("Combined reflection/refraction color: ", Vec_To_String(combined_reflection_refraction));
    Pixel_Print("Final color (opacity blend): ", Vec_To_String(final_color));

    return final_color;
}
