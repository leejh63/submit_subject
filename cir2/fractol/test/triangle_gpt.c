#include <mlx.h>
#include <math.h>
#include <stdlib.h>

typedef struct s_mlx {
    void *mlx;
    void *win;
    int width;
    int height;
} t_mlx;

// 선을 그리는 함수 (Bresenham 알고리즘, 단순화)
void draw_line(t_mlx *mlx, int x1, int y1, int x2, int y2, int color)
{
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;

    while (1) {
        mlx_pixel_put(mlx->mlx, mlx->win, x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        int e2 = err;
        if (e2 > -dx) { err -= dy; x1 += sx; }
        if (e2 <  dy) { err += dx; y1 += sy; }
    }
}

// 세 점을 잇는 삼각형을 그리는 함수
void draw_triangle(t_mlx *mlx, int x1, int y1, int x2, int y2, int x3, int y3, int color)
{
    draw_line(mlx, x1, y1, x2, y2, color);
    draw_line(mlx, x2, y2, x3, y3, color);
    draw_line(mlx, x3, y3, x1, y1, color);
}

// 시에르핀스키 삼각형 재귀 함수
void sierpinski(t_mlx *mlx, int x1, int y1, int x2, int y2, int x3, int y3, int depth)
{
    if (depth <= 0) {
        draw_triangle(mlx, x1, y1, x2, y2, x3, y3, 0xFFFFFF); // 흰색
        return;
    }
    // 중점 계산
    int mx12 = (x1 + x2) / 2;
    int my12 = (y1 + y2) / 2;
    int mx23 = (x2 + x3) / 2;
    int my23 = (y2 + y3) / 2;
    int mx31 = (x3 + x1) / 2;
    int my31 = (y3 + y1) / 2;

    // 3개의 작은 삼각형에 대해 재귀 호출
    sierpinski(mlx, x1, y1, mx12, my12, mx31, my31, depth - 1);
    sierpinski(mlx, x2, y2, mx23, my23, mx12, my12, depth - 1);
    sierpinski(mlx, x3, y3, mx31, my31, mx23, my23, depth - 1);
}

int main()
{
    t_mlx mlx;
    mlx.mlx = mlx_init();
    mlx.width = 800;
    mlx.height = 600;
    mlx.win = mlx_new_window(mlx.mlx, mlx.width, mlx.height, "Sierpinski Triangle");

    // 삼각형 세 점 (예시)
    int x1 = mlx.width / 2, y1 = 50;
    int x2 = 100, y2 = mlx.height - 50;
    int x3 = mlx.width - 100, y3 = mlx.height - 50;

    sierpinski(&mlx, x1, y1, x2, y2, x3, y3, 20); // depth=5

    mlx_loop(mlx.mlx);
    return 0;
}
