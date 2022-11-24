inline void push_cl(double *m, size_t *n, double x)
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

}

__kernel void compute_stats(__global const double* numbers, __global double* result)
{
	double m[4] = {0,0,0,0};
	size_t n = 0;
	size_t id = get_global_id(0);

	for (size_t i = 0; i < 100; i += 1) {
		push_cl(m, &n, numbers[id + i]);
	}

	result[0] = n;
	result[1] = m[0];
	result[2] = m[1];
	result[3] = m[2];
	result[4] = (m[3]*n)/(m[1]*m[1]) - 3;
}



inline double add(double a, double b)
{
	double ab = 1;
	return a + b + 0.5 + ab;
}

__kernel void simple_add(
	__global const double* a,
	__global const double* b,
	__global double* c)
{
	int gid = get_global_id(0);
	c[gid] = add(a[gid], b[gid]);
}
