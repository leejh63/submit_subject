/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print_func.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/24 17:59:07 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/27 20:38:58 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

void	print_timestamp(t_phi *info_p, int p_state)
{
	pthread_mutex_lock(info_p->m_lock);
	if (p_state == t_fork)
		printf("%lld %d has taken a fork\n", ct_ms(info_p->start), info_p->num);
	else if (p_state == eating)
		printf("%lld %d is eating\n", ct_ms(info_p->start), info_p->num);
	else if (p_state == thinking)
		printf("%lld %d is thinking\n", ct_ms(info_p->start), info_p->num);
	else if (p_state == sleeping)
		printf("%lld %d is sleeping\n", ct_ms(info_p->start), info_p->num);
	else if (p_state == died)
		printf("%lld %d died\n", ct_ms(info_p->start), info_p->num);
	pthread_mutex_unlock(info_p->m_lock);
}
