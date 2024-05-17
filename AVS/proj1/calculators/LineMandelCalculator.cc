/**
 * @file LineMandelCalculator.cc
 * @author Adam Zvara <xzvara01@stud.fit.vutbr.cz>
 * @brief Implementation of Mandelbrot calculator that uses SIMD paralelization over lines
 * @date 2023-10-26
 */
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <stdlib.h>


#include "LineMandelCalculator.h"


LineMandelCalculator::LineMandelCalculator (unsigned matrixBaseSize, unsigned limit) :
	BaseMandelCalculator(matrixBaseSize, limit, "LineMandelCalculator")
{
	// Allocate mem for result matrix
	data = (int *)_mm_malloc(height * width * sizeof(int), 64);

	// And for intermediate results - only need half of the height of original matrix
	real = (float *)_mm_malloc(height / 2 * width * sizeof(float), 64);
	imag = (float *)_mm_malloc(height / 2 * width * sizeof(float), 64);
}

LineMandelCalculator::~LineMandelCalculator() {
	_mm_free(data);
	_mm_free(real);
	_mm_free(imag);
}

#pragma omp declare simd notinbranch simdlen(64)
template <typename T>
static inline int mandelbrot(T* real, T* imag, T real_start, T imag_start, int *data, int index) {
	T r2 = real[index] * real[index];
	T i2 = imag[index] * imag[index];
	
	// If the value has escaped, we can signal our caller that we can stop calculating
	if (r2 + i2 > 4.0f) {
		return 1;
	}

	// Otherwise calculate the next iteration
	imag[index] = 2.0f * real[index] * imag[index] + imag_start;
	real[index] = r2 - i2 + real_start;

	// Increment the number of iterations
	data[index] += 1;

	// Signal that we still haven't reached the threshold
	return 0;
}

int * LineMandelCalculator::calculateMandelbrot () {
	// Need to declare width and height as constants, otherwise the compiler will not vectorize
	const int h = height;
	const int w = width;
	
	float* preal = real;
	float* pimag = imag;
	int* pdata = data;

	// Initialize starting values
	#pragma omp simd aligned(preal, pimag:64) simdlen(64)
	for (int i = 0; i < h / 2 * w; i++) {
		preal[i] = x_start + (i % w) * dx;
		pimag[i] = y_start + (i / w) * dy;
	}

	// Initialize result matrix
	#pragma omp simd aligned(pdata:64) simdlen(64)
	for (int i = 0; i < w * h; i++) {
		pdata[i] = 0;
	}

	// Iterate through half of the columns in matrix (other half is symetric)
	for (int i = 0; i < h / 2; i++) {
		float y = y_start + i * dy; // current imaginary value for the whole row
		// Iterate through limits for the current row
		for (int k = 1; k < limit; k++) {
			int end_row = 0;

			#pragma omp simd reduction(+:end_row) linear(i, k) simdlen(64) 
			for (int j = 0; j < w; j++) {
				float x = x_start + j * dx; // current real value
				const int idx = i * w + j;
				end_row += mandelbrot(preal, pimag, x, y, pdata, idx);
			}	
			// If all values have escaped (going to infinity), we can stop
			if (end_row == width) {
				break;
			}
		}
	}

	// Fill the rest of the array from calculated values
	for (int i = h / 2; i < h; i++) {
		for (int j = 0; j < w; j++) {
			int idx = i * w + j;
			int idx2 = (h - i - 1) * w + j;
			pdata[idx] = pdata[idx2];
		}
	}
	
	return data;
}
