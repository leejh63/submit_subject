/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bsp.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/17 18:38:44 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/17 18:38:45 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Point.hpp"

namespace {
	//
    bool skip_p = true;
    /*/
    bool skip_p = false;
	//*/
}

void get_check( const std::string name, const Point& target ) {
	if (skip_p) { return; }
	std::cout << "\n" << name << " get x :" << target.get_x()
			  << "\n" << name << " get y :" << target.get_y()
			  << "\n";
};

Fixed side_check(const Point& line, const Point& point) {
	//std::cout << "cross1 > cross2 " << (cross1 < cross2) << "\n";
	return (point.get_y() * line.get_x()) - (point.get_x() * line.get_y());
}

bool bsp( Point const a, Point const b, Point const c, Point const point) {
	Point ab(b, a), bc(c, b), ca(a, c);
	Point ap(point, a), bp(point, b), cp(point, c);
	
	Fixed check1(side_check(ab, ap)), check2(side_check(bc, bp)), check3(side_check(ca, cp));
	if (check1 > 0 && check2 > 0 && check3 > 0)
		return true;
	if (check1 < 0 && check2 < 0 && check3 < 0)
		return true;
	return false;
}
