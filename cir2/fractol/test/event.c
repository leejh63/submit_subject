#include <stdio.h>
#include "mlx.h"
#include <X11/X.h> // X 이벤트 코드 상수

int	event_debugger(void *param)
{
	printf("EVENT CODE: %d\n", (int)(long)param);
	return (0);
}

int main(void)
{
	void *mlx = mlx_init();
	void *win = mlx_new_window(mlx, 800, 600, "Event Debugger");

	// 주요 X 이벤트에 대해 직접 등록
	for (int i = 0; i <= 24; i++) {
		mlx_hook(win, i, 1L << i, event_debugger, (void *)(long)i);
		//      윈도우포인터, 이벤트 코드, 이벤트 마스크, 호출할 함수, 함수에 넘겨줄 인자 
	}

	mlx_loop(mlx);
}
