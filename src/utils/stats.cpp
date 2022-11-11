#include "stats.h"
#include "utils.h"
#include <cmath>
#include <iostream>

void Stats::push_v(__m256d vec_x)
{
	n1_v = n_v;
	n_v = _mm256_add_pd(n_v, increment_v);

	delta_v = _mm256_sub_pd(vec_x, m1_v);
	delta_n_v = _mm256_div_pd(delta_v, n_v);
	delta_n2_v = _mm256_mul_pd(delta_n_v, delta_n_v);
	term1_temp_v = _mm256_mul_pd(delta_v, delta_n_v);
	term1_v = _mm256_mul_pd(term1_temp_v, n1_v);
	m1_v = _mm256_add_pd(m1_v, delta_n_v);

	/*
	std::cout << "vec: " << pd_v_str(delta_v) << std::endl;
	std::cout << "delta_v " << pd_v_str(delta_v) << std::endl;
	std::cout << "delta_n_v " << pd_v_str(delta_n_v) << std::endl;
	std::cout << "m1_v " << pd_v_str(m1_v) << std::endl;
	*/
}

double Stats::mean_v()
{
	//TODO: change to stream_pd
	double mean_temp = 0;
	__m128d a = _mm256_extractf128_pd(m1_v, 0);
	__m128d b = _mm256_extractf128_pd(m1_v, 1);

	double means[4];
	double* p = means;
	_mm_storel_pd(p, a);
	_mm_storeh_pd(p+1, a);
	_mm_storel_pd(p+2, b);
	_mm_storeh_pd(p+3, b);

	//_mm256_stream_pd(means, m1_v);
	for (int i = 0; i < 4; i++)
	{
		mean_temp += means[i];
	}

	//std::cout << "mean vector " <<  pd_v_str(m1_v) <<std::endl;
	return mean_temp / 4;

}

void Stats::push(double x)
{
	double delta, delta_n, delta_n2, term1; 

	long long n1 = n;
	only_ints += x - std::floor(x);
	n++;
	delta = x - m1;
	delta_n = delta / n;
	delta_n2 = delta_n * delta_n;
	term1 = delta * delta_n * n1;
	m1 += delta_n;
	m4 += term1 * delta_n2 * (n * n - 3 * n + 3) + 6 * delta_n2 * m2 - 4 * delta_n * m3;
	m3 += term1 * delta_n * (n - 2) - 3 * delta_n * m2;
	m2 += term1;
}

void Stats::clear()
{
	m1 = 0;
	m2 = 0;
	m3 = 0;
	m4 = 0;
	n = 0;
	only_ints = 0;
}

double Stats::mean() const
{
	return m1;
}

double Stats::kurtosis() const
{
	return static_cast<double>(n)* m4 / (m2 * m2) - 3.0;
}

bool Stats::only_integers() const
{
	bool result = false;
	if (only_ints == 0)
	{
		result = true;
	}
	return result;
}