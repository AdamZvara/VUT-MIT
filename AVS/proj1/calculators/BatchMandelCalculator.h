/**
 * @file BatchMandelCalculator.h
 * @author Adam Zvara <xzvara01@stud.fit.vutbr.cz>
 * @brief Implementation of Mandelbrot calculator that uses SIMD paralelization over small batches
 * @date 2023-10-26
 */
#ifndef BATCHMANDELCALCULATOR_H
#define BATCHMANDELCALCULATOR_H

#include <BaseMandelCalculator.h>

class BatchMandelCalculator : public BaseMandelCalculator
{
public:
    BatchMandelCalculator(unsigned matrixBaseSize, unsigned limit);
    ~BatchMandelCalculator();
    int * calculateMandelbrot();

private:
    // Result matrix
    int *data;

    // SoA to store intermediate results of real and imaginary parts of complex numbers
    float *real;
    float *imag;
};

#endif