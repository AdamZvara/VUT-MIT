/**
 * @file    main.cpp
 *
 * @authors Filip Vaverka <ivaverka@fit.vutbr.cz>
 *          Vojtech Mrazek <mrazek@fit.vutbr.cz>
 *
 * @brief   AVS Assignment 2
 *          See: https://en.wikipedia.org/wiki/Marching_cubes
 *               https://en.wikipedia.org/wiki/Isosurface
 *               http://paulbourke.net/geometry/polygonise/
 *
 * @date    06 November 2021, 11:07
 **/

#include <string>
#include <omp.h>

#include "cxxopts.hpp"

#include "parametric_scalar_field.h"
#include "ref_mesh_builder.h"
#include "loop_mesh_builder.h"
#include "tree_mesh_builder.h"

int main(int argc, char *argv[])
{
    // Initialize CXXOPTS library used to parse command line arguments
    cxxopts::Options options("AVS: Marching cubes", "AVS Assignment 2 - Parallel Marching Cubes");
    options.add_options()
            ("i,input",  "Input field file",                        cxxopts::value<std::string>())
            ("o,output", "Output mesh file",                        cxxopts::value<std::string>()->default_value(""))
            ("l,level",  "Iso surface value",                       cxxopts::value<float>()->default_value("0.15"))
            ("g,grid",   "Grid size",                               cxxopts::value<unsigned>()->default_value("64"))
            ("b,builder", "Builder name [ref, loop, tree]",         cxxopts::value<std::string>()->default_value("ref"))
            ("t,threads", "Number of OpenMP threads",               cxxopts::value<unsigned>()->default_value("0"))
            ("batch", "Run in silent/batch mode")
            ("h,help", "Print help");
    options.positional_help("INPUT <OUTPUT>");

    try
    {
        options.parse_positional({"input", "output"});

        auto args = options.parse(argc, argv);

        if(args.count("help"))
        {
            std::cout << options.help() << std::endl;
            std::exit(0);
        }

        if(args["input"].count() == 0)
        {
            std::cout << options.help() << std::endl;
            std::cout << "Missing required arguments: \"INPUT\"" << std::endl;
            std::exit(1);
        }

        if(args["threads"].as<unsigned>() > 0)
            omp_set_num_threads(int(args["threads"].as<unsigned>()));

        // Load field from file and build it (using specified isolevel)
        ParametricScalarField field(args["input"].as<std::string>(), args["level"].as<float>());

        if(args["builder"].as<std::string>() == "ref")
        {
            RefMeshBuilder builder(args["grid"].as<unsigned>());
            builder.setBatchMode(args.count("batch"));
            builder.buildMesh(field, args["output"].as<std::string>());
        }
        else if(args["builder"].as<std::string>() == "loop")
        {
            LoopMeshBuilder builder(args["grid"].as<unsigned>());
            builder.setBatchMode(args.count("batch"));
            builder.buildMesh(field, args["output"].as<std::string>());
        }
        else if(args["builder"].as<std::string>() == "tree")
        {
            TreeMeshBuilder builder(args["grid"].as<unsigned>());
            builder.setBatchMode(args.count("batch"));
            builder.buildMesh(field, args["output"].as<std::string>());
        }
        else
        {
            std::cerr << "Unknown builder (" << args["builder"].as<std::string>() << ") specified!" << std::endl;
            std::exit(1);
        }
    }
    catch(const cxxopts::OptionException &e)
    {
        std::cerr << "Invalid options specified: " << e.what() << std::endl;
        std::exit(1);
    }

    return 0;
}
