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
	//std::string c_name = "Fixed";
	//
    bool skip_p = true;
    /*/
    bool skip_p = false;
	//*/
	enum e_print {
		D_CON, I_CON, F_CON, C_CON, D_DST, CP_OPR,
		OS_OPR,
        CL_OPR, CR_OPR, CLE_OPR, CRE_OPR, CE_OPR, CD_OPR,
        PU_OPR, MI_OPR, MU_OPR, DI_OPR,
        PRU_OPR, PRD_OPR, POU_OPR, POD_OPR,
        G_VAL, S_VAL,
        I_COV, F_COV,
        MX_SFUNC, CMX_SFUNC, MN_SFUNC, CMN_SFUNC,
	};

	void helper_print( int num, const Fixed* const _this) {
		(void) _this;
		if (skip_p) { return; }
        if (num > 5) { return; }
		const static char* const print_word[] = {
			"Default constructor called\n",
			"Int constructor called\n",
			"Float constructor called\n",
			"Copy constructor called\n",                    
			"Copy assignment operator called\n",
			"Destructor called\n",                          // e_print 1 line end
			"Out Stream operator called\n",                 // e_print 2 line end
            "Compare > operator called\n",
            "Compare < operator called\n",
            "Compare >= operator called\n",
            "Compare <= operator called\n",
            "Compare == operator called\n",
            "Compare != operator called\n",                 // e_print 3 line end
            "Calculate + operator called\n",
            "Calculate - operator called\n",
            "Calculate * operator called\n",
            "Calculate / operator called\n",                // e_print 4 line end
            "++obj operator called\n",
            "--obj operator called\n",
            "obj++ operator called\n",
            "obj-- operator called\n",                      // e_print 5 line end
			"getRawBits member function called\n",
			"setRawBits member function called\n",          // e_print 6 line end
			"toInt member function called\n",
			"toFloat member function called\n",             // e_print 7 line end
            "max static member function called\n",
            "Const max static member function called\n",
            "min static member function called\n",
            "Const min static member function called\n"     // e_print 8 line end
		};
		std::cout << print_word[num];
		//std::cout << c_name << " "<< _this << " : "<< print_word[num];
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

bool Fixed::operator>(const Fixed& right) const{
    ::helper_print(CL_OPR, this);
    return (*this)._val > right._val;
}

bool Fixed::operator<(const Fixed& right) const{
    ::helper_print(CR_OPR, this);
    return (*this)._val < right._val;
}

bool Fixed::operator>=(const Fixed& right) const{
    ::helper_print(CLE_OPR, this);
    return (*this)._val >= right._val;
}

bool Fixed::operator<=(const Fixed& right) const{
    ::helper_print(CRE_OPR, this);
    return (*this)._val <= right._val;
}

bool Fixed::operator==(const Fixed& right) const{
    ::helper_print(CE_OPR, this);
    return (*this)._val == right._val;
}

bool Fixed::operator!=(const Fixed& right) const{
    ::helper_print(CD_OPR, this);
    return (*this)._val != right._val;
}

Fixed Fixed::operator+(const Fixed& right) const {
    ::helper_print(PU_OPR, this);
    Fixed _fixed;
    _fixed.setRawBits((*this)._val + right._val);
    return _fixed;
}

Fixed Fixed::operator-(const Fixed& right) const {
    ::helper_print(MI_OPR, this);
    Fixed _fixed;
    _fixed.setRawBits((*this)._val - right._val);
    return _fixed;
}

Fixed Fixed::operator*(const Fixed& right) const {
    ::helper_print(MU_OPR, this);
    Fixed _fixed;
    long long tmp = (long long)(*this)._val * (long long)right._val;
    _fixed.setRawBits((int)( tmp >> _raw_bit));
    return _fixed;
}

Fixed Fixed::operator/(const Fixed& right) const {
    ::helper_print(DI_OPR, this);
    // Div by zero is not handled in this subject,
	// but it must be handled in another subject.
    Fixed _fixed;
    long long tmp = (long long)(*this)._val * (1 << _raw_bit);
    _fixed.setRawBits((int)(tmp / right._val));
    return _fixed;
}


Fixed& Fixed::operator++( void ) {
    ::helper_print(PRU_OPR, this);
    (*this)._val += 1;
    return *this;
}

Fixed& Fixed::operator--( void ) {
    ::helper_print(PRD_OPR, this);
    (*this)._val -= 1;
    return *this;
}

Fixed Fixed::operator++( int ) {
    ::helper_print(POU_OPR, this);
    Fixed tmp(*this);
    (*this)._val += 1;
    return tmp;
}

Fixed Fixed::operator--( int ) {
    ::helper_print(POD_OPR, this);
    Fixed tmp(*this);
    (*this)._val -= 1;
    return tmp;
}

Fixed& Fixed::max( Fixed& left, Fixed& right) {
    ::helper_print(MX_SFUNC, &left);
    if (left._val >= right._val) { return left; }
    else { return right; }
}
const Fixed& Fixed::max(const Fixed& left, const Fixed& right) {
    ::helper_print(CMX_SFUNC, &left);
    if (left._val >= right._val) { return left; }
    else { return right; }
}
Fixed& Fixed::min( Fixed& left, Fixed& right) {
    ::helper_print(MN_SFUNC, &left);
    if (left._val >= right._val) { return right; }
    else { return left; }
}
const Fixed& Fixed::min(const Fixed& left, const Fixed& right) {
    ::helper_print(CMN_SFUNC, &left);
    if (left._val >= right._val) { return right; }
    else { return left; }
}














































