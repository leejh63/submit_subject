/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minimap.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/26 14:10:41 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/08/26 14:10:41 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cub3d.h"

// --- 새로 추가/교체할 코드 ---

double	clampd(double v, double lo, double hi)
{
	if (v < lo) return lo;
	if (v > hi) return hi;
	return v;
}

/**
 * ifo 배열 인덱스 의미(기존 스타일 유지)
 * ifo[0], ifo[1] : 현재 그릴 화면 상의 x,y (루프용)
 * ifo[2], ifo[3] : 미니맵 박스의 폭, 높이 (픽셀)
 * ifo[4], ifo[5] : 맵의 폭, 높이 (타일)
 * ifo[6]         : bpp/8 (픽셀당 바이트)
 * ifo[7]         : line_size (한 줄 바이트)
 * ifo[8]         : endian (mlx_get_data_addr 반환용)
 *
 * dfo 배열 인덱스 의미(기존 스타일 유지)
 * dfo[0], dfo[1] : 현재 미니맵 픽셀이 가리키는 맵 좌표(x,y) (타일 단위, 실수)
 * dfo[2], dfo[3] : 픽셀 1칸 이동 시 맵 좌표 증가량 (x,y)
 * dfo[4]         : 플레이어 박스 반폭(타일 단위) 하이라이트용
 */

// 화면 크기를 1/5 박스로 쓰되, "플레이어 중심의 뷰 윈도우"로 매핑
int	fill_map_info_centered(t_param *p, int *ifo, double *dfo, double r_tiles)
{
	double	r_map;
	double	r_box;
	double	view_x0, view_x1;
	double	view_y0, view_y1;

	ifo[0] = 0;
	ifo[1] = 0;

	// 미니맵 박스 크기(픽셀) 결정: 기존과 동일하게 윈도우의 1/5 사용
	r_map = (double)p->map.w / (double)p->map.h;
	r_box = (double)(WIN_W / 5) / (double)(WIN_H / 5);
	if (r_box > r_map)
	{
		ifo[2] = (int)((double)(WIN_H / 5) * r_box);
		ifo[3] = (WIN_H / 5);
	}
	else
	{
		ifo[2] = (WIN_W / 5);
		ifo[3] = (int)((double)(WIN_W / 5) / r_box);
	}

	ifo[4] = p->map.w;  // 맵 크기(타일)
	ifo[5] = p->map.h;

	// 플레이어 중심 뷰 윈도우(타일 단위) 설정 및 가장자리 클리핑
	view_x0 = p->v.xpos - r_tiles;
	view_x1 = p->v.xpos + r_tiles;
	view_y0 = p->v.ypos - r_tiles;
	view_y1 = p->v.ypos + r_tiles;

	/*/ 맵 경계 안으로 잘라내기 (좌표는 [0, w), [0, h)에서 움직이도록)
	view_x0 = clampd(view_x0, 0.0, (double)ifo[4]);
	view_x1 = clampd(view_x1, 0.0, (double)ifo[4]);
	view_y0 = clampd(view_y0, 0.0, (double)ifo[5]);
	view_y1 = clampd(view_y1, 0.0, (double)ifo[5]);
	/*/
	// dfo 세팅: 시작 좌표와 픽셀→타일 스텝
	dfo[0] = view_x0;
	dfo[1] = view_y0;
	dfo[2] = (ifo[2] > 0) ? (view_x1 - view_x0) / (double)ifo[2] : 0.0; // x per pixel
	dfo[3] = (ifo[3] > 0) ? (view_y1 - view_y0) / (double)ifo[3] : 0.0; // y per pixel
	dfo[4] = 0.3; // 플레이어 박스 반폭(타일) 하이라이트용

	return (0);
}

int	get_imgaddr(t_param *p, int *ifo, char **img_addr)
{
	*img_addr = mlx_get_data_addr(p->img, &ifo[6], &ifo[7], &ifo[8]);
	ifo[6] = ifo[6] / 8; // bpp -> bytes per pixel
	return (0);
}

int	map_range(int *ifo, double *dfo)
{
	// dfo[0], dfo[1]가 맵 범위 안인지 체크
	if (0.0 <= dfo[0] && dfo[0] < (double)ifo[4] &&
		0.0 <= dfo[1] && dfo[1] < (double)ifo[5])
		return (1);
	else
		return (0);
}

