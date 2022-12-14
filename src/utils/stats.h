#pragma once

#include <cstdint>
#include <immintrin.h>
#include "utils.h"

//helper struct for cases when whole Stats instance isn't needed
struct Stats_partial
{
public:
	double m1 = 0;
	double m2 = 0;
	double m3 = 0;
	double m4 = 0;
	double n = 0;
	double only_ints = 0;
};

class Stats
{
private:
	//variables for computing kurtosis in one pass
	uint64_t n = 0;
	double m1 = 0.0;
	double m2 = 0.0;
	double m3 = 0.0;
	double m4 = 0.0;
	double only_ints = 0.0;;
	//their AVX2 vector equivalents
	__m256d n_v = _mm256_setzero_pd();
	__m256d n1_v = _mm256_setzero_pd();
	__m256d m1_v = _mm256_setzero_pd();
	__m256d m2_v = _mm256_setzero_pd();
	__m256d m3_v = _mm256_setzero_pd();
	__m256d m4_v = _mm256_setzero_pd();
	//helper variables for computation
	__m256d delta_v = _mm256_setzero_pd();
	__m256d delta_n_v = _mm256_setzero_pd();
	__m256d delta_n2_v = _mm256_setzero_pd();
	__m256d term1_v = _mm256_setzero_pd();
	__m256d term1_temp_v = _mm256_setzero_pd();
public:
	//set stats according to input partial stats
	void set_stats(Stats_partial sp);
	//set all values to 0
	void clear();
	//push 4 doubles in as a AVX2 vector
	void push_v(__m256d vec_x);
	//push 1 double in
	void push(double x);
	//return kurtosis
	double kurtosis() const;
	//return if all elements are int
	bool only_integers() const;
	//combine AVX2 vector and double values to finalize stats
	void finalize_stats();
	//adds stats to this stats
	void add_stats(Stats_partial stats_to_add);
	void add_stats(Stats stats_to_add);
	//computes only_ints for x
	void push_only_int(double x);
	//returns mean
	double mean() const;
	//returns number of elements
	uint64_t get_n();
	//returns value of only_ints
	double get_only_ints() const;
	//helper function returns total count of AVX2 stats
	uint64_t n_of_v();
	//returns stats distribution
	int get_distribution_s();
	//returns parameters of distributions
	double get_distr_par(uint16_t distr);

};

//combine a pair of stats
Stats_partial combine_stats(double count_a, double count_b, double m4_a, double m4_b, double m3_a, double m3_b,
	double m2_a, double m2_b, double m1_a, double m1_b, double only_ints_a, double only_ints_b);
