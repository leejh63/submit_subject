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

#include "Point.hpp"


void answer( Point const a, Point const b, Point const c, Point const point) {
	get_check("a", a);
	get_check("b", b);
	get_check("c", c);
	get_check("point", point);

	if (bsp(a, b, c, point)) {
		std::cout << "The point is inside the triangle!\n";
	}
	else {
		std::cout << "The point isn't inside the triangle!\n";
	}
}

int main( void ) {

    Point a( 0.0f, 0.0f );
    Point c( 4.0f, 0.0f );
    Point b( 0.0f, 4.0f );
    Point p1( 1.1f, 1.1f );
    Point p2( 1.5f, 1.5f );
   
	answer( a, b, c, p1);
	answer( a, b, c, p2);

	return 0;
}
