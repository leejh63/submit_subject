/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/02 14:52:57 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/02 14:52:58 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
// 일단 작성한 렉서 초기 로직
#include "lexer.h"

size_t	ft_strlen(const char *str)
{
	size_t	i;

	i = 0;
	while (str[i] != '\0')
		i++;
	return (i);
}

void	*ft_memmove(void *dest, const void *src, size_t n)
{
	size_t	i;

	if (!dest && !src)
		return (NULL);
	if (dest < src)
	{
		i = 0;
		while (i < n)
		{
			((unsigned char *)dest)[i] = ((unsigned char *)src)[i];
			i++;
		}
	}
	else
	{
		while (n > 0)
		{
			((unsigned char *)dest)[n - 1] = ((unsigned char *)src)[n - 1];
			n--;
		}
	}
	return (dest);
}

int	change_word(char *test)
{
	int	i;
	int 	start;
	int	s_quo;
	int	b_quo;
	int	slen;

	i = 0;
	s_quo = 0;
	b_quo = 0;
	slen = ft_strlen(test);
	while (test[i])
	{
		if (test[i] == 92 && test[i + 1])
		{
			ft_memmove(&test[i], &test[i + 1], slen - i);
			i += 1;	
			continue ;
		}
		if (test[i] == 34 || test[i] == 39)
		{
			if (test[i] == 34)
				s_quo++;
			else
				b_quo++;
			ft_memmove(&test[i], &test[i + 1], slen - i);
		}
		i++;

	}
	return (b_quo % 2 != 0 || s_quo % 2 != 0);
}
