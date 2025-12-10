/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 12:17:59 by kkeum             #+#    #+#             */
/*   Updated: 2025/07/16 12:18:00 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

char	*ft_strndup(const char *str, size_t n)
{
	char	*ptr;

	ptr = (char *)malloc(n + 1);
	if (ptr == NULL)
		return (NULL);
	ptr = ft_memcpy(ptr, str, n);
	ptr[n] = '\0';
	return (ptr);
}

int	is_whitespace(char c)
{
	if (c == 9 || c == 10 || c == 13 || c == 32)
		return (1);
	return (0);
}

int	is_general_char(char c, int in_quote)
{
	const char	*str = " \t\n\r\"'|<>$";
	const char	*str2 = " \t\n\r\"$";

	if (in_quote == 1)
	{
		if (c == '\'' || c == '\0')
			return (0);
		else
			return (1);
	}
	else if (in_quote == 2)
	{
		if (ft_strchr(str2, c))
			return (0);
		else
			return (1);
	}
	else if (ft_strchr(str, c))
		return (0);
	else
		return (1);
}

int	ft_strcmp(const char *s1, const char *s2)
{
	size_t	i;
	int		ret;

	i = 0;
	ret = 0;
	while (1)
	{
		ret = (unsigned char)s1[i] - (unsigned char)s2[i];
		if (s1[i] == 0 || s2[i] == 0)
			return (ret);
		if (ret)
			return (ret);
		i++;
	}
	return (0);
}

void	ft_perror(char *s1, char *s2, char *s3, int strerr)
{
	ft_putstr_fd("minishell: ", 2);
	if (s1)
		ft_putstr_fd(s1, 2);
	if (s2)
		ft_putstr_fd(s2, 2);
	if (s3)
		ft_putstr_fd(s3, 2);
	if (strerr)
		perror("");
	else
		ft_putstr_fd("\n", 2);
}
