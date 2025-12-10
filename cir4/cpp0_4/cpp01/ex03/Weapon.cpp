/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Weapon.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/14 18:36:06 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/14 18:36:08 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Weapon.hpp"

Weapon::Weapon( const std::string type ) : type(type) {
	//std::cout << "W C\n";
}

Weapon::Weapon( const Weapon& wp ) : type(wp.type) {
    //std::cout << "W CC\n";
}

Weapon::~Weapon( void ) {
	//std::cout << "W ~\n";
}

const std::string& Weapon::getType( void  ) const {
	//std::cout << "W G\n";
	return type;
}

void Weapon::setType( const std::string& newtype ) {
	//std::cout << "W S\n";
	type = newtype;
}

