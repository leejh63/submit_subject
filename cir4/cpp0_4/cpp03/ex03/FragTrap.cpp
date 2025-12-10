/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FragTrap.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/19 19:43:31 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/19 19:43:38 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FragTrap.hpp"
#include <iostream>

const char* FragTrap::g_class( void ) const { return "FragTrap"; }

FragTrap::FragTrap( void )
: ClapTrap()
	{
	log_print( "Default Constructor", "FragTrap" );
	this->_h_point = 100;
	this->_e_point = 100;
	this->_damage = 30;
	std::cout << "gae-gool gae-gool\n";
}

FragTrap::FragTrap( const std::string& name )
: ClapTrap(name)
	{
	log_print( "Name Constructor", "FragTrap" );
	this->_h_point = 100;
	this->_e_point = 100;
	this->_damage = 30;
	std::cout << "gae-gool gae-gool\n";
}

FragTrap::FragTrap( const FragTrap& copy )
: ClapTrap(copy)
	{
	log_print( "Copy Constructor", "FragTrap" );
	std::cout << "gae-gool gae-gool\n";
}

FragTrap& FragTrap::operator=( const FragTrap& copy ) {
	if (this != &copy) {
		ClapTrap::operator=(copy);
	}
	log_print( "Assignment Operator", "FragTrap" );
	std::cout << "gae-gool gae-gool\n";
	return *this;
}

FragTrap::~FragTrap( void ) {
	log_print( "Destructor", "FragTrap" );
	std::cout << "gae-gool gae-gool\n";
}


void FragTrap::attack( const std::string& target ) {
	log_print( "attack(target)", "FragTrap" );
	std::cout << "gae-gool gae-gool\n";

	if (this->_h_point == 0) { log_print( "Already Dead", g_class() ); return; }

	if (this->_e_point == 0) { log_print( "Not enough Energy", g_class() ); return; }

	this->_e_point -= 1;
	std::cout << g_class() << " " << this->_name
			  << " attacks " << target 
			  << ", causing " << this->_damage 
			  << " points of damage!\n";
}

void FragTrap::highFivesGuys( void ) {
	log_print( "highFivesGuys( void )", "FragTrap" );
	std::cout << "gae-gool gae-gool\n";

	if (this->_h_point == 0) { log_print( "Already Dead", g_class() ); return; }

	std::cout << g_class() << " " << this->_name
			  << ": Hey guys! Let's just do a high five!\n";
}
















