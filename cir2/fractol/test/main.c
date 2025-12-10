#include <stdlib.h>
#include <stdio.h>
#include "mlx.h"

#define	WIDTH 800
#define	HEIGHT 600

int key_handler(int keycode, void *param)
{
    if (keycode == 65307) // ESC
        exit(0);
    return (0);
}

int	main(void)
{
	unsigned char *data;
	void	*mlx;
	void	*win;
	void	*img;
	int	bpp;
	int	line_len;
	int	endian;
	int	offset;
	int	x;
	int	xi;
	int	y;
	int	yi;
	int	div_y;
	int	div_x;
	int	box_w;
	int	box_h;
	int	box_x;
	int	box_y;
	int	rel_x;
	int	rel_y;

	bpp = 0;
	line_len = 0;
	endian = 0;
	mlx = mlx_init();
	win = mlx_new_window(mlx, WIDTH, HEIGHT, "Test_!");
	img = mlx_new_image(mlx, WIDTH, HEIGHT);
	data = (unsigned char *)mlx_get_data_addr(img, &bpp, &line_len, &endian);
	x = 0;
	y = 0;
	xi = 0;
	yi = 0;
	div_x = 4;
	div_y = 4;
	box_w = WIDTH / div_x;
	box_h = HEIGHT / div_y;
	while (y < HEIGHT)
	{
		if (y == ((HEIGHT - 1) * yi / div_y))
		{
			x = 0;
			while (x < WIDTH)
			{
				offset = y * line_len + x * (bpp / 8);
				data[offset + 0] = 0;
				data[offset + 1] = 0;
				data[offset + 2] = 255;
				x++;
			}
			yi++;
		}
		else
		{
			x = 0;
			xi = 0;
			while (x < WIDTH)
			{
				if (x == ((WIDTH - 1) * xi / div_x))
				{
					offset = y * line_len + x * (bpp / 8);
					data[offset + 0] = 0;
					data[offset + 1] = 255;
					data[offset + 2] = 0;
					xi++;
				}
				/* // 대각선 그리기
				if (x == (int)((double)y * (WIDTH - 1) / (HEIGHT - 1)))
				{
					offset = y * line_len + x * (bpp / 8);
					data[offset + 0] = 0;
					data[offset + 1] = 255;
					data[offset + 2] = 0;
				}
				if (x == (int)((double)(HEIGHT - 1 - y) * (WIDTH - 1) / (HEIGHT - 1)))
				{
					offset = y * line_len + x * (bpp / 8);
					data[offset + 0] = 127;
					data[offset + 1] = 127;
					data[offset + 2] = 127;
				}
				*/
				box_x = x / box_w; //박스 인덱스 즉, 박스의 값을 몇번 더해야하는지 알 수 있음
				box_y = y / box_h;
				rel_x = x - box_x * box_w; // 박스 기준 상대적 위치 즉, 현재위치 - (박스 인덱스 * 박스 크기)
				rel_y = y - box_y * box_h;
				// 주의 점 아래에 있음
				if (rel_x == (int)((double)rel_y * (box_w - 1) / (box_h - 1)))
				{
					offset = y * line_len + x * (bpp / 8);
					data[offset + 0] = 255;
					data[offset + 1] = 0;
					data[offset + 2] = 0;
				}
				if (rel_x == (int)((double)(box_h - 1 - rel_y) * (box_w - 1) / (box_h - 1)))
				{
					offset = y * line_len + x * (bpp / 8);
					data[offset + 0] = 255;
					data[offset + 1] = 0;
					data[offset + 2] = 0;
				}
				x++;
			}
		}
		y++;
	}
	// mlx_put_image_to_window(mlx, win, img, 0, 0); 사용할 경우 이거 사용하면안됨
	// 윈도우에는 항상 마지막에 출력된 내용만 보인다.
	mlx_put_image_to_window(mlx, win, img, 0, 0); 
	mlx_key_hook(win, key_handler, NULL);
	mlx_loop(mlx);
	mlx_destroy_window(mlx, win);
	mlx_destroy_image(mlx, img);
	mlx_destroy_display(mlx);
}
/*
식
rel_x == (int)((double)rel_y * (box_w - 1) / (box_h - 1))
rel_x == (int)((double)rel_y * ((box_w - 1) / (box_h - 1)))

2. 괄호 위치에 따른 연산 순서
식 1
(box_w - 1) / (box_h - 1)
이 부분이 곱셈보다 나중에 계산됩니다.

먼저 (double)rel_y와 (box_w - 1)을 곱함

그 결과를 (box_h - 1)로 나눔

즉,
((double)rel_y * (box_w - 1)) / (box_h - 1)

식 2
((box_w - 1) / (box_h - 1))
이 부분이 곱셈보다 먼저 계산됩니다.

(box_w - 1) / (box_h - 1)을 먼저 계산
(둘 다 int형이므로 결과도 int형) 
>>>> 여기서 int형이기 때문에 소수점 표현 불가 따라서 기울기가 정확히 1, 2, 3,,, 이런식으로만 적용됨


그 결과를 (double)rel_y와 곱함

즉,
(double)rel_y * (int)((box_w - 1) / (box_h - 1))



식 1은
rel_y의 증가에 따라 rel_x가 박스의 가로, 세로 비율에 맞게 변합니다.
즉, 박스의 대각선을 정확하게 그릴 수 있습니다.

식 2는
(box_w - 1) / (box_h - 1)이 정수 나눗셈이기 때문에
대부분의 경우 1 또는 0이 되어버립니다.
즉, 대부분의 박스에서 대각선이 제대로 안 그려집니다.
*/

















