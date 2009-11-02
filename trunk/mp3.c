
struct timeval g_tv;

void show_time(void)
{
	if (0 == gettimeofday(&g_tv, NULL))
		printf("Time: %s\n", ctime(&g_tv.tv_sec));
}
