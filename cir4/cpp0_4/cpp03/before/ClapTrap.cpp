/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClapTrap.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/18 12:56:12 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/18 12:56:13 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ClapTrap.hpp"
#include <iostream>

namespace {
	std::string c_name = "ClapTrap";
	/*/
    bool skip_p = true;
    /*/
    bool skip_p = false;
	//*/
	enum e_print {
		D_CON, S_CON, C_CON, C_OPR, D_DST,
		F_ATK, F_TDG, F_RPR
	};
	
	enum e_state {
		NOT_E, A_DIE, O_RIP,
	};

	void helper_print( int num, const ClapTrap* const _this) {
        if (skip_p) { return; }
		const static char* const print_word[] = {
			"\nDefault constructor called\n",
			"\nstring constructor called\n",
			"\nCopy constructor called\n",
			"\nCopy assignment operator called\n",
			"\nDestructor called\n",					// e_print 1 line end
			"\nattack function called\n",
			"\ntakeDamage function called\n",
			"\nbeRepaired function called\n"			// e_print 2 line end
		};

		std::cout << c_name << " "<< _this << " : "<< print_word[num];
	};
}

ClapTrap::ClapTrap( void )
: _name("unknown"), _h_point(10), _e_point(10), _damage(0)
	{
	helper_print( D_CON, this );
}

ClapTrap::ClapTrap( const std::string name )
: _name(name), _h_point(10), _e_point(10), _damage(0)
	{
	helper_print( S_CON, this );
}

ClapTrap::ClapTrap( const ClapTrap& copy )
:	_name(copy._name), _h_point(copy._h_point),
	_e_point(copy._e_point), _damage(copy._damage)
	{
	helper_print( C_CON, this );
}

ClapTrap& ClapTrap::operator=( const ClapTrap& copy ) {
	helper_print( C_OPR, this );
	if (this != &copy) {
		this->_name = copy._name;
		this->_h_point = copy._h_point;
		this->_e_point = copy._e_point;
		this->_damage = copy._damage;
	}
	return *this;
}

ClapTrap::~ClapTrap( void ) {
	helper_print( D_DST, this );
}

void ClapTrap::state_print( int num ) const {
	const static char* const print_word[] = {
			"Not enough Energy!\n",
			"Already Die!\n",
			"Break down!\n"
			
		};
	std::cout << this->_name << " : " << print_word[num];
}

void ClapTrap::attack( const std::string& target ) {
	helper_print( F_ATK, this );
	if (this->_h_point == 0) { return this->state_print(A_DIE); }
	if (this->_e_point == 0) { return this->state_print(NOT_E); }
	this->_e_point -= 1;
	std::cout << "ClapTrap " << this->_name
			  << " attacks " << target 
			  << ", causing " << this->_damage 
			  << " points of damage!\n";
	
}

void ClapTrap::takeDamage( unsigned int amount ) {
	helper_print( F_TDG, this );

	if (this->_h_point == 0) { return this->state_print(A_DIE); }

	if (this->_h_point <= amount) { 
		this->_h_point = 0;
		return this->state_print(O_RIP); 
	}

	this->_h_point -= amount;
	std::cout << "ClapTrap " << this->_name
			  << " loses " << amount << " hp point!, "
			  << "hp is " << this->_h_point << " left\n";
}

void ClapTrap::beRepaired( unsigned int amount ) {
	helper_print( F_RPR, this );
	if (this->_h_point == 0) { return this->state_print(A_DIE); }
	if (this->_e_point == 0) { return this->state_print(NOT_E); }
	this->_e_point -= 1;
	this->_h_point += amount;
	std::cout << "ClapTrap " << this->_name
			  << " repairs " << amount << " points! "
			  << "hp is " << this->_h_point << " left\n";
}





























