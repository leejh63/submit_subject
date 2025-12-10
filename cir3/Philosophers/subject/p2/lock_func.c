/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lock_func.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 12:58:00 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/27 21:59:54 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	get_fork(t_phi *info_p)
{
	while (get_fir_fork(info_p))
	{
		if (die_check(info_p))
			return (1);
		usleep(10);
	}
	if (!info_p->r_fork)
	{
		usleep(info_p->ta->die);
		pthread_mutex_lock(info_p->q_lock);
		info_p->ta->check = info_p->num;
		pthread_mutex_unlock(info_p->q_lock);
		return (1);
	}
	while (get_sec_fork(info_p))
	{
		if (die_check(info_p))
			return (1);
		usleep(10);
	}
	return (0);
}

int	get_fir_fork(t_phi *info_p)
{
	if (info_p->num % 2)
	{
		pthread_mutex_lock(&info_p->l_fork);
		if (*(info_p->lfork))
			return (pthread_mutex_unlock(&info_p->l_fork), 1);
		*(info_p->lfork) = 1;
		pthread_mutex_unlock(&info_p->l_fork);
	}
	else
	{
		pthread_mutex_lock(info_p->r_fork);
		if (*(info_p->rfork))
			return (pthread_mutex_unlock(info_p->r_fork), 1);
		*(info_p->rfork) = 1;
		pthread_mutex_unlock(info_p->r_fork);
	}
	print_timestamp(info_p, t_fork);
	return (0);
}

int	get_sec_fork(t_phi *info_p)
{
	if (die_check(info_p))
		return (1);
	if (info_p->num % 2)
	{
		pthread_mutex_lock(info_p->r_fork);
		if (*(info_p->rfork))
			return (pthread_mutex_unlock(info_p->r_fork), 1);
		*(info_p->rfork) = 1;
		pthread_mutex_unlock(info_p->r_fork);
	}
	else
	{
		pthread_mutex_lock(&info_p->l_fork);
		if (*(info_p->lfork))
			return (pthread_mutex_unlock(&info_p->l_fork), 1);
		*(info_p->lfork) = 1;
		pthread_mutex_unlock(&info_p->l_fork);
	}
	print_timestamp(info_p, t_fork);
	return (die_check(info_p));
}

int	put_down_fork(t_phi *info_p)
{
	if (info_p->r_fork)
	{
		pthread_mutex_lock(info_p->r_fork);
		if (*(info_p->rfork) == 1)
			*(info_p->rfork) = 0;
		pthread_mutex_unlock(info_p->r_fork);
	}
	pthread_mutex_lock(&info_p->l_fork);
	if (*(info_p->lfork) == 1)
		*(info_p->lfork) = 0;
	pthread_mutex_unlock(&info_p->l_fork);
	return (die_check(info_p));
}
