/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   do_func.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 14:50:52 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/27 22:16:08 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	do_something(t_phi *info_p, long long do_ing)
{
	long long	check_time;
	long long	left;

	check_time = now_time_us();
	while (now_time_us() - check_time < do_ing)
	{
		if (die_check(info_p))
			return (1);
		left = do_ing - (now_time_us() - check_time);
		usleep(left_time(left));
	}
	if (die_check(info_p))
		return (1);
	return (0);
}

int	eat_ing(t_phi *info_p)
{
	print_timestamp(info_p, eating);
	info_p->eat_time = now_time_us() - info_p->start;
	if (do_something(info_p, info_p->ta->eat))
		return (1);
	info_p->eat_count++;
	if (info_p->ta->max_eat == info_p->eat_count)
	{
		pthread_mutex_lock(info_p->q_lock);
		info_p->ta->p_num--;
		pthread_mutex_unlock(info_p->q_lock);
		return (1);
	}
	return (0);
}

void	*starving(void *philo_info)
{
	t_phi	*info_p;

	info_p = (t_phi *)philo_info;
	if (!(info_p->num % 2))
		usleep(info_p->ta->eat);
	while (1)
	{
		if (die_check(info_p))
			break ;
		if (get_fork(info_p))
			continue ;
		if (eat_ing(info_p))
			break ;
		if (put_down_fork(info_p))
			break ;
		print_timestamp(info_p, sleeping);
		if (do_something(info_p, info_p->ta->sleep))
			break ;
		print_timestamp(info_p, thinking);
		if (do_something(info_p, info_p->ta->think))
			break ;
	}
	put_down_fork(info_p);
	return (NULL);
}
