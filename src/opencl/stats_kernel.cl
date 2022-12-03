//combines two Stats passed as an array
inline void add_stats(__local double* m_a, __local double* m_b)
{
	double delta_f = m_b[1] - m_a[1];
	double n_combined = m_a[0] + m_b[0];
	double delta_n_f = delta_f / (n_combined);
	double delta_n2_f = delta_n_f * delta_n_f;
	double delta_n3_f = delta_n_f * delta_n2_f;
	double count_sq = m_a[0] * m_a[0];
	double rn_sq = m_b[0] * m_b[0];

	double m1_combined = (m_a[1] * m_a[0] + m_b[1] * m_b[0]) / (m_a[0] + m_b[0]);

	double m4_combined = m_a[4] + m_b[4] + delta_f * delta_n3_f * m_a[0] * m_b[0] *
		(count_sq - m_a[0] * m_b[0] + rn_sq) + 6 * delta_n2_f * (count_sq * m_b[2] + rn_sq * m_a[2]) +
		4 * delta_n_f * (m_a[0] * m_b[3] - m_b[0] * m_a[3]);

	double m2_combined = m_a[2] + m_b[2] + delta_f * delta_n_f * m_a[0] * m_b[0];

	double m3_combined = m_a[3] + m_b[3] + delta_f * delta_n2_f * m_a[0] * m_b[0] * (m_a[0] - m_b[0]) +
		3 * delta_n_f * (m_a[0] * m_b[2] - m_b[0] * m_a[2]);

	m_a[0] = n_combined;
	m_a[1] = m1_combined;
	m_a[2] = m2_combined;
	m_a[3] = m3_combined;
	m_a[4] = m4_combined;
	m_a[5] += m_b[5];

}

//implementation based on: https://opensource.apple.com/source/Libm/Libm-315/Source/ARM/fpclassify.c.auto.html
//returns 1 for NORMAL and ZERO values, otherwise 0
inline int fpclassify(double x)
{
	unsigned long mask1 = 0x7fffffffffffffff;
	unsigned long mask2 = 0x000fffffffffffff;
	union { double d; ulong u; }u = { x };

	ulong exp = (ulong)((u.u & mask1) >> 52);

	if (0 == exp)
	{
		if (u.u & mask2)
			return 0;

		return 1;
	}

	if (0x7ff == exp)
	{
		return 0;
	}

	return 1;
}

//equivalent of Stats::push()
inline void push_cl(double *m, size_t *n, double *only_ints, double x)
{
	double delta, delta_n, delta_n2, term1;

	if (fpclassify(x) == 1)
	{
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
}

__kernel void compute_stats(__global const double* numbers, __local double* partial_result, __global double* result)
{
	double m[4] = { 0,0,0,0 };
	double only_ints = 0;
	size_t n = 0;
	size_t lid = get_local_id(0);
	size_t gid = get_global_id(0);
	size_t group_size = get_local_size(0);

	//compute stats
	for (size_t i = 0; i < LOOP_SIZE; i += 1) {
		push_cl(m, &n, &only_ints, numbers[gid*LOOP_SIZE + i]);
	}

	//add results to partial_result array
	partial_result[6*lid] = n;
	partial_result[6 * lid+1] = m[0];
	partial_result[6 * lid+2] = m[1];
	partial_result[6 * lid+3] = m[2];
	partial_result[6 * lid+4] = m[3];
	partial_result[6 * lid+5] = only_ints;

	//sync work group
	barrier(CLK_LOCAL_MEM_FENCE);

	//parallel reduce for work group
	for (int i = group_size / 2; i > 0; i >>= 1) {
		if (lid < i) {
			add_stats(partial_result + (6 * lid), partial_result+ (6 * lid + 6 * i));
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	//first workitem in workgroup writes the results of work group to output
	if (get_local_id(0) == 0)
	{
		result[6 * get_group_id(0)] = partial_result[0];
		result[6 * get_group_id(0)+1] = partial_result[1];
		result[6 * get_group_id(0)+2] = partial_result[2];
		result[6 * get_group_id(0)+3] = partial_result[3];
		result[6 * get_group_id(0)+4] = partial_result[4];
		result[6 * get_group_id(0)+5] = partial_result[5];
	}
}