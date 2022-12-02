#include "utils.h"
#include <iostream>
#include <sstream>
#include <queue>

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

void Distribution::push_distribution(distr_type distribution_type)
{
	//increments count according to input
	switch (distribution_type)
	{
	case normal:
		normal_c++;
		break;
	case poisson:
		poisson_c++;
		break;
	case exponential:
		exponential_c++;
		break;
	case uniform:
		uniform_c++;
		break;
	case unknown:
	default:
		unknown_c++;
		break;
	}
}

void Distribution::make_distribution_decision()
{
	std::vector<size_t> sizes;
	sizes.push_back(normal_c);
	sizes.push_back(poisson_c);
	sizes.push_back(exponential_c);
	sizes.push_back(uniform_c);
	sizes.push_back(unknown_c);
	size_t max = 0;
	//find max count
	for (size_t size : sizes)
	{
		if (size > max)
		{
			max = size;
		}
	}
	//decide which distribution max belongs to
	if (max == normal_c)
	{
		final_distribution = normal;
	}
	else if (max == poisson_c)
	{
		final_distribution = poisson;
	}
	else if (max == exponential_c)
	{
		final_distribution = exponential;
	}
	else if (max == uniform_c)
	{
		final_distribution = uniform;
	}
	else
	{
		final_distribution = unknown;
	}

	//finalize instance of class
	max_size = max;
	total = normal_c + poisson_c + exponential_c + uniform_c + unknown_c;
}

void Distribution::print_distribution_decision()
{
	if (total == 0)
	{
		std::cout << "0 chunks computed" << std::endl;
		exit(1);
	}
	//if 70% chunks belong to one distribution, it's probably it
	if (static_cast<double>(max_size)/total > 0.7)
	{
		switch (final_distribution)
		{
		case normal:
			std::cout << std::endl << "Data seem to be normally distributed, " << 100 * normal_c / total << "% are normal chunks" << std::endl;
			break;
		case poisson:
			std::cout << std::endl << "Data seem to be poisson distributed, " << 100 * poisson_c / total << "% are poisson chunks" << std::endl;
			break;
		case exponential:
			std::cout << std::endl << "Data seem to be exponentially distributed, " << 100 * exponential_c / total << "% are exponential chunks" << std::endl;
			break;
		case uniform:
			std::cout << std::endl << "Data seem to be uniformly distributed, " << 100 * uniform_c / total << "% are uniform chunks" << std::endl;
			break;
		case unknown:
		default:
			std::cout << std::endl << "Data don't have a clear distribution, chunks are: " <<std::endl
				<< 100 * normal_c / total << "% normal " << std::endl << 100 * poisson_c / total << "% poisson" << std::endl
				<< 100 * exponential_c / total << "% exponential " << std::endl << 100 * uniform_c / total << "% uniform " << std::endl
				<< 100 * unknown_c / total << "% unknown" << std::endl;
			break;
		}
	}
	//no distribution has over 70% chunks
	else
	{
		std::cout << std::endl << "Data don't have a clear distribution, chunks are: " <<std::endl
			<< 100 * normal_c / total << "% normal "<< std::endl << 100 * poisson_c / total << "% poisson" << std::endl
			<< 100 * exponential_c / total << "% exponential " << std::endl << 100 * uniform_c / total << "% uniform " << std::endl
			<< 100 * unknown_c / total << "% unknown" << std::endl;
	}
}
