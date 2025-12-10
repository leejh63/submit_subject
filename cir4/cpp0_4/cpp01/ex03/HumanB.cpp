/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HumanB.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/14 18:36:30 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/14 18:36:32 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HumanB.hpp"

HumanB::HumanB( const std::string name, Weapon& wp ) 
: name(name), wp(&wp) 
	{
	//std::cout << "HB C(n, w)\n";
}

HumanB::HumanB( const std::string name )
: name(name), wp(NULL)
	{
	//std::cout << "HB C(n)\n";
}

HumanB::~HumanB( void ) {
	//std::cout << "HB D\n";
}

void HumanB::attack( void ) const {
	//std::cout << "HB At\n";
	std::string usewp;
	(wp) ? usewp = wp->getType() : usewp = "fist";
	std::cout << name << " attacks with their " << usewp << "\n";
}

void HumanB::setWeapon( Weapon& newwp ) {
	//std::cout << "HB SW(Weapon)\n";
	wp = &newwp;
}





