int	fill_minimap(t_param *p, int *ifo, double *dfo, char *img_addr)
{
	double			pl[4];
	char			**map;
	unsigned int	*minimap;

	map = p->map.map;

	// 플레이어 하이라이트 박스(타일 좌표계에서 비교)
	pl[0] = p->v.xpos - dfo[4];
	pl[1] = p->v.xpos + dfo[4];
	pl[2] = p->v.ypos - dfo[4];
	pl[3] = p->v.ypos + dfo[4];

	// 화면 버퍼에서 현재 픽셀 위치
	minimap = (unsigned int *)(img_addr + ifo[1] * ifo[7] + ifo[0] * ifo[6]);

	// 안전: map_range로 이미 보장되지만, 실수→정수 캐스팅 전 한 번 더 점검해도 OK
	if (!(0.0 <= dfo[0] && dfo[0] < (double)ifo[4] &&
		  0.0 <= dfo[1] && dfo[1] < (double)ifo[5]))
	{
		*minimap = 0x000000;
		return (0);
	}

	// 3) FOV 전체를 빨간색으로 (FOV + 레이 거리 제한)
	{
	    // 플레이어 → 현재 픽셀(타일) 벡터
	    double dx = dfo[0] - p->v.xpos;
	    double dy = dfo[1] - p->v.ypos;
	    double vlen2 = dx*dx + dy*dy;

	    if (vlen2 > 1e-12 && p->dda_count >= WIN_W)  // 거리 버퍼 채워진 프레임만
	    {// Amazing! wow0~2
		// 전방/측면 직교 기저(d̂, p̂) //wow! 0
		double cx = cos(p->v.theta),  cy = sin(p->v.theta);
		double sx = -sin(p->v.theta), sy = cos(p->v.theta);

		// v를 (d̂, p̂)로 분해
		double comp_d = dx*cx + dy*cy;  // 전방 성분
		double comp_p = dx*sx + dy*sy;  // 측면 성분

		if (comp_d > 0.0)  // 앞쪽만
		{
		    // 화면 평면 길이: tan(FOV/2). (#define FOV 90, #define FOV_RAD (FOV*M_PI/180.0))
		    const double plane_len = tan(M_PI_2 * 0.5);

		    // camX = (comp_p/comp_d)/plane_len ∈ [-1,1] 이면 FOV 안 //wow! 1
		    double camX = (fabs(comp_d) > 1e-12) ? (comp_p / comp_d) / plane_len : 999.0;

		    if (camX >= -1.0 && camX <= 1.0)
		    {
		        // 해당 화면 컬럼  //wow! 2!
		        int col = (int)((camX + 1.0) * 0.5 * (double)WIN_W);
		        if (col < 0) col = 0;
		        if (col >= WIN_W) col = WIN_W - 1;

		        // 그 컬럼의 벽까지 레이 거리
		        //double L = (p->dda_dis[col] > 5) ? 5.0 : p->dda_dis[col];
		        double L = p->dda_dis[col];
		        if (L > 0.0 && comp_d <= L + 1e-6)   // 벽 앞까지만 칠함
		        {
		            *minimap = 0xf0f0ff;  // 빨강
		            return (0);
		        }
		    }
		}
	    }
	}

	// 현재 픽셀이 가리키는 맵 타일
	if (pl[0] < dfo[0] && dfo[0] < pl[1] && pl[2] < dfo[1] && dfo[1] < pl[3])
		*minimap = 0xffff00;                 // 플레이어 근처(노란색)
	else if (map[(int)dfo[1]][(int)dfo[0]] == '0')
		*minimap = 0x00ff00;                 // 바닥(초록)
	else if (map[(int)dfo[1]][(int)dfo[0]] == '1')
		*minimap = 0x0000ff;                 // 벽(파랑)
	else if (map[(int)dfo[1]][(int)dfo[0]] == '2')
		*minimap = 0xff00ff;                 // 문 등(보라)
	else if (map[(int)dfo[1]][(int)dfo[0]] == '3')
		*minimap = 0x00ffff;                 // 기타(하늘)
	else
		*minimap = 0x000000;  
	return (0);
}

