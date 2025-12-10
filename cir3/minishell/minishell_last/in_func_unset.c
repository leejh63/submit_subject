/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   in_func_unset.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 16:53:44 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/15 16:53:45 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	unset_one_env(t_env *head, t_env *pre, t_env **envs)
{
	if (head == pre)
		(*envs) = head->next;
	else
		pre->next = head->next;
	if (head->val)
		free(head->val);
	if (head->key)
		free(head->key);
	free(head);
	head = NULL;
}

int	in_func_unset(char **argv, t_env **envsf, int out_fd)
{
	int		argc;
	t_env	*head;
	t_env	*pre;
	t_env	**envs;

	envs = &(*envsf)->next;
	argc = argv_len(argv);
	if (argc == 1)
		return (write(out_fd, "", 0), set_q(*envsf, 0), 0);
	while (--argc)
	{
		head = *envs;
		pre = *envs;
		while (head)
		{
			if (ft_strcmp(head->key, argv[argc]) == 0)
			{
				unset_one_env(head, pre, envs);
				break ;
			}
			pre = head;
			head = head->next;
		}
	}
	return (set_q(*envsf, 0), 0);
}
