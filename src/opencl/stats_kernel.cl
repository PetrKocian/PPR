//equivalent of Stats::push()
inline void push_cl(double *m, size_t *n, double *only_ints, double x)
{
	double delta, delta_n, delta_n2, term1;

	*n += 1;

	delta = x - m[0];

	delta_n = delta / (*n);
	delta_n2 = delta_n * delta_n;
	term1 = delta * delta_n * ((*n) - 1);
	m[0] += delta_n;
	m[3] += term1 * delta_n2 * ((*n) * (*n) - 3 * (*n) + 3) + 6 * delta_n2 * m[1] - 4 * delta_n * m[2];
	m[2] += term1 * delta_n * ((*n) - 2) - 3 * delta_n * m[1];
	m[1] += term1;
	*only_ints += x - floor(x);

}

__kernel void compute_stats(__global const double* numbers, __global double* result)
{
	double m[4] = { 0,0,0,0 };
	double only_ints = 0;
	size_t n = 0;
	size_t id = get_global_id(0);

	//compute stats
	for (size_t i = 0; i < 100; i += 1) {
		push_cl(m, &n, &only_ints, numbers[id*100 + i]);
	}

	//add results to result array
	result[6*id] = n;
	result[6 * id+1] = m[0];
	result[6 * id+2] = m[1];
	result[6 * id+3] = m[2];
	result[6 * id+4] = m[3];
	result[6 * id+5] = only_ints;

}