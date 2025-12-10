/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strncmp.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <kkeum@student.42gyeongsan.kr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/06 16:09:44 by kkeum             #+#    #+#             */
/*   Updated: 2025/04/06 16:09:58 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	ft_strncmp(const char *s1, const char *s2, size_t n)
{
	size_t	i;
	int		ret;

	i = 0;
	ret = 0;
	while (i < n)
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
