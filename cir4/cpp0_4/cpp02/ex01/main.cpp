/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/17 11:28:03 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/17 11:28:04 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Fixed.hpp"
#include <iostream>

int main( void ) {

	Fixed a;
	Fixed b( a );
	Fixed c;

	c = b;

	std::cout << a.getRawBits() << std::endl;
	std::cout << b.getRawBits() << std::endl;
	std::cout << c.getRawBits() << std::endl;

	Fixed a1;
	Fixed const b1( 10 );
	Fixed const c1( 42.42f );
	Fixed const d1( b1 );
	a1 = Fixed( 1234.4321f );
	
	std::cout << "a1 is " << a1 << std::endl;
	std::cout << "b1 is " << b1 << std::endl;
	std::cout << "c1 is " << c1 << std::endl;
	std::cout << "d1 is " << d1 << std::endl;
	
	std::cout << "\na is " << a1.toInt() << " as integer" << std::endl;
	std::cout << "b is " << b1.toInt() << " as integer" << std::endl;
	std::cout << "c is " << c1.toInt() << " as integer" << std::endl;
	std::cout << "d is " << d1.toInt() << " as integer" << std::endl;

	std::cout << "\na is " << a1.toFloat() << " as Float" << std::endl;
	std::cout << "b is " << b1.toFloat() << " as Float" << std::endl;
	std::cout << "c is " << c1.toFloat() << " as Float" << std::endl;
	std::cout << "d is " << d1.toFloat() << " as Float\n" << std::endl;

	return 0;
}
