/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time_func.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/24 17:58:26 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/27 14:58:25 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

long long	now_time_us(void)
{
	struct timeval	now;

	gettimeofday(&now, NULL);
	return (now.tv_sec * 1000000 + now.tv_usec);
}

long long	eat_time_us(t_phi *info_p)
{
	return (now_time_us() - info_p->start - info_p->eat_time);
}

long long	ct_ms(long long start)
{
	return ((now_time_us() - start) / 1000);
}

int	eat_check(t_phi *info_p)
{
	return (eat_time_us(info_p) / 1000 >= info_p->ta->die / 1000);
}

long long	left_time(long long left_val)
{
	return ((left_val > 2000) * (1000) + (left_val <= 2000) * (200));
}
