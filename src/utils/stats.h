#pragma once

#include <cstdint>
#include <immintrin.h>


struct Stats_partial
{
public:
	double m1 = 0;
	double m2 = 0;
	double m3 = 0;
	double m4 = 0;
	double n = 0;
};

class Stats
{
private:
	uint64_t n;
	double m1, m2, m3, m4, only_ints;
	__m256d increment_v = _mm256_set1_pd(1.0);
	__m256d n_v = _mm256_setzero_pd();
	__m256d n1_v = _mm256_setzero_pd();
	__m256d m1_v = _mm256_setzero_pd();
	__m256d m2_v = _mm256_setzero_pd();
	__m256d m3_v = _mm256_setzero_pd();
	__m256d m4_v = _mm256_setzero_pd();
	__m256d only_ints_v = _mm256_setzero_pd();
	__m256d delta_v = _mm256_setzero_pd();
	__m256d delta_n_v = _mm256_setzero_pd();
	__m256d delta_n2_v = _mm256_setzero_pd();
	__m256d term1_v = _mm256_setzero_pd();
	__m256d term1_temp_v = _mm256_setzero_pd();

	Stats_partial combine_m4(double count_a, double count_b, double m4_a, double m4_b, double m3_a, double m3_b, double m2_a, double m2_b, double m1_a, double m1_b);


public:
	void clear();
	void push_v(__m256d vec_x);
	void push(double x);
	double mean() const;
	double kurtosis() const;
	bool only_integers() const;
	double mean_v();
	uint64_t n_of_v();
	double kurtosis_v();
	double variance_v();
	double skewness_v();
	double kurtosis_complete();
};


