#include "dump_png.h"
#include "object.h"
#include "parse.h"
#include "render_world.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <unistd.h>

/*

  Usage: ./ray_tracer -i <test-file> [ -s <solution-file> ] [ -o <output-file> ] [ -x <debug-x-coord> -y <debug-y-coord> ]

  Examples:

  ./ray_tracer -i 00.txt

  Renders the scene described by 00.txt.  Dumps the result to output.png.

  ./ray_tracer -i 00.txt -s 00.png

  Renders the scene described by 00.txt.  Dumps the result to output.png.
  Compares this to the solution in 00.png and dumps the error to diff.png.
  Outputs a measure of the error.

  ./ray_tracer -i 00.txt -x 123 -y 234

  The -x and -y flags give you the opportunity to print out lots of detailed
  information about the rendering of a single pixel.  This allows you to be
  verbose about a pixel of interest without printing this information for every
  pixel.  For many of the scenes, there is a pixel trace on the project page
  detailing the results of various computations (intersections, shading, etc.)
  for one specially chosen pixel.

  If you are getting the wrong results for a test, check to see if there is a
  pixel trace available for that test.  When there is, the pixel trace is often
  the easiest way to debug the problem.  Print out the same information from
  your code that is available in the pixel trace.  This will tell you which
  quantities are being computed correctly and can help you narrow down your
  search.

  The -f flag is used by the grading script.  It causes the results of your ray
  tracer to be printed to a file rather than to the standard output.  This
  prevents the grading script from getting confused by debugging output.

  The -o flag can be used to specify the output file.  The default is
  output.png.

  The -h flag disables the acceleration structure.  This is useful for testing
  and debugging the acceleration structure.

  The -z flag changes the resolution of the acceleration structure.  This is
  useful for testing correctness, runtime performance, and scaling.
 */

// Indicates that we are debugging one pixel; can be accessed everywhere.
bool Debug_Scope::enable=false;
int Debug_Scope::level=0;
bool enable_acceleration=true;
int acceleration_grid_size=40;

void Usage(const char* exec)
{
    std::cerr<<"Usage: "<<exec<<" -i <test-file> [ -s <solution-file> ] [ -f <stats-file> ] [ -o <output-file> ] [ -x <debug-x-coord> -y <debug-y-coord> ] [ -h ]  [ -z <resolution> ] "<<std::endl;
    exit(1);
}

void Setup_Parsing(Parse& parse);

int main(int argc, char** argv)
{
    const char* solution_file = 0;
    const char* input_file = 0;
    const char* output_file = "output.png";
    const char* statistics_file = 0;
    int test_x=-1, test_y=-1;

    // Parse commandline options
    while(1)
    {
        int opt = getopt(argc, argv, "s:i:o:f:x:y:hz:");
        if(opt==-1) break;
        switch(opt)
        {
            case 's': solution_file = optarg; break;
            case 'i': input_file = optarg; break;
            case 'f': statistics_file = optarg; break;
            case 'o': output_file = optarg; break;
            case 'x': test_x = atoi(optarg); break;
            case 'y': test_y = atoi(optarg); break;
            case 'h': enable_acceleration=false; break;
            case 'z': acceleration_grid_size = atoi(optarg); break;
        }
    }
    if(!input_file) Usage(argv[0]);

    Render_World render_world;
    
    // Parse test scene file
    Parse parse;
    Setup_Parsing(parse);
    
    std::ifstream fin(input_file);
    if(!fin)
    {
        std::cerr<<"Error: Failed to open file "<<input_file<<std::endl;
        exit(1);
    }
    assert(fin);
    parse.Parse_Input(render_world,fin);
    
    // Render the image
    render_world.Render();

    // For debugging.  Render only the pixel specified on the commandline.
    // Useful for printing out information about a single pixel.
    if(test_x>=0 && test_y>=0)
    {
        // Set a global variable to indicate that we are debugging one pixel.
        // This way you can do: debug.Print("foo = ",foo);
        Debug_Scope::enable = true;
        std::cout<<"debug pixel: -x "<<test_x<<" -y "<<test_y<<std::endl;

        // Render just the pixel we are debugging
        render_world.Render_Pixel(ivec2(test_x,test_y));

        // Mark the pixel we are testing green in the output image.
        render_world.camera.Set_Pixel(ivec2(test_x,test_y),0x00ff00ff);
    }

    // Save the rendered image to disk
    Dump_png(render_world.camera.colors,render_world.camera.number_pixels[0],render_world.camera.number_pixels[1],output_file);
    
    // If a solution is specified, compare against it.
    if(solution_file)
    {
        int width = 0, height = 0;
        Pixel* data_sol = 0;

        // Read solution from disk
        Read_png(data_sol,width,height,solution_file);
        assert(render_world.camera.number_pixels[0]==width);
        assert(render_world.camera.number_pixels[1]==height);

        // For each pixel, check to see if it matches solution
        double error = 0, total = 0;
        for(int i=0; i<height*width; i++)
        {
            vec3 a=From_Pixel(render_world.camera.colors[i]);
            vec3 b=From_Pixel(data_sol[i]);
            for(int c=0; c<3; c++)
            {
                double e = fabs(a[c]-b[c]);
                error += e;
                total++;
                b[c] = e;
            }
            data_sol[i]=Pixel_Color(b);
        }

        // Output information on how well it matches. Optionally save to file
        // to avoid getting confused by debugging print statements.
        FILE* stats_file = stdout;
        if(statistics_file) stats_file = fopen(statistics_file, "w");
        fprintf(stats_file,"diff: %.2f\n",error/total*100);
        if(statistics_file) fclose(stats_file);

        // Output images showing the error that was computed to aid debugging
        Dump_png(data_sol,width,height,"diff.png");
        delete [] data_sol;
    }

    return 0;
}

