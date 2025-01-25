#include "light.h"
#include "parse.h"
#include "object.h"
#include "phong_shader.h"
#include "ray.h"
#include "render_world.h"

Phong_Shader::Phong_Shader(const Parse* parse, std::istream& in)
{
    in >> name;
    color_ambient = parse->Get_Color(in);
    color_diffuse = parse->Get_Color(in);
    color_specular = parse->Get_Color(in);
    in >> specular_power;

    // Ensure all colors are valid
    if (!color_ambient || !color_diffuse || !color_specular)
    {
        throw std::runtime_error("Failed to initialize Phong_Shader colors.");
    }
}

vec3 Phong_Shader::Shade_Surface(const Render_World& render_world, const Ray& ray, const Hit& hit,
                                 const vec3& intersection_point, const vec3& normal, int recursion_depth) const
{
    vec3 color(0, 0, 0);

    // Ensure the normal is normalized
    vec3 norm = normal.normalized();
    if (norm.magnitude_squared() < 1e-6)
    {
        return color; // Return black for invalid normals
    }

    // Retrieve material properties
    vec3 ambient_color = color_ambient ? color_ambient->Get_Color(hit.uv) : vec3(0, 0, 0);
    vec3 diffuse_color = color_diffuse ? color_diffuse->Get_Color(hit.uv) : vec3(0, 0, 0);
    vec3 specular_color = color_specular ? color_specular->Get_Color(hit.uv) : vec3(0, 0, 0);

    // Small epsilon offset to avoid self-intersection
    const double epsilon = 1e-4;
    vec3 offset_point = intersection_point + norm * epsilon;

    // Ambient component
    if (render_world.ambient_color)
    {
        vec3 ambient = render_world.ambient_intensity *
                       render_world.ambient_color->Get_Color(vec2(0, 0)) * ambient_color;
        color += ambient;
    }

    // Iterate over all lights in the scene
    for (const auto* light : render_world.lights)
    {
        if (!light)
            continue; // Skip null pointers

        // Light direction and light intensity
        vec3 l = (light->position - intersection_point); // Light vector
        vec3 light_intensity = light->Emitted_Light(l);

        // Shadow handling
        bool in_shadow = false;
        if (render_world.enable_shadows)
        {
            Ray shadow_ray(offset_point, l);
            auto [shadowed_object, shadow_hit] = render_world.Closest_Intersection(shadow_ray);

            // Determine if the light is blocked
            if (shadowed_object.object && shadow_hit.dist < l.magnitude())
            {
                in_shadow = true;
            }
        }

        // If not in shadow, calculate diffuse and specular contributions
        if (!in_shadow)
        {
            // Diffuse component
            vec3 light_dir = l.normalized();
            double diffuse_factor = std::max(dot(norm, light_dir), 0.0);
            vec3 diffuse = diffuse_color * light_intensity * diffuse_factor;
            color += diffuse;

            // Specular component
            vec3 view_dir = -ray.direction.normalized();
            vec3 reflection_dir = (2.0 * dot(light_dir, norm) * norm - light_dir); // Correct reflection vector

            double specular_factor = pow(std::max(dot(view_dir, reflection_dir.normalized()), 0.0), specular_power);
            vec3 specular = specular_color * light_intensity * specular_factor;
            color += specular;
        }
    }

    return color;
}
