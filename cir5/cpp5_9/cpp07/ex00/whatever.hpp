/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   whatever.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/10 10:57:22 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/12/10 11:10:39 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WHATEVER_HPP
#define WHATEVER_HPP

#include <iostream>

template<typename tp>
void swap( tp& first, tp& second ) {
	tp tmp = second;
	second = first;
	first = tmp;
}

template<typename tp>
tp min( const tp& first, const tp& second ) {
	return (first >= second) ? second : first;
}

template<typename tp>
tp max( const tp& first, const tp& second ) {
	return (first > second) ? first : second;
}

template <typename p>
void test_check(p& high, p& low) {
	std::cout << "\n=======test=======\n"
		  << "high  : " << high << '\n'
		  << "low   : " << low << '\n'
		  << "====================\n"
		  << "min   : " << min(high, low) << '\n'
		  << "max   : " << max(high, low) << '\n'
		  << "=======swap=========\n"
		  << "      before        \n"
		  << "high  : " << high << '\n'
		  << "low   : " << low << '\n';
	swap(high, low);
	std::cout << "       after        \n"
		  << "high  : " << high << '\n'
		  << "low   : " << low << '\n'
		  << "=======test end======\n\n";
}

#endif
