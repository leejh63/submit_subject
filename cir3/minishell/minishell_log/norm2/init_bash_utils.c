/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_bash_utils.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 17:00:14 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/15 17:00:15 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	ft_env_add_back(t_env **lst, t_env *new_node)
{
	t_env	*current;

	if (!*lst)
	{
		*lst = new_node;
		return ;
	}
	current = *lst;
	while (current->next)
		current = current->next;
	current->next = new_node;
}

void	set_q(t_env *lst, int i)
{
	char	*val;
	int		divisor;

	val = get_env(lst, "?");
	if (!val)
		return ;
	i = i & 0xff;
	divisor = 1;
	while (i / divisor >= 10)
		divisor *= 10;
	while (divisor)
	{
		*val = i / divisor + '0';
		i = i % divisor;
		divisor = divisor / 10;
		val++;
	}
	*val = '\0';
}

int	get_q(t_env *lst)
{
	return (ft_atoi(get_env(lst, "?")));
}
