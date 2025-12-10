/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Point.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/17 17:58:13 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/17 17:58:21 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef POINT_HPP
# define POINT_HPP
# include "Fixed.hpp"


class Point {
private:
	Fixed const _x;
	Fixed const _y;
	
	Point& operator=( const Point& copy );
public:
	Point( void );
	Point( const Point& copy );
	Point( const Point& end, const Point& start );
	Point( const float _x, const float _y );
	~Point( void );

	const Fixed& get_x( void ) const ;
	const Fixed& get_y( void ) const ;
};

void get_check( const std::string name, const Point& target );
bool bsp( Point const a, Point const b, Point const c, Point const point);

#endif
