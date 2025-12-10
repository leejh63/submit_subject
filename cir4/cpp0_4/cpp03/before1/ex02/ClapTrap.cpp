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

/*/
bool ClapTrap::skip_p = true;
/*/
bool ClapTrap::skip_p = false;
//*/

const char* ClapTrap::g_class( void ) const { return "ClapTrap"; }

void ClapTrap::log_print( const std::string& msg, const std::string& _class ) const {
	if(skip_p) { return; }
	std::cout << "\n[" << _class << "] " << msg << "\n";
}

ClapTrap::ClapTrap( void )
: _name("unknown"), _h_point(10), _e_point(10), _damage(0)
	{
	log_print( "Default Constructor", "ClapTrap" );
}

ClapTrap::ClapTrap( const std::string& name )
: _name(name), _h_point(10), _e_point(10), _damage(0)
	{
	log_print( "Name Constructor", "ClapTrap" );
}

ClapTrap::ClapTrap( const ClapTrap& copy )
:	_name(copy._name), _h_point(copy._h_point),
	_e_point(copy._e_point), _damage(copy._damage)
	{
	log_print( "Copy Constructor", "ClapTrap" );
}

ClapTrap& ClapTrap::operator=( const ClapTrap& copy ) {
	log_print( "Assignment Operator", "ClapTrap" );
	if (this != &copy) {
		this->_name = copy._name;
		this->_h_point = copy._h_point;
		this->_e_point = copy._e_point;
		this->_damage = copy._damage;
	}
	return *this;
}

ClapTrap::~ClapTrap( void ) {
	log_print( "Destructor", "ClapTrap" );
}

void ClapTrap::attack( const std::string& target ) {
	log_print( "attack(target)", g_class() );

	if (this->_h_point == 0) { log_print( "Already Dead", g_class() ); return; }

	if (this->_e_point == 0) { log_print( "Not enough Energy", g_class() ); return; }

	this->_e_point -= 1;
	std::cout << g_class() << " " << this->_name
			  << " attacks " << target 
			  << ", causing " << this->_damage 
			  << " points of damage!\n";
}

void ClapTrap::takeDamage( unsigned int amount ) {
	log_print( "takeDamage(amount)", g_class() );

	if (this->_h_point == 0) { log_print( "Already Dead", g_class() ); return; }

    _h_point = (amount >= _h_point) ? 0 : (_h_point - amount);

    std::cout << g_class() << " " << _name
              << " loses " << amount << " hp, "
              << "hp is " << _h_point << " left";

    if (_h_point == 0)
        std::cout << "\nOh! " << g_class() << " " << _name << " has died";
    
    std::cout << '\n';
}

void ClapTrap::beRepaired( unsigned int amount ) {
	log_print( "beRepaired(amount)", g_class() );
	
	if (this->_h_point == 0) { log_print( "Already Dead", g_class() ); return; }

	if (this->_e_point == 0) { log_print( "Not enough Energy", g_class() ); return; }
	
	this->_e_point -= 1;
	this->_h_point += amount;
	std::cout << g_class() << " " << this->_name
			  << " repairs " << amount << " points! "
			  << "hp is " << this->_h_point << " left\n";
}





























