/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   word.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 17:50:42 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/15 17:50:59 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

char	**word_to_str(t_word *words)
{
	int		i;
	char	**ret;
	t_word	*next;

	ret = calloc(sizeof(char *), word_lstsize(words));
	if (!ret)
		return (perror("word, calloc"), NULL);
	i = 0;
	while (words)
	{
		next = words->next;
		ret[i] = words->str;
		i++;
		free(words);
		words = next;
	}
	return (ret);
}

int	word_lstsize(t_word *words)
{
	int	i;

	i = 0;
	if (words == NULL)
		return (0);
	while (words)
	{
		i++;
		words = words->next;
	}
	return (i);
}

int	word_append(t_word *cur, char *str)
{
	char	*joined;

	joined = ft_strjoin(cur->str, str);
	if (joined == NULL)
		return (1);
	free(cur->str);
	cur->str = joined;
	return (0);
}

void	word_next(t_word **words, t_word **cur)
{
	if (*cur == NULL)
	{
		*cur = calloc(sizeof(t_word), 1);
		*words = *cur;
	}
	else if ((*cur)->str == NULL)
		return ;
	else
	{
		(*cur)->next = calloc(sizeof(t_word), 1);
		*cur = (*cur)->next;
	}
}
