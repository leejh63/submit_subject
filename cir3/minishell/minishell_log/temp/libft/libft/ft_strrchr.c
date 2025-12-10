/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strrchr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <kkeum@student.42gyeongsan.kr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/06 16:09:07 by kkeum             #+#    #+#             */
/*   Updated: 2025/04/06 20:33:32 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strrchr(const char *s, int c)
{
	char	*cursor;

	cursor = (char *)s;
	while (*cursor)
		cursor++;
	while (cursor >= s)
	{
		if (*cursor == (char) c)
			return (cursor);
		cursor--;
	}
	return (0);
}
