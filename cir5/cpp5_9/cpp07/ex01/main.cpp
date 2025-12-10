/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/10 12:05:43 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/12/10 12:05:45 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "iter.hpp"

#include <iostream>

void div_2(int& arg) {
	arg /= 2;
}

template <typename T>
void multi_2(T& arg) {
	arg *= 2;
}

template <typename T>
void test_check(T* array, const T* const_array, std::size_t len) {

    std::cout << "======== non-const ========\n";
    std::cout << "==[before]==\n";
    iter(array, len, print_arg<T>);

    std::cout << "==[add_one]==\n";
    iter(array, len, add_one<T>);

    std::cout << "==[after]==\n";
    iter(array, len, print_arg<T>);

    std::cout << "======multi_2======\n";
    std::cout << "==[before]==\n";
    iter(array, len, print_arg<T>);

    iter(array, len, multi_2<T>);
    
    std::cout << "==[after]==\n";
    iter(array, len, print_arg<T>);

    std::cout << "\n======== const ========\n";
    std::cout << "==[const print]==\n";
    iter(const_array, len, print_arg<const T>);

    std::cout << "==[const_add_one]==\n";
    iter(const_array, len, const_add_one<const T>);

    std::cout << "=========================\n\n";
}

int main( void ) {
	size_t len = 10;
	int int_array[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	const int const_int_array[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	test_check(int_array, const_int_array, len);
	iter(int_array, len, print_arg<int>);
	std::cout << "======div_2======\n";
	iter(int_array, len, div_2);
	iter(int_array, len, print_arg<int>);
	
	float float_array[] = {0.1f, 1.1f, 2.1f, 3.1f, 4.1f, 5.1f, 6.1f, 7.1f, 8.1f, 9.1f};
	const float const_float_array[] = {0.1f, 1.1f, 2.1f, 3.1f, 4.1f, 5.1f, 6.1f, 7.1f, 8.1f, 9.1f};
	test_check(float_array, const_float_array, len);
	return 0;
}

