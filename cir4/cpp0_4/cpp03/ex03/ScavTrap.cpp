/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ScavTrap.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/18 16:43:40 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/18 16:43:41 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ScavTrap.hpp"
#include <iostream>

const char* ScavTrap::g_class( void ) const { return "ScavTrap"; }

ScavTrap::ScavTrap( void ) 
: ClapTrap(), _keeper(false)
	{
	log_print( "Default Constructor", "ScavTrap" );
	this->_h_point = 100;
	this->_e_point = 50;
	this->_damage = 20;
}

ScavTrap::ScavTrap( const std::string& name )
: ClapTrap( name ), _keeper(false)
	{
	log_print( "Name Constructor", "ScavTrap" );
	this->_h_point = 100;
	this->_e_point = 50;
	this->_damage = 20;
}

ScavTrap::ScavTrap( const ScavTrap& copy )
: ClapTrap( copy ), _keeper(copy._keeper)
	{
	log_print( "Copy Constructor", "ScavTrap" );
}

ScavTrap& ScavTrap::operator=( const ScavTrap& copy ) {
	log_print( "Assignment Operator", "ScavTrap" );
	if (this != &copy) {
		ClapTrap::operator=(copy);
		this->_keeper = copy._keeper;
	}
	return *this;
}

ScavTrap::~ScavTrap( void ) {
	log_print( "Destructor", "ScavTrap" );
}

void ScavTrap::attack( const std::string& target ){
	log_print( "attack(target)", "ScavTrap" );

	if (this->_h_point == 0) { log_print( "Already Dead", g_class() ); return; }

	if (this->_keeper) {
		std::cout << g_class() << " Serena:" << this->_name
				  << " can't attack in Gate keeper mode!\n";
		return;
	}

	if (this->_e_point == 0) { log_print( "Not enough Energy", g_class() ); return; }

	this->_e_point -= 1;
	std::cout << g_class() << " Serena:" << this->_name
			  << " attacks " << target 
			  << ", causing " << this->_damage 
			  << " points of damage!\n";
}

void ScavTrap::guardGate( void ){
	log_print( "guardGate( void )", "ScavTrap" );

	if (this->_e_point == 0) { log_print( "Not enough Energy", g_class() ); return; }

	this->_keeper = !this->_keeper;

	std::cout << g_class() << " Serena:" << this->_name;
	if (this->_keeper)	{
		std::cout << " is now in Gate keeper mode!\n";
	}
	else {
		std::cout << " is now in normal mode!\n";
	}
}























