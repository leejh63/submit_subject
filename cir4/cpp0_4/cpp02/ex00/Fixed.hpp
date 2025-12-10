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

class Fixed {

private:
	int _val;
	static const int _raw_bit;

	void helper_print( int num ) const;
public:

	Fixed( void );
	Fixed( const Fixed& copy );
	Fixed& operator=( const Fixed& copy );
	~Fixed( void );
	
	int getRawBits( void ) const;
	void setRawBits( const int new_val );
};

#endif
