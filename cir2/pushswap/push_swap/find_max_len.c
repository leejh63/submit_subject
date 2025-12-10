/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   find_max_len.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/01 12:18:45 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/01 13:25:43 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

void	set_table(long long *max_table, long long num)
{
	long long	i;

	i = 0;
	while (max_table[i] != -1)
	{
		max_table[i] = num;
		i++;
	}
}

long long	find_max_last(long long *max_table, long long size)
{
	long long	i;
	long long	max;

	i = 0;
	max = -1;
	while (i < size)
	{
		if (max < max_table[i])
			max = max_table[i];
		i++;
	}
	return (max);
}

long long	insert_max_table(long long *max_table, long long num)
{// 맥스 테이블 끝에 데이터 삽입
	long long	i;

	i = 0;
	while (max_table[i] != -1)
		i++;
	max_table[i] = num;
	return (i);
}

long long	change_max_table(long long *max_table, long long num)
{// 작은 값이 어디로 들어가야하는지 확인후 삽입로직 (인자로 받은 값보다 맥스테이블 값이 클때 삽입) 
	long long	i;

	i = 0;
	while (max_table[i] < num)
		i++;
	max_table[i] = num;
	return (i);
}

void	find_max_len(long long *c_tab, long long *m_tab, long long *i_tab)
{
	long long	i;
	long long	total_size;

	i = 0;
	total_size = table_size(c_tab);
	set_table(m_tab, -1);
	set_table(i_tab, -1);
	m_tab[i] = c_tab[i];
	while (c_tab[i] != -1)
	{
		if (c_tab[i] > find_max_last(m_tab, total_size))
			i_tab[i] = insert_max_table(m_tab, c_tab[i]);
		else
			i_tab[i] = change_max_table(m_tab, c_tab[i]);
		i++;
	}
}
//	구현 계기 "랜덤으로 스택에 저장될경우 이미정렬되어있는애들은 굳이 b스택으로 넘기필요없다"
//		   "다양한 수열들이 존재할 텐데 그중 가장긴 수열을 찾아서 그 겂들은 건들말자!"
// 	LIS 검색
/*   	//   사실 맥스 테이블의 경우 최장 증가 부분수열의 길이를 구하는 로직에서 필요하다.
	//   내 로직에서 필요한것은 최장 증가 부분값들의 위치 값이다. 
	//   처음 구현시 위 알고리즘에 대한 이해가 부족한 상태와 밤을 새며 만들다보니 이런 일이 벌어짐
	//   참고로 이분탐색 혹은 dp를 통해서 진행하는데 나는 그냥 처음부터 위치를 찾아감
	//   왜? 이게 주된 로직이 아니기때문에 그냥 구현함
위 로직은 맥스테이블 갱신시 갱신 될때의 인덱스를 i_tab에 넣어준다 
i_tab내에 담긴 정보는 c_tab인자들이 어디 인덱스에 들어갔는지를 저장한다.
예를 들어서 c_tab > 2 5 3 4 6 7 8 일떄 i_tab > 0 1 1 2 3 4 5
그럼 이제 i_tab을  뒤에서부터 처음 나오는 인덱스를 통해서 최장 증가 부분 수열을 알 수 있다
> 5 처음으로 나오는 값 8, 4이 처음으로 나오는값 7, 3 > 6, 2 > 4, 1 > 3, 0 > 2
즉 2, 3, 4, 6, 7 이 최장 수열이다. 
로직이 이해가 안간다면 인터넷에 겁색 및 하나씩 하다보면 감이 잡힐것이다.
참고, 현재구현된 로직은 두개가 짬뽕된 로직이다보니 이상하기 때문에 검색해보는걸 추천
테이블들은 끝이 -1로 지정되어잇음
c_tab < 현재 수열들의 상태테이블// a스택 초기 값
m_tab < 최장 수열 로직에 의해서 값이 지정되는 테이블
i_tab < 최장 수열의 위치에 대한 테이블
*/
