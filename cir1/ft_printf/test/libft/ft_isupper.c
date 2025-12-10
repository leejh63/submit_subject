/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_isupper.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 12:56:29 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/01 13:02:52 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

int	ft_isupper(int str)
{
	unsigned char	unstr;

	unstr = (unsigned char)str;
	return (unstr <= 'Z' && unstr >= 'A');
}
