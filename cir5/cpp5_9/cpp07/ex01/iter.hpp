/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   iter.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/10 12:05:34 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/12/10 12:05:36 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ITER_HPP
#define ITER_HPP

#include <iostream>

template <typename Tp, typename F>
void iter(Tp* array, const size_t len, F func) {
	for (size_t i = 0; i < len; ++i) {
		func(array[i]);
	}
}

template <typename Tp>
Tp print_arg(Tp& arg) {
	std::cout << "[print_arg]: " << arg << '\n';
	return arg;
}

template <typename Tp>
Tp add_one(Tp& arg) {
	++arg;
	return arg;
}

template <typename Tp>
Tp const_add_one(Tp& arg) {
	Tp const_add_one = arg + 1;
	std::cout << "[const_add_one]: " << const_add_one << '\n';
	return arg;
}

#endif
