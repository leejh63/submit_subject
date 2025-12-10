/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/17 14:14:46 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/27 15:34:46 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	die_check(t_phi *info_p)
{
	pthread_mutex_lock(info_p->q_lock);
	if (eat_check(info_p))
	{
		if (!info_p->ta->check)
			info_p->ta->check = info_p->num;
		pthread_mutex_unlock(info_p->q_lock);
		return (1);
	}
	pthread_mutex_unlock(info_p->q_lock);
	return (0);
}

void	end_func(t_phi *philo)
{
	while (1)
	{
		pthread_mutex_lock(philo->q_lock);
		if (!philo->ta->p_num)
		{
			pthread_mutex_unlock(philo->q_lock);
			break ;
		}
		else if (!philo->ta->check)
		{
			pthread_mutex_unlock(philo->q_lock);
			usleep(2000);
		}
		else
		{
			philo->ta->die = 0;
			print_timestamp(&philo[philo->ta->check - 1], died);
			pthread_mutex_unlock(philo->q_lock);
			break ;
		}
	}
}

int	main(int argc, char **argv)
{
	t_key	key;
	t_arg	args_num;
	t_phi	*philos;
	int		i;

	if (set_args(argc, argv, &args_num))
		return (err_num(1));
	pthread_mutex_init(&key.m_lock, NULL);
	pthread_mutex_init(&key.q_lock, NULL);
	if (set_philos(&args_num, &philos, &key))
		return (err_num(2));
	i = -1;
	while (++i < args_num.p_num)
		pthread_create(&philos[i].name, NULL, starving, (void *)&philos[i]);
	end_func(philos);
	while (--i >= 0)
		pthread_join(philos[i].name, NULL);
	while (++i < args_num.p_num)
		pthread_mutex_destroy(&philos[i].l_fork);
	pthread_mutex_destroy(&key.m_lock);
	pthread_mutex_destroy(&key.q_lock);
	return (free(philos), 0);
}
