/**
 * @file BaseMandelCalculator.cc
 * @author Vojtech Mrazek (mrazek@fit.vutbr.cz)
 * @brief Abstract class, precalculates the constant values in calculator
 * @date 2021-09-24
 */
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include "BaseMandelCalculator.h"

BaseMandelCalculator::BaseMandelCalculator(unsigned matrixBaseSize, unsigned limit, const std::string &cName)
	: width(3 * matrixBaseSize), height(2 * matrixBaseSize), x_start(-2.0), x_fin(1.0), y_start(-1.5), y_fin(1.5), limit(limit), cName(cName)

{
	dx = (x_fin - x_start) / (width - 1);
	dy = (y_fin - y_start) / (height - 1);
}

void BaseMandelCalculator::info(std::ostream &cout, bool batchMode)
{
	if (batchMode)
	{
		cout << cName << ";";
		cout << width / 3 << ";";
		cout << width << ";" << height << ";";
		cout << limit << ";";
	}
	else
	{
		cout << "======================== Mandelbrot SIMD calculator ==========================" << std::endl;
		cout << "Calculator:        " << cName << std::endl;
		cout << "Base size:         " << width / 3 << std::endl;
		cout << "Matrix size:       " << width << "x" << height << std::endl;
		cout << "Iteration limit:   " << limit << std::endl;
	}
}
