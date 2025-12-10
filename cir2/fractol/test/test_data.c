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
	void			*mlx;
	void			*win;
	void			*img;
	unsigned char	*data;
	int				bpp, line_len, endian;
	int				x, y, offset;
	long			start, end;

	mlx = mlx_init();
	win = mlx_new_window(mlx, WIDTH, HEIGHT, "Buffer Put");
	img = mlx_new_image(mlx, WIDTH, HEIGHT);
	data = (unsigned char *)mlx_get_data_addr(img, &bpp, &line_len, &endian);

	start = get_time_ms();
	y = 0;
	while (y < HEIGHT)
	{
		x = 0;
		while (x < WIDTH)
		{
			offset = y * line_len + x * (bpp / 8);
			data[offset + 0] = 0;   // B
			data[offset + 1] = 0;   // G
			data[offset + 2] = 255; // R
			x++;
		}
		y++;
	}
	end = get_time_ms();

	mlx_put_image_to_window(mlx, win, img, 0, 0);
	printf("buffer version time: %ld ms\n", end - start);
	mlx_loop(mlx);
}

