#include "parse.h"
#include "texture.h"
#include "dump_png.h"
#include "misc.h"
#include <cmath>

Texture::Texture(const Parse* parse, std::istream& in)
{
    std::string filename;
    in >> name >> filename >> use_bilinear_interpolation;
    Read_png(data, width, height, filename.c_str());
}

Texture::~Texture()
{
    delete[] data;
}

// Helper function to wrap floating-point values into the range [0, 1)
inline double Wrap_Float(double value, double max)
{
    double wrapped = std::fmod(value, max);
    if (wrapped < 0) wrapped += max;
    return wrapped;
}

vec3 Texture::Get_Color(const vec2& uv) const
{
    // Wrap texture coordinates to ensure they are in the range [0, 1)
    double u = Wrap_Float(uv[0], 1.0);
    double v = Wrap_Float(uv[1], 1.0);

    // Convert to pixel indices
    int i = std::min(static_cast<int>(u * width), width - 1);
    int j = std::min(static_cast<int>(v * height), height - 1);

    // Get the pixel color at the calculated index
    const Pixel& pixel = data[j * width + i];

    // Convert the Pixel to vec3 (assuming RGB values are scaled between 0 and 255)
    return vec3(pixel >> 24, (pixel >> 16) & 0xff, (pixel >> 8) & 0xff) / 255.0;
}
