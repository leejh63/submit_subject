/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/17 14:34:19 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/27 15:12:40 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILO_H
# define PHILO_H

# include <unistd.h>
# include <stdlib.h>
# include <pthread.h>
# include <stdio.h>
# include <sys/time.h>

typedef enum e_state
{
	t_fork,
	eating,
	thinking,
	sleeping,
	died
}	t_state;

typedef enum e_error
{
	success,
	e_args,
	e_malloc
}	t_error;

typedef struct s_arg
{
	long long	die;
	long long	eat;
	long long	sleep;
	long long	think;
	long long	p_num;
	long long	max_eat;
	int			check;
}	t_arg;

typedef struct s_key
{
	pthread_mutex_t	m_lock;
	pthread_mutex_t	q_lock;
}	t_key;

typedef struct s_phi
{
	t_arg			*ta;
	pthread_t		name;
	pthread_mutex_t	l_fork;
	pthread_mutex_t	*r_fork;
	pthread_mutex_t	*m_lock;
	pthread_mutex_t	*q_lock;
	long long		start;
	long long		eat_time;
	int				num;
	int				eat_count;
	int				lfork;
	int				rfork;
}	t_phi;

int			ft_atoi(long long *result, char *asc);

long long	now_time_us(void);
long long	eat_time_us(t_phi *info_p);
long long	ct_ms(long long start);
int			eat_check(t_phi *info_p);
long long	left_time(long long left_val);

void		print_arg(t_arg args_num);
void		print_philo(t_phi *info_philo);
void		print_timestamp(t_phi *info_p, int p_state);

int			err_num(int err_info);
int			set_args(int argc, char **argv, t_arg *args_num);
int			set_philos(t_arg *args_num, t_phi **philos, t_key *key);

int			put_down_fork(t_phi *info_p);
int			get_fork(t_phi *info_p);
int			get_sec_fork(t_phi *info_p);
int			get_fir_fork(t_phi *info_p);

int			die_check(t_phi *info_p);

void		*starving(void *philo_info);
int			eat_ing(t_phi *info_p);
int			do_something(t_phi *info_p, long long do_ing);

#endif
