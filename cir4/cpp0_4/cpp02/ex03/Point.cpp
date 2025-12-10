/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Point.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/17 17:58:24 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/17 17:58:26 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Point.hpp"

namespace {
	//std::string c_name = "Point";
	//
    bool skip_p = true;
    /*/
    bool skip_p = false;
	//*/
	enum e_print {
		D_CON, C_CON, E2S_CON, F_CON, C_OPR, D_DST,
		X_GET, Y_GET
	};

	void helper_print( int num, const Point* const _this) {
		(void) _this;
		if (skip_p) { return; }
        if (num > 5) { return; }
		const static char* const print_word[] = {
			"\nDefault constructor called\n",
			"\nCopy constructor called\n",
			"\nEnd-Start constructor called\n",
			"\nFloat constructor called\n",
			"\nCopy assignment operator called\n",
			"\nDestructor called\n",					// e_print 1 line end
			"\nget_x function called\n",
			"\nget_y function called\n",				// e_print 2 line end
		};
		std::cout << print_word[num];
		//std::cout << c_name << " "<< _this << " : "<< print_word[num];
	};
}

Point::Point( void )
: _x(0), _y(0)
	{
	::helper_print(D_CON, this);
}

Point::Point( const Point& copy )
: _x(copy._x), _y(copy._y)
	{
	::helper_print(C_CON, this);
}

Point::Point( const Point& end, const Point& start )
: _x(end._x - start._x),
  _y(end._y - start._y)
	{
	::helper_print(E2S_CON, this);
}

Point::Point( const float _x, const float _y )
: _x(_x), _y(_y)
	{
	::helper_print(F_CON, this);
}

Point::~Point( void ) {
	::helper_print(D_DST, this);
}

const Fixed& Point::get_x( void ) const {
	::helper_print(X_GET, this);
	return (*this)._x;
}

const Fixed& Point::get_y( void ) const {
	::helper_print(Y_GET, this);
	return (*this)._y;
}
































