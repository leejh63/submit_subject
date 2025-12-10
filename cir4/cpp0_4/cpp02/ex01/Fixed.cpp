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
#include <cmath>

namespace {
	enum e_print {
		D_CON,
		I_CON,
		F_CON,
		C_CON,
		CP_OPR,
		D_DST,
		OS_OPR,
		G_VAL,
		S_VAL,
		I_COV,
		F_COV,
	};

	void helper_print( int num, const Fixed* const _this) {
		(void) _this;
		if (num > 5) { return; }
		const static char* const print_word[] = {
			"Default constructor called\n",
			"Int constructor called\n",
			"Float constructor called\n",
			"Copy constructor called\n",
			"Copy assignment operator called\n",
			"Destructor called\n",
			"Out Stream assignment operator called\n",
			"getRawBits member function called\n",
			"setRawBits member function called\n",
			"toInt member function called\n",
			"toFloat member function called\n"
		};
		std::cout << print_word[num];
		//std::cout << _this << " : "<< print_word[num];
	};
}

// Overload stream operator
std::ostream& operator<<( std::ostream& os, const Fixed& obj) {
	::helper_print(OS_OPR, &obj);
	os << obj.toFloat();
	return os;
}


// static val
const int Fixed::_raw_bit = 8;

// private


// public
Fixed::Fixed( void )
: _val(0) 
	{
	::helper_print(D_CON, this);
}

Fixed::Fixed( const int save_val )
: _val( save_val << _raw_bit )
	{
	::helper_print(I_CON, this);
}

Fixed::Fixed( const float save_val )
: _val( roundf( save_val * (1 << _raw_bit) ) )
	{
	::helper_print(F_CON, this);
}

Fixed::Fixed( const Fixed& copy )
: _val(copy.getRawBits())
	{
	::helper_print(C_CON, this);
}

Fixed& Fixed::operator=( const Fixed& dupli ) {
	::helper_print(CP_OPR, this);
	if (this != &dupli) { this->_val = dupli.getRawBits(); }
	return *this;
}

Fixed::~Fixed( void ) {
	::helper_print(D_DST, this);
}

int Fixed::getRawBits( void ) const {
	::helper_print(G_VAL, this);
	return _val;
}

void Fixed::setRawBits( const int new_val ) {
	::helper_print(S_VAL, this);
	_val = new_val;
}

float Fixed::toFloat( void ) const {
	::helper_print(F_COV, this);
	return ( this->_val / (float)(1 << _raw_bit) );
}

int Fixed::toInt( void ) const {
	::helper_print(I_COV, this);
	return this->_val >> this->_raw_bit;
}
















































