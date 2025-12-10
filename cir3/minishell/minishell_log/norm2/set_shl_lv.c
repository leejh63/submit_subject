/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   set_shl_lv.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 16:57:52 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/15 16:57:53 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	make_shl(t_env **lst)
{
	t_env	*var;
	char	*val;
	char	*key;

	val = ft_strdup("SHLVL");
	if (!val)
		return (perror("set_pwd, ft_strdup"), 1);
	key = ft_itoa(1);
	if (!val)
		return (perror("set_shl malloc"), free(val), 1);
	var = make_env(val, key);
	if (!var)
		return (perror("set_shl malloc"), free(key), free(val), 1);
	ft_env_add_back(lst, var);
	return (0);
}

int	lv_up_shl(t_env **lst, char *val)
{
	int		shlvl;

	shlvl = ft_atoi(val) + 1;
	val = ft_itoa(shlvl * (!(shlvl < 0)));
	if (!val)
		return (perror("set_shl malloc"), 1);
	if (set_env(*lst, "SHLVL", val))
		return (perror("set_shl malloc"), free(val), 1);
	free(val);
	return (0);
}

int	set_shl_lv(t_env **lst)
{
	char	*val;

	val = get_env(*lst, "SHLVL");
	if (!val)
	{
		if (make_shl(lst))
			return (1);
	}
	else
	{
		if (lv_up_shl(lst, val))
			return (1);
	}
	return (0);
}
