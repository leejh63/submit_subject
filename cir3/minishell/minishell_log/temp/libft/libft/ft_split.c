/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <kkeum@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 20:13:59 by kkeum             #+#    #+#             */
/*   Updated: 2025/04/07 20:36:59 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	count_tokens(char *s, char c)
{
	int	cnt;
	int	tok;

	cnt = 0;
	tok = 0;
	while (*s)
	{
		if (*s != c && tok == 0)
		{
			tok = 1;
			cnt++;
		}
		else if (*s == c && tok == 1)
			tok = 0;
		s++;
	}
	return (cnt);
}

static char	*prefix(char **s, char c)
{
	int		i;
	char	*pos;
	char	*ptr;

	i = 0;
	pos = *s;
	while (*pos && *pos != c)
	{
		pos++;
		i++;
	}
	ptr = (char *)malloc(i + 1);
	if (!ptr)
		return (0);
	ft_strlcpy(ptr, *s, i + 1);
	while (**s != c && **s)
		(*s)++;
	while (**s == c && **s)
		(*s)++;
	return (ptr);
}

static char	**ft_split_allocate(char const *s, char c, int *cnt, char **head)
{
	char	**ret;

	*cnt = count_tokens((char *)s, c);
	ret = (char **)malloc(sizeof(char *) * (*cnt + 1));
	if (!ret)
		return (0);
	*head = (char *)s;
	while (**head == c && **head)
		(*head)++;
	return (ret);
}

char	**ft_split(char const *s, char c)
{
	int		cnt;
	char	**ret;
	int		i;
	char	*head;

	if (!s)
		return (0);
	ret = ft_split_allocate(s, c, &cnt, &head);
	if (!ret)
		return (0);
	i = 0;
	while (i < cnt)
	{
		ret[i] = prefix(&head, c);
		if (!ret[i])
		{
			while (--i >= 0)
				free(ret[i]);
			free(ret);
			return (0);
		}
		i++;
	}
	ret[cnt] = 0;
	return (ret);
}
/*
#include <stdio.h>

int main(void)
{
	char	*s1 = "asef";
	char	c = ' ';
	char	**split;
	int		i = 0;

	split = ft_split(s1, c);
	while (split[i])
	{
		printf("%s\n", split[i++]);
	}
	return (0);
}*/
