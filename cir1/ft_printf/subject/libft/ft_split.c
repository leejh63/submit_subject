/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 15:23:17 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/03 16:33:53 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static void	free_all(char **words, unsigned int ind)
{
	while (ind--)
		free(words[ind]);
	free(words);
}

static unsigned int	count_word(const char *s, char c)
{
	unsigned int	count;
	unsigned int	i;

	count = 0;
	i = 0;
	while (s[i])
	{
		if (s[i] != c)
		{
			count++;
			while (s[i] != c && s[i])
				i++;
		}
		else
			i++;
	}
	return (count);
}

static int	fill_words(char **words, const char *s, char c, unsigned int ind)
{
	unsigned int	i;
	unsigned int	i2;

	i = 0;
	while (s[i])
	{
		if (s[i] != c)
		{
			i2 = i;
			while (s[i2] != c && s[i2])
				i2++;
			words[ind] = malloc(sizeof (char) * (i2 - i + 1));
			if (!words[ind])
			{
				free_all(words, ind);
				return (0);
			}
			ft_strlcpy(words[ind], &s[i], (i2 - i + 1));
			ind++;
			i = i2;
		}
		else
			i++;
	}
	return (1);
}

char	**ft_split(const char *s, char c)
{
	unsigned int	wordcount;
	char			**words;

	if (!s)
		return (NULL);
	wordcount = count_word(s, c);
	words = malloc(sizeof (char *) * (wordcount + 1));
	if (!words)
		return (NULL);
	words[wordcount] = NULL;
	if (!fill_words(words, s, c, 0))
		return (NULL);
	return (words);
}
