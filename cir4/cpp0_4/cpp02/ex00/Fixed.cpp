/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Fixed.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/17 11:28:12 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/17 11:28:13 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Fixed.hpp"
#include <iostream>

namespace {
	enum e_print {
		D_CON,
		C_CON,
		C_OPR,
		D_DST,
		G_VAL,
		S_VAL,
	};
}

// static val
const int Fixed::_raw_bit = 8;

// private
void Fixed::helper_print( int num ) const {
	const static char* const print_word[] = {
		"Default constructor called\n",
		"Copy constructor called\n",
		"Copy assignment operator called\n",
		"Destructor called\n",
		"getRawBits member function called\n",
		"setRawBits member function called\n"
	};
	std::cout << print_word[num];
	//std::cout << this << " : "<< print_word[num];
};


// public
Fixed::Fixed( void ) : _val(0) {
	helper_print(D_CON);
}

Fixed::Fixed( const Fixed& copy )
: _val(copy.getRawBits())
	{
	helper_print(C_CON);
}

Fixed& Fixed::operator=( const Fixed& dupli ) {
	helper_print(C_OPR);
	if (this != &dupli) { this->_val = dupli.getRawBits(); }
	return *this;
}

Fixed::~Fixed( void ) {
	helper_print(D_DST);
}

int Fixed::getRawBits( void ) const {
	helper_print(G_VAL);
	return _val;
}

void Fixed::setRawBits( const int new_val ) {
	helper_print(S_VAL);
	_val = new_val;
}
