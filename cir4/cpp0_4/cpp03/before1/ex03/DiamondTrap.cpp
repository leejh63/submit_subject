/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DiamondTrap.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/19 22:07:15 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/19 22:07:16 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "DiamondTrap.hpp"
#include <iostream>

const char* DiamondTrap::g_class( void ) const { return "DiamondTrap"; }

DiamondTrap::DiamondTrap( void )
: ClapTrap("noname_clap_name"),
  FragTrap("noname"),
  ScavTrap("noname"),
  _name("noname")
{
    log_print("Default Constructor", "DiamondTrap");
    _h_point = 100;
    _e_point = 50;
    _damage  = 30;

}

DiamondTrap::DiamondTrap( const std::string& name )
: ClapTrap(name + "_clap_name")
	, FragTrap(name)
	, ScavTrap(name)
	, _name(name)
{
    log_print("Name Constructor", "DiamondTrap");
    _h_point = 100;
    _e_point = 50;
    _damage  = 30;

}

DiamondTrap::DiamondTrap( const DiamondTrap& other)
: ClapTrap(other._name + "_clap_name"),
  FragTrap(other),
  ScavTrap(other),
  _name(other._name)
{
    log_print("Copy Constructor", "DiamondTrap");
    _h_point = other._h_point;
    _e_point = other._e_point;
    _damage  = other._damage;
}

DiamondTrap& DiamondTrap::operator=( const DiamondTrap& other ) {
    if (this != &other) {
    	log_print("Copy Assignment Operator", "DiamondTrap");
        ClapTrap::operator=(other);
        FragTrap::operator=(other);
        ScavTrap::operator=(other);

        _name    = other._name;
        _h_point = other._h_point;
        _e_point = other._e_point;
        _damage  = other._damage;
        ClapTrap::_name = _name + "_clap_name";
    }
    return *this;
}

DiamondTrap::~DiamondTrap( void ) {
    log_print("Destructor", g_class());
}

void DiamondTrap::attack( const std::string& target ) {
    ScavTrap::attack(target);
}

void DiamondTrap::whoAmI( void ) {
    std::cout << g_class() << " "
              << "my name: " << _name
              << ", clap name: " << ClapTrap::_name
              << std::endl;
}


