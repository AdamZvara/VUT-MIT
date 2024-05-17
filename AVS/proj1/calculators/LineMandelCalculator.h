/**
 * @file LineMandelCalculator.h
 * @author Adam Zvara <xzvara01@stud.fit.vutbr.cz>
 * @brief Implementation of Mandelbrot calculator that uses SIMD paralelization over lines
 * @date 2023-10-26
 */

#include <BaseMandelCalculator.h>

class LineMandelCalculator : public BaseMandelCalculator
{
public:
    LineMandelCalculator(unsigned matrixBaseSize, unsigned limit);
    ~LineMandelCalculator();
    int *calculateMandelbrot();

private:
    // Result matrix
    int *data;

    // SoA to store intermediate results of real and imaginary parts of complex numbers
    float *real;
    float *imag;
};