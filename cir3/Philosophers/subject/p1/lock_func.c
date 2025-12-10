/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lock_func.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 12:58:00 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/27 16:43:20 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	can_get_fork(t_phi *info_p)
{
	long long	left;

	left = info_p->ta->die - (now_time_us() - info_p->eat_time);
	if (left < info_p->ta->eat + 1000)
	{
		put_down_fork(info_p);
		usleep(500);
		return (die_check(info_p));
	}
	return (0);
}

int	get_fork(t_phi *info_p)
{
	if (get_fir_fork(info_p))
		return (1);
	if (!info_p->r_fork)
	{
		usleep(info_p->ta->die);
		pthread_mutex_lock(info_p->q_lock);
		info_p->ta->check = info_p->num;
		pthread_mutex_unlock(info_p->q_lock);
		return (die_check(info_p));
	}
	if (get_sec_fork(info_p))
		return (1);
	return (0);
}

int	get_fir_fork(t_phi *info_p)
{
	if (can_get_fork(info_p))
		return (1);
	if (info_p->num % 2)
	{
		pthread_mutex_lock(&info_p->l_fork);
		info_p->lfork = 1;
	}
	else
	{
		pthread_mutex_lock(info_p->r_fork);
		info_p->rfork = 1;
	}
	print_timestamp(info_p, t_fork);
	return (die_check(info_p));
}

int	get_sec_fork(t_phi *info_p)
{
	if (can_get_fork(info_p))
		return (1);
	if (info_p->num % 2)
	{
		pthread_mutex_lock(info_p->r_fork);
		info_p->rfork = 1;
	}
	else
	{
		pthread_mutex_lock(&info_p->l_fork);
		info_p->lfork = 1;
	}
	print_timestamp(info_p, t_fork);
	return (die_check(info_p));
}

int	put_down_fork(t_phi *info_p)
{
	if (info_p->rfork == 1)
	{
		pthread_mutex_unlock(info_p->r_fork);
		info_p->rfork = 0;
	}
	if (info_p->lfork == 1)
	{
		pthread_mutex_unlock(&info_p->l_fork);
		info_p->lfork = 0;
	}
	return (die_check(info_p));
}
