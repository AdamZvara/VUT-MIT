/**
 * @file    main.cc
 *
 * @authors Vojtech Mrazek <mrazek@fit.vutbr.cz>
 *
 * @brief   AVS Assignment 1
 *          See: https://en.wikipedia.org/wiki/Mandelbrot_set
 *
 * @date    24 September 2021, 11:07
 **/
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include "cxxopts.hpp"

#include "cnpy.h"
#include "vector_helpers.h"

#include "RefMandelCalculator.h"
#include "LineMandelCalculator.h"
#include "BatchMandelCalculator.h"

using namespace std;

/**
 * @brief Creates mandelbrot calculator object (template T), evaluates the
 *        speed, and prints output
 **/
template <typename T>
void evaluateCalculator(unsigned baseSize, unsigned iters, const std::string &fileName, bool batchMode)
{
	T calculator(baseSize, iters);

	calculator.info(std::cout, batchMode);

	auto startTime = PerfClock_t::now();
	auto data = calculator.calculateMandelbrot();
	auto elapsedTime = PerfClockDurationMs(PerfClock_t::now() - startTime).count();

	if (batchMode)
		std::cout << elapsedTime << std::endl;
	else
	{
		std::cout << "Elapsed Time:      " << elapsedTime << " ms" << std::endl;
	}

	if (fileName.length() > 0)
	{
		if(data == NULL)
			std::cerr << "No data returned, skipping saving!" << std::endl;
		else
			cnpy::npz_save(fileName, "d", data, {(size_t)calculator.height, (size_t)calculator.width}, "wb");
	}
}

int main(int argc, char *argv[])
{

	// Initialize CXXOPTS library used to parse command line arguments
	cxxopts::Options options("AVS: Mandelbrot", "AVS Assignment 1 - Mandelbrot calculation using SIMD instructions");
	options.add_options()
		("o,output", "Output numpy file", cxxopts::value<std::string>()->default_value(""))
		("s,size", "Base matrix size", cxxopts::value<unsigned>()->default_value("2048"))
		("i,iters", "Number of iterations", cxxopts::value<unsigned>()->default_value("100"))
		("c,calculator", "Calculator name [ref, batch, line]", cxxopts::value<std::string>()->default_value("ref"))
		("batch", "Run in silent/batch mode")
		("h,help", "Print help");

	options.positional_help("<OUTPUT>");

	try
	{
		options.parse_positional({"output"});

		auto args = options.parse(argc, argv);

		if (args.count("help"))
		{
			std::cout << options.help() << std::endl;
			std::exit(0);
		}

		const std::string calculator = args["calculator"].as<std::string>();
		if (calculator == "ref")
		{
			evaluateCalculator<RefMandelCalculator>(args["size"].as<unsigned>(), args["iters"].as<unsigned>(), args["output"].as<std::string>(), args.count("batch"));
		}
		else if (calculator == "line")
		{
			evaluateCalculator<LineMandelCalculator>(args["size"].as<unsigned>(), args["iters"].as<unsigned>(), args["output"].as<std::string>(), args.count("batch"));
		}
		else if (calculator == "batch")
		{
			evaluateCalculator<BatchMandelCalculator>(args["size"].as<unsigned>(), args["iters"].as<unsigned>(), args["output"].as<std::string>(), args.count("batch"));
		}
		else
		{
			std::cerr << "Unknown calculator (" << calculator << ")" << std::endl;
			std::exit(1);
		}
	}
	catch (const cxxopts::OptionException &e)
	{
		std::cerr << "Invalid options specified: " << e.what() << std::endl;
		std::exit(1);
	}

	return 0;
}