// 새 메인 함수: 플레이어를 중심으로 r_tiles 범위만 그린다.
int minimap_centered(t_param *p, double r_tiles)
{
	int     ifo[9];
	double  dfo[5];
	char    *img_addr;
	double  view_x0, view_y0;

	get_imgaddr(p, ifo, &img_addr);
	fill_map_info_centered(p, ifo, dfo, r_tiles);

	// 시작 좌표 저장
	view_x0 = dfo[0];
	view_y0 = dfo[1];
	ifo[1] = 0;
	while (ifo[1] < ifo[3])   // y 픽셀
	{
		ifo[0] = 0;
		while (ifo[0] < ifo[2])   // x 픽셀
		{
		// 각 픽셀마다 정확히 대응하는 맵 좌표 계산
			dfo[0] = view_x0 + (double)ifo[0] * dfo[2]; // x
			dfo[1] = view_y0 + (double)ifo[1] * dfo[3]; // y

			//if (map_range(ifo, dfo))
			fill_minimap(p, ifo, dfo, img_addr);
			ifo[0]++;
		}
		ifo[1]++;
	}
	mlx_put_image_to_window(p->mlx, p->win, p->img, 0, 0);
	return (0);
}

/* 사용 예시:
   // 예: 플레이어 기준 좌우/상하 10타일 범위만 미니맵으로 그림
   minimap_centered(&p, 10.0);
*/



/*
int	fill_map_info(t_param *p, int *ifo, double *dfo)
{
	double	r_map;
	double	r_box;

	ifo[0] = 0;
	ifo[1] = 0;
	r_map = (double)p->map.w / (double)p->map.h;
	r_box = (double)(WIN_W / 5) / (double)(WIN_H / 5);
	if (r_box > r_map)
	{
		ifo[2] = (int)(((double)(WIN_H / 5)) * r_map);
		ifo[3] = (WIN_H / 5);
	}
	else
	{
		ifo[2] = (WIN_W / 5);
		ifo[3] = (int)((double)(WIN_W / 5) / r_map);
	}
	ifo[4] = p->map.w;
	ifo[5] = p->map.h;
	dfo[0] = 0.0;
	dfo[1] = 0.0;
	dfo[2] = (double)ifo[4] / (double)ifo[2];
	dfo[3] = (double)ifo[5] / (double)ifo[3];
	dfo[4] = 0.3;
	return (0);
}

int	get_imgaddr(t_param *p, int *ifo, char **img_addr)
{
	*img_addr = mlx_get_data_addr(p->img, &ifo[6], &ifo[7], &ifo[8]);
	ifo[6] = ifo[6] / 8;
	return (0);
}

int	fill_minimap(t_param *p, int *ifo, double *dfo, char *img_addr)
{
	double			pl[4];
	char			**map;
	unsigned int	*minimap;

	map = p->map.map;
	pl[0] = p->v.xpos - dfo[4];
	pl[1] = p->v.xpos + dfo[4];
	pl[2] = p->v.ypos - dfo[4];
	pl[3] = p->v.ypos + dfo[4];
	minimap = (unsigned int *)(img_addr + ifo[1] * ifo[7] + ifo[0] * ifo[6]);
	if (pl[0] < dfo[0] && dfo[0] < pl[1] && pl[2] < dfo[1] && dfo[1] < pl[3])
		*minimap = 0xffff00;
	else if (map[(int)dfo[1]][(int)dfo[0]] == '0')
		*minimap = 0x00ff00;
	else if (map[(int)dfo[1]][(int)dfo[0]] == '1')
		*minimap = 0x0000ff;
	else if (map[(int)dfo[1]][(int)dfo[0]] == '2')
		*minimap = 0xff00ff;
	else if (map[(int)dfo[1]][(int)dfo[0]] == '3')
		*minimap = 0x00ffff;
	return (0);
}

int	map_range(int *ifo, double *dfo)
{
	if (0 <= dfo[0] && dfo[0] < ifo[4] && 0 <= dfo[1] && dfo[1] < ifo[5])
		return (1);
	else
		return (0);
}

int	minimap(t_param	*p)
{
	int		ifo[9];
	double	dfo[5];
	char	*img_addr;

	get_imgaddr(p, ifo, &img_addr);
	fill_map_info(p, ifo, dfo);
	while (ifo[1] < ifo[3])
	{
		ifo[0] = 0;
		dfo[0] = 0.0;
		while (ifo[0] < ifo[2])
		{
			if (map_range(ifo, dfo))
				fill_minimap(p, ifo, dfo, img_addr);
			dfo[0] += dfo[2];
			ifo[0] += 1;
		}
		dfo[1] += dfo[3];
		ifo[1] += 1;
	}
	return (0);
}
*/
