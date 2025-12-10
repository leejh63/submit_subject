#include "mlx.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>

#define WIDTH 800
#define HEIGHT 600

static long	get_time_ms(void)
{
	struct timeval	tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

int main(void)
{
	void	*mlx;
	void	*win;
	int		x, y;
	long	start, end;

	mlx = mlx_init();
	win = mlx_new_window(mlx, WIDTH, HEIGHT, "Pixel Put");

	start = get_time_ms();
	y = 0;
	while (y < HEIGHT)
	{
		x = 0;
		while (x < WIDTH)
		{
			mlx_pixel_put(mlx, win, x, y, 0x00FF0000); // Red
			x++;
		}
		y++;
	}
	end = get_time_ms();

	printf("mlx_pixel_put version time: %ld ms\n", end - start);
	mlx_loop(mlx);
}

