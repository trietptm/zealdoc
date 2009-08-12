typedef boolean int;

boolean zlib_num_is_pow2(int num)
{
	return ((num & (num -1)) == 0);
}
