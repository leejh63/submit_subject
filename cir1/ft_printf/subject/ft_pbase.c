/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_pbase.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/10 11:09:39 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/10 11:10:17 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"
// 대문자 소문자 구별에 플래그는 좋은데 인자로 받는것보다는 enum이나 매직넘버등을 통해서 받는게 좀더 좋아보인다.
// basenum도 굳이 받기보다는 그냥 16, 10을 따로만드는게 좀더 좋을수도 있다 주석따로 만들기도 애매하기 때문
// 
void	ft_pbase(unsigned long long n, int *i, int basenum, int c)
{
	unsigned long long	num;
	int					mod;
	char				numchar;
	char				*base;

	if (c)
		base = "0123456789ABCDEF";
	else
		base = "0123456789abcdef";
	num = n / basenum;
	mod = n % basenum;
	if (num)
		ft_pbase(num, i, basenum, c);
	numchar = base[mod];
	*i += write(1, &numchar, 1);
}
