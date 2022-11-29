#include "utils.h"
#include <iostream>
#include <sstream>

//helper function which returns double values from AVX2 double vector as one string
std::string pd_v_str(__m256d vec)
{
	std::stringstream ss;
	__m128d a = _mm256_extractf128_pd(vec, 0);
	__m128d b = _mm256_extractf128_pd(vec, 1);

	double doubles[4];
	double* p = doubles;
	_mm_storel_pd(p, a);
	_mm_storeh_pd(p + 1, a);
	_mm_storel_pd(p + 2, b);
	_mm_storeh_pd(p + 3, b);

	 ss << doubles[0] << ", " << doubles[1] << ", " << doubles[2] << ", " << doubles[3];
	 std::string result = ss.str();

	return result;
}

distr decide_distribution(Stats stats)
{
	if (stats.kurtosis() > 3)
	{
		return exponential;
	}
	return normal;
}
