/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Fixed.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/17 11:28:09 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/17 11:28:09 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FIXED_HPP
# define FIXED_HPP
# include <string>
# include <iostream>

class Fixed {

private:
	int _val;
	static const int _raw_bit;

public:

	Fixed( void );
	Fixed( const int save_val );
	Fixed( const float save_val );
	Fixed( const Fixed& copy );
	Fixed& operator=( const Fixed& copy );
	~Fixed( void );
	
	int getRawBits( void ) const;
	void setRawBits( const int new_val );
	
	float toFloat( void ) const;
	int toInt( void ) const;
};

std::ostream& operator<<( std::ostream& os, const Fixed& obj);

#endif
