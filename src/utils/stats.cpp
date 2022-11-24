#include "stats.h"
#include "utils.h"
#include <cmath>
#include <iostream>

Stats_partial Stats::combine_stats(double count_a, double count_b, double m4_a, double m4_b, double m3_a, double m3_b, double m2_a, double m2_b, double m1_a, double m1_b)
{
	Stats_partial result;

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

	result.n = n_combined;
	result.m1 = m1_combined;
	result.m2 = m2_combined;
	result.m3 = m3_combined;
	result.m4 = m4_combined;

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
	only_ints_v = _mm256_setzero_pd();
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
	m4_v = _mm256_add_pd(m4_v, _mm256_fmadd_pd(_mm256_mul_pd(term1_v, delta_n2_v), _mm256_add_pd(_mm256_sub_pd(_mm256_mul_pd(n_v, n_v),
		_mm256_mul_pd({ 3.0,3.0,3.0,3.0 }, n_v)), { 3.0,3.0,3.0,3.0 })
		, _mm256_fmadd_pd(_mm256_mul_pd({ 6.0,6.0,6.0,6.0 }, delta_n2_v), m2_v,
			_mm256_mul_pd(_mm256_mul_pd({ -4.0,-4.0,-4.0,-4.0 }, delta_n_v), m3_v))));
	m3_v = _mm256_add_pd(m3_v, _mm256_sub_pd(_mm256_mul_pd(_mm256_mul_pd(term1_v, delta_n_v), _mm256_sub_pd(n_v, { 2.0,2.0,2.0,2.0 })),
		_mm256_mul_pd(_mm256_mul_pd({ 3.0,3.0,3.0,3.0 }, delta_n_v), m2_v)));
	m2_v = _mm256_add_pd(m2_v, term1_v);

	/*
	std::cout << "n: " << pd_v_str(n_v) << std::endl;
	std::cout << "vec: " << pd_v_str(delta_v) << std::endl;
	std::cout << "delta_v " << pd_v_str(delta_v) << std::endl;
	std::cout << "delta_n_v " << pd_v_str(delta_n_v) << std::endl;
	std::cout << "delta_n2_v " << pd_v_str(delta_n2_v) << std::endl;
	std::cout << "m1_v " << pd_v_str(m1_v) << std::endl;
	std::cout << "m2_v " << pd_v_str(m2_v) << std::endl;
	std::cout << "m3_v " << pd_v_str(m3_v) << std::endl;
	std::cout << "m4_v " << pd_v_str(m4_v) << std::endl;
	*/

	/*
	double* test = (double*)& vec_x;
	push(test[0]);
	push(test[1]);
	push(test[2]);
	push(test[3]);
	*/

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

	
	std::cout << "m1 " << m1 << std::endl;
	std::cout << "m2 " << m2 << std::endl;
	std::cout << "m3 " << m3 << std::endl;
	std::cout << "m4 " << m4 << std::endl;
	

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
	Stats_partial a = combine_stats(n_p[0], n_p[1], m4_p[0], m4_p[1], m3_p[0], m3_p[1], m2_p[0], m2_p[1], m1_p[0], m1_p[1]);
	Stats_partial b = combine_stats(n_p[2], n_p[3], m4_p[2], m4_p[3], m3_p[2], m3_p[3], m2_p[2], m2_p[3], m1_p[2], m1_p[3]);

	//combine AVX2 halves together
	Stats_partial result_partial = combine_stats(a.n, b.n, a.m4, b.m4, a.m3, b.m3, a.m2, b.m2, a.m1, b.m1);

	//combine AVX2 and single doubles
	Stats_partial result_final = combine_stats(result_partial.n, n, result_partial.m4, m4, result_partial.m3, m3, result_partial.m2, m2, result_partial.m1, m1);

	//set all variables to zero in case this function is called multiple times
	//TODO: move only_ints to combine_stats()
	double temp = this->only_ints;
	this->clear();
	this->only_ints = temp;

	//assign final values
	this->m1 = result_final.m1;
	this->m2 = result_final.m2;
	this->m3 = result_final.m3;
	this->m4 = result_final.m4;
	this->n = result_final.n;

}

void Stats::add_stats(Stats stats_to_add)
{
	Stats_partial result = combine_stats(n, stats_to_add.n, m4, stats_to_add.m4, m3, stats_to_add.m3, m2, stats_to_add.m2, m1, stats_to_add.m1);

	this->m1 = result.m1;
	this->m2 = result.m2;
	this->m3 = result.m3;
	this->m4 = result.m4;
	this->n = result.n;
}

//DEBUG PART

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

double Stats::mean() const
{
	return m1;
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

uint64_t Stats::get_n() {
	return n;
}

double Stats::kurtosis_v()
{
	double* m1_p = (double*)& m1_v;
	double* m2_p = (double*)& m2_v;
	double* m3_p = (double*)& m3_v;
	double* m4_p = (double*)& m4_v;
	double* n_p = (double*)& n_v;
	double n_f = n_p[0] + n_p[1] + n_p[2] + n_p[3];

	Stats_partial a = combine_stats(n_p[0], n_p[1], m4_p[0], m4_p[1], m3_p[0], m3_p[1], m2_p[0], m2_p[1], m1_p[0], m1_p[1]);
	Stats_partial b = combine_stats(n_p[2], n_p[3], m4_p[2], m4_p[3], m3_p[2], m3_p[3], m2_p[2], m2_p[3], m1_p[2], m1_p[3]);

	Stats_partial result = combine_stats(a.n, b.n, a.m4, b.m4, a.m3, b.m3, a.m2, b.m2, a.m1, b.m1);

	return (result.m4 * result.n) / (result.m2 * result.m2) - 3;
}

double Stats::kurtosis_complete()
{
	double* n_p = (double*)& n_v;
	double* m1_p = (double*)& m1_v;
	double* m2_p = (double*)& m2_v;
	double* m3_p = (double*)& m3_v;
	double* m4_p = (double*)& m4_v;

	Stats_partial a = combine_stats(n_p[0], n_p[1], m4_p[0], m4_p[1], m3_p[0], m3_p[1], m2_p[0], m2_p[1], m1_p[0], m1_p[1]);
	Stats_partial b = combine_stats(n_p[2], n_p[3], m4_p[2], m4_p[3], m3_p[2], m3_p[3], m2_p[2], m2_p[3], m1_p[2], m1_p[3]);

	Stats_partial result = combine_stats(a.n, b.n, a.m4, b.m4, a.m3, b.m3, a.m2, b.m2, a.m1, b.m1);

	Stats_partial result_final = combine_stats(result.n, n, result.m4, m4, result.m3, m3, result.m2, m2, result.m1, m1);

	return (result_final.m4 * result_final.n) / (result_final.m2 * result_final.m2) - 3;
}