/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Zombie.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/14 11:50:11 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/14 11:50:12 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Zombie.hpp"
#include <iostream>

Zombie::Zombie( void ) {
	//announce();
}

Zombie::Zombie( const std::string& name ) : name(name) {
	announce();
}

Zombie::~Zombie( void ) {
	std::cout << name << ": delete!\n";
}

void Zombie::announce( void )  const  {
	std::cout << name << ": BraiiiiiiinnnzzzZ...\n";
}

void Zombie::setname( const std::string& newname ) {
	name = newname;
}
