/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_isprint.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 14:19:00 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/01 14:21:00 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

int	ft_isprint(int str)
{
	unsigned char	unstr;

	if (str < 0 || str > 255)
		return (0);
	unstr = (unsigned char)str;
	return (unstr >= ' ' && unstr <= '~');
}
