/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HumanA.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/14 18:36:19 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/14 18:36:20 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HumanA.hpp"

HumanA::HumanA( const std::string name, Weapon& wp )
: name(name), wp(wp) 
	{
	//std::cout << "HA C\n";
}

HumanA::~HumanA( void ) {
	//std::cout << "HA D\n";
}

void HumanA::attack( void ) const {
	//std::cout << "HA AT\n";
	std::cout << name << " attacks with their " << wp.getType() << "\n";
}
void HumanA::setWeapon( const std::string newwp ) {
	//std::cout << "HA SW\n";
	wp.setType(newwp);
}
