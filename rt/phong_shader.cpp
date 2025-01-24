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
}

vec3 Phong_Shader::
Shade_Surface(const Render_World& render_world, const Ray& ray, const Hit& hit,
    const vec3& intersection_point, const vec3& normal, int recursion_depth) const
{
    vec3 color(0, 0, 0);

    // Retrieve the color components using Get_Color
    vec3 ambient_color = color_ambient->Get_Color(hit.uv);
    vec3 diffuse_color = color_diffuse->Get_Color(hit.uv);
    vec3 specular_color = color_specular->Get_Color(hit.uv);

    // Ambient component
    vec3 ambient = render_world.ambient_intensity *
                   render_world.ambient_color->Get_Color(vec2(0, 0)) * ambient_color;
    color += ambient;

    // Iterate over all lights
    for (const auto* light : render_world.lights)
    {
        vec3 light_dir = (light->position - intersection_point).normalized();
        vec3 light_intensity = light->Emitted_Light(light_dir);

        // Shadow check
        bool in_shadow = false;
        if (render_world.enable_shadows)
        {
            Ray shadow_ray(intersection_point + normal * small_t, light_dir);
            auto [shadowed_object, shadow_hit] = render_world.Closest_Intersection(shadow_ray);

            if (shadow_hit.dist >= small_t && shadow_hit.dist < (light->position - intersection_point).magnitude())
            {
                in_shadow = true;
            }
        }

        if (!in_shadow)
        {
            // Diffuse component
            double diffuse_factor = std::max(dot(normal, light_dir), 0.0);
            vec3 diffuse = diffuse_color * light_intensity * diffuse_factor;

            // Specular component
            vec3 view_dir = -ray.direction.normalized();
            vec3 reflection_dir = (2 * dot(normal, light_dir) * normal - light_dir).normalized();
            double specular_factor = pow(std::max(dot(view_dir, reflection_dir), 0.0), specular_power);
            vec3 specular = specular_color * light_intensity * specular_factor;

            // Add contributions
            color += diffuse + specular;
        }
    }

    return color;
}
