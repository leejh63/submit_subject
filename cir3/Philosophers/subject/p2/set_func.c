/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   set_func.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/24 18:26:54 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/27 22:18:24 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	err_num(int err_info)
{
	if (err_info == e_args)
		return (write(1, "wrong arg\n", 10), err_info);
	if (err_info == e_malloc)
		return (write(1, "malloc fail\n", 12), err_info);
	return (success);
}

int	set_arg_data(t_arg *args_num)
{
	args_num->my_fork = malloc(sizeof(int) * args_num->p_num);
	if (!args_num->my_fork)
		return (err_num(2));
	memset(args_num->my_fork, 0, sizeof(int) * args_num->p_num);
	args_num->die = args_num->die * 1000;
	args_num->eat = args_num->eat * 1000;
	args_num->sleep = args_num->sleep * 1000;
	args_num->think = (args_num->die - args_num->eat - args_num->sleep) / 2;
	args_num->check = 0;
	return (0);
}

int	set_args(int argc, char **argv, t_arg *args_num)
{
	args_num->check = 0;
	if (argc != 5 && argc != 6)
		return (1);
	if (ft_atoi(&args_num->p_num, argv[1]) || args_num->p_num <= 0)
		return (1);
	if (ft_atoi(&args_num->die, argv[2]) || args_num->die <= 0)
		return (1);
	if (ft_atoi(&args_num->eat, argv[3]) || args_num->eat <= 0)
		return (1);
	if (ft_atoi(&args_num->sleep, argv[4]) || args_num->sleep <= 0)
		return (1);
	if (set_arg_data(args_num))
		return (1);
	if (argv[5])
		return (ft_atoi(&args_num->max_eat, argv[5]) || args_num->max_eat <= 0);
	else
		return (ft_atoi(&args_num->max_eat, "-1"));
}

void	set_pval(t_arg *args_num, t_phi *philo, t_key *key, int i)
{
	struct timeval	start;

	gettimeofday(&start, NULL);
	pthread_mutex_init(&philo->l_fork, NULL);
	philo->start = start.tv_sec * 1000000 + start.tv_usec;
	philo->eat_time = 0;
	philo->eat_count = 0;
	philo->ta = args_num;
	philo->num = i + 1;
	philo->m_lock = &key->m_lock;
	philo->q_lock = &key->q_lock;
	philo->lfork = &args_num->my_fork[i];
}

int	set_philos(t_arg *args_num, t_phi **philos, t_key *key)
{
	int	i;

	*philos = malloc(sizeof(t_phi) * args_num->p_num);
	if (!*philos)
		return (free(args_num->my_fork), 1);
	i = -1;
	while (++i < args_num->p_num)
	{
		set_pval(args_num, &(*philos)[i], key, i);
		if (args_num->p_num == 1)
		{
			(*philos)[i].r_fork = NULL;
			(*philos)[i].rfork = NULL;
		}
		else
		{
			(*philos)[i].r_fork = &(*philos)[(i + 1) % args_num->p_num].l_fork;
			(*philos)[i].rfork = &args_num->my_fork[(i + 1) % args_num->p_num];
		}
	}
	return (0);
}
