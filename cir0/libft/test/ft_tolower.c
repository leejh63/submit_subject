/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_tolower.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaeholee <jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/02 10:16:52 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/08 17:45:52 by jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	ft_isupper(int str)
{
	unsigned char	unstr;

	unstr = (unsigned char)str;
	return (unstr <= 'Z' && unstr >= 'A');
}

int	ft_tolower(int str)
{
	if (ft_isupper(str))
		return (str + 32);
	else
		return (str);
}
