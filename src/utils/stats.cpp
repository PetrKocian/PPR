#include "stats.h"
#include <cmath>
#include <iostream>

/*
Implementation based on
https://www.johndcook.com/blog/skewness_kurtosis/
*/

void Stats::set_stats(Stats_partial sp)
{
	this->m1 = sp.m1;
	this->m2 = sp.m2;
	this->m3 = sp.m3;
	this->m4 = sp.m4;
	this->n = sp.n;
	this->only_ints = sp.only_ints;
}

Stats_partial combine_stats(double count_a, double count_b, double m4_a, double m4_b, double m3_a, double m3_b, double m2_a, double m2_b, double m1_a, double m1_b, double only_ints_a, double only_ints_b)
{
	Stats_partial result;

	//return only a/b if the count of the other one is 0
	if (count_a == 0)
	{
		result.m1 = m1_b;
		result.m2 = m2_b;
		result.m3 = m3_b;
		result.m4 = m4_b;
		result.n = count_b;
		result.only_ints = only_ints_b;

		return result;
	}
	else if (count_b == 0)
	{
		result.m1 = m1_a;
		result.m2 = m2_a;
		result.m3 = m3_a;
		result.m4 = m4_a;
		result.n = count_a;
		result.only_ints = only_ints_a;

		return result;
	}

	//implementation of formulas for combining m properties
	double delta_f = m1_b - m1_a;
	double n_combined = count_a + count_b;
	double delta_n_f = delta_f / (n_combined);
	double delta_n2_f = delta_n_f * delta_n_f;
	double delta_n3_f = delta_n_f * delta_n2_f;
	double count_sq = count_a * count_a;
	double rn_sq = count_b * count_b;

	double m1_combined = (m1_a * count_a + m1_b * count_b) / (count_a + count_b);

	double m4_combined = m4_a + m4_b + delta_f * delta_n3_f * count_a * count_b *
		(count_sq - count_a * count_b + rn_sq) + 6 * delta_n2_f * (count_sq * m2_b + rn_sq * m2_a) +
		4 * delta_n_f * (count_a * m3_b - count_b * m3_a);

	double m2_combined = m2_a + m2_b + delta_f * delta_n_f * count_a * count_b;

	double m3_combined = m3_a + m3_b + delta_f * delta_n2_f * count_a * count_b * (count_a - count_b) +
		3 * delta_n_f * (count_a * m2_b - count_b * m2_a);

	//return combined stats
	result.n = n_combined;
	result.m1 = m1_combined;
	result.m2 = m2_combined;
	result.m3 = m3_combined;
	result.m4 = m4_combined;
	result.only_ints = only_ints_a + only_ints_b;

	return result;
}

void Stats::clear()
{
	m1 = 0;
	m2 = 0;
	m3 = 0;
	m4 = 0;
	n = 0;
	only_ints = 0;
	n_v = _mm256_setzero_pd();
	n1_v = _mm256_setzero_pd();
	m1_v = _mm256_setzero_pd();
	m2_v = _mm256_setzero_pd();
	m3_v = _mm256_setzero_pd();
	m4_v = _mm256_setzero_pd();
	delta_v = _mm256_setzero_pd();
	delta_n_v = _mm256_setzero_pd();
	delta_n2_v = _mm256_setzero_pd();
	term1_v = _mm256_setzero_pd();
	term1_temp_v = _mm256_setzero_pd();
}

