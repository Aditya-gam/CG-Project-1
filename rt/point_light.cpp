#include "point_light.h"
#include "parse.h"
#include "color.h"

Point_Light::Point_Light(const Parse* parse,std::istream& in)
{
    in>>name>>position;
    color=parse->Get_Color(in);
    in>>brightness;
}

vec3 Point_Light::Emitted_Light(const vec3& vector_to_light) const
{
    double dist2 = vector_to_light.magnitude_squared();
    if(dist2 < 1e-16)
    {
        // If the intersection is essentially at the light source, 
        // we can return some reasonable default (like no falloff).
        return color->Get_Color({}) * brightness;
    }
    // Otherwise, do the usual falloff
    return color->Get_Color({}) * brightness / (4.0 * pi * dist2);
}

