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

    std::cout << "\na is " << a << std::endl;
    std::cout << "++a is " << ++a << std::endl;
    std::cout << "a is " << a << std::endl;
    std::cout << "a++ is " << a++ << std::endl;
    std::cout << "a is " << a << std::endl;
    std::cout << "a-- is " << a-- << std::endl;
    std::cout << "a is " << a << std::endl;
    std::cout << "--a is " << --a << std::endl;
    std::cout << "a is " << a << std::endl;
    std::cout << "--a is " << --a << std::endl;
    std::cout << "a is " << a << std::endl;
    std::cout << "--a is " << --a << std::endl;
    std::cout << "a is " << a << std::endl;

	Fixed a1;
	Fixed const b1( 10 );
	Fixed const c1( 42.42f );
	Fixed d1( b1 );
	a1 = Fixed( 1000.4321f );

	std::cout << "\na1 is " << a1 << std::endl;
	std::cout << "b1 is " << b1 << std::endl;
	std::cout << "c1 is " << c1 << std::endl;
	std::cout << "d1 is " << d1 << std::endl;
    
    std::cout << "\na1 + b1 " << a1 + b1 << std::endl;
    std::cout << "a1 - b1 " << a1 - b1 << std::endl;
    std::cout << "a1 * b1 " << a1 * b1 << std::endl;
    std::cout << "a1 / b1 " << a1 / b1 << std::endl;
    
    std::cout << "\na1 > b1 " << (a1 > b1) << std::endl;
    std::cout << "a1 < b1 " << (a1 < b1) << std::endl;
    std::cout << "a1 >= b1 " << (a1 >= b1) << std::endl;
    std::cout << "a1 <= b1 " << (a1 <= b1) << std::endl;
    std::cout << "a1 == b1 " << (a1 == b1) << std::endl;
    std::cout << "a1 != b1 " << (a1 != b1) << std::endl;
    
	std::cout << "\na1 is " << a1.toInt() << " as integer" << std::endl;
	std::cout << "b1 is " << b1.toInt() << " as integer" << std::endl;
	std::cout << "c1 is " << c1.toInt() << " as integer" << std::endl;
	std::cout << "d1 is " << d1.toInt() << " as integer" << std::endl;
    
	std::cout << "\na1 is " << a1.toFloat() << " as Float" << std::endl;
	std::cout << "b1 is " << b1.toFloat() << " as Float" << std::endl;
	std::cout << "c1 is " << c1.toFloat() << " as Float" << std::endl;
	std::cout << "d1 is " << d1.toFloat() << " as Float\n" << std::endl;
    

    std::cout << "\nmax(a1, d1) is " << Fixed::max(a1, d1) << "\n" << std::endl;
    std::cout << "\nmax(b1, c1) is " << Fixed::max(b1, c1) << "\n" << std::endl;
    std::cout << "\nmax(a1, d1) is " << Fixed::min(a1, d1) << "\n" << std::endl;
    std::cout << "\nmax(b1, c1) is " << Fixed::min(b1, c1) << "\n" << std::endl;
	return 0;
}