void Stats::push_v(__m256d vec_x)
{
	n1_v = n_v;
	n_v = _mm256_add_pd(n_v, { 1.0,1.0,1.0,1.0 });

	delta_v = _mm256_sub_pd(vec_x, m1_v);
	delta_n_v = _mm256_div_pd(delta_v, n_v);
	delta_n2_v = _mm256_mul_pd(delta_n_v, delta_n_v);
	term1_temp_v = _mm256_mul_pd(delta_v, delta_n_v);
	term1_v = _mm256_mul_pd(term1_temp_v, n1_v);

	m1_v = _mm256_add_pd(m1_v, delta_n_v);

	m4_v = _mm256_add_pd(m4_v, _mm256_fmadd_pd(_mm256_mul_pd(term1_v, delta_n2_v)
		, _mm256_add_pd(_mm256_sub_pd(_mm256_mul_pd(n_v, n_v)
		, _mm256_mul_pd({ 3.0,3.0,3.0,3.0 }, n_v)), { 3.0,3.0,3.0,3.0 })
		, _mm256_fmadd_pd(_mm256_mul_pd({ 6.0,6.0,6.0,6.0 }, delta_n2_v), m2_v,
			_mm256_mul_pd(_mm256_mul_pd({ -4.0,-4.0,-4.0,-4.0 }, delta_n_v), m3_v))));

	m3_v = _mm256_add_pd(m3_v, _mm256_sub_pd(_mm256_mul_pd(_mm256_mul_pd(term1_v, delta_n_v)
		, _mm256_sub_pd(n_v, { 2.0,2.0,2.0,2.0 })),
		_mm256_mul_pd(_mm256_mul_pd({ 3.0,3.0,3.0,3.0 }
		, delta_n_v), m2_v)));

	m2_v = _mm256_add_pd(m2_v, term1_v);

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

double Stats::kurtosis() const
{
	if (n == 0)
	{
		return 0;
	}
	else
	{
		return static_cast<double>(n)* m4 / (m2 * m2) - 3.0;
	}
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

double Stats::get_only_ints() const
{
	return only_ints;
}

void Stats::finalize_stats()
{
	if (n_of_v() == 0)
	{
		return;
	}

	//access AVX2 vectors
	double* n_p = (double*)& n_v;
	double* m1_p = (double*)& m1_v;
	double* m2_p = (double*)& m2_v;
	double* m3_p = (double*)& m3_v;
	double* m4_p = (double*)& m4_v;

	//combine first and second half of AVX2 vector
	Stats_partial a = combine_stats(n_p[0], n_p[1], m4_p[0], m4_p[1], m3_p[0], m3_p[1], m2_p[0], m2_p[1], m1_p[0], m1_p[1], 0, 0);
	Stats_partial b = combine_stats(n_p[2], n_p[3], m4_p[2], m4_p[3], m3_p[2], m3_p[3], m2_p[2], m2_p[3], m1_p[2], m1_p[3], 0, 0);

	//combine AVX2 halves together
	Stats_partial result_partial = combine_stats(a.n, b.n, a.m4, b.m4, a.m3, b.m3, a.m2, b.m2, a.m1, b.m1, a.only_ints, b.only_ints);

	//combine AVX2 and single doubles
	Stats_partial result_final = combine_stats(result_partial.n, n, result_partial.m4, m4, result_partial.m3, m3, result_partial.m2, m2, result_partial.m1, m1, only_ints, result_partial.only_ints);

	//set all variables to zero in case this function is called multiple times
	this->clear();

	//assign final values
	this->m1 = result_final.m1;
	this->m2 = result_final.m2;
	this->m3 = result_final.m3;
	this->m4 = result_final.m4;
	this->n = result_final.n;
	this->only_ints = result_final.only_ints;

}

void Stats::add_stats(Stats stats_to_add)
{
	Stats_partial result = combine_stats(n, stats_to_add.n, m4, stats_to_add.m4, m3, stats_to_add.m3, m2, stats_to_add.m2, m1, stats_to_add.m1, only_ints, stats_to_add.only_ints);

	this->m1 = result.m1;
	this->m2 = result.m2;
	this->m3 = result.m3;
	this->m4 = result.m4;
	this->n = result.n;
	this->only_ints = result.only_ints;

}

void Stats::add_stats(Stats_partial stats_to_add)
{
	Stats_partial result = combine_stats(n, stats_to_add.n, m4, stats_to_add.m4, m3, stats_to_add.m3, m2, stats_to_add.m2, m1, stats_to_add.m1, only_ints, stats_to_add.only_ints);

	this->m1 = result.m1;
	this->m2 = result.m2;
	this->m3 = result.m3;
	this->m4 = result.m4;
	this->n = result.n;
	this->only_ints = result.only_ints;

}

void Stats::push_only_int(double x)
{
	only_ints += x;
}

double Stats::mean() const
{
	return m1;
}

uint64_t Stats::get_n() {
	return n;
}

uint64_t Stats::n_of_v()
{
	double n_temp = 0;
	__m128d a = _mm256_extractf128_pd(n_v, 0);
	__m128d b = _mm256_extractf128_pd(n_v, 1);
	
	double ns[4];
	double* p = ns;
	_mm_storel_pd(p, a);
	_mm_storeh_pd(p + 1, a);
	_mm_storel_pd(p + 2, b);
	_mm_storeh_pd(p + 3, b);
	
	for (int i = 0; i < 4; i++)
	{
		n_temp += ns[i];
	}
	
	uint64_t n_result = static_cast<uint64_t>(n_temp);
	return n_result;
}

double Stats::get_distr_par(uint16_t distr)
{
	distr_type type = static_cast<distr_type>(distr);
	switch (type)
	{
	case normal:
		return 0;
		break;
	case poisson:
		return 1/this->mean();
		break;
	case exponential:
		return 6;
		break;
	case uniform:
		return (-6.0 / 5.0);
		break;
	case unknown:
	default:
		return 0;
		break;
	}
}

int Stats::get_distribution_s()
{
	double mean = this->mean();
	double kurtosis = this->kurtosis();
	int index = 0;
	distr_type return_type;

	double min_dev_from_distribution = 100;
	for (int i = 0; i < 4; i++)
	{
		//do not return poisson if data contains non integers
		if (!this->only_integers() && i == 1)
		{
			continue;
		}
		//do not return normal if data contains only ints and mean > 20 => poission with m>20 is very similar to normal
		if (this->mean() > 20 && this->only_integers() && i == 0)
		{
			continue;
		}
		//get distribution with smallest deviation from kurtosis
		if (std::abs(kurtosis - get_distr_par(i)) < min_dev_from_distribution)
		{
			min_dev_from_distribution = std::abs(kurtosis - get_distr_par(i));
			index = i;
		}
	}

	switch (static_cast<distr_type>(index))
	{
	case normal:
		return_type = normal;
		break;
	case poisson:
		return_type = poisson;
		break;
	case exponential:
		return_type = exponential;
		break;
	case uniform:
		return_type = uniform;
		break;
	case unknown:
	default:
		return_type = unknown;
		break;
	}

	return return_type;	
}
