/**
 * @file BaseMandelCalculator.h 
 * @author Vojtech Mrazek (mrazek@fit.vutbr.cz)
 * @brief Abstract class for mandelbrot calculator
 * @date 2021-09-24
 * 
 */

#ifndef BASEMANDELCALCULATOR_H
#define BASEMANDELCALCULATOR_H

#include <string>
#include <iostream>

/**
 * @brief Abstract class for Mandelbrot set calculator, calculates the dimensions
 * 
 */
class BaseMandelCalculator
{
public:
    /**
     * @brief Construct a new Base Mandel Calculator object
     * 
     * @param matrixBaseSize basic size (width will be multiplied by 3, height by 2)
     * @param limit number of iterations
     * @param cName name of the calculator
     */
    BaseMandelCalculator(unsigned matrixBaseSize, unsigned limit, const std::string & cName);
    
    /**
     * @brief Prints output to ostream 
     * 
     * @param cout output stream
     * @param batchMode true = compact CSV output
     */
    void info(std::ostream & cout, bool batchMode);
    
    int width; // width of the set
    int height; // hegiht of the set


protected:
    const std::string cName;
    const int limit;
    bool batchMode;


	const double x_start; // minimal real value
	const double x_fin; // maximal real value
	const double y_start; // minimal imag value
	const double y_fin; // maximal imag value
	
    double dx; // step of real vaues
	double dy; // step of imag values
};

#endif