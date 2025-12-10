/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cure.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/24 13:26:11 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/24 17:05:10 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cure.hpp"
#include "ICharacter.hpp"
#include <iostream>

void Cure::plog( int num ) const {
	if (skip) { return; }
	std::cout << "Cure's : " <<  msg[num] << "\n";
}

Cure::Cure( void )
: AMateria("cure")
{
	plog(D_ctor);
}

Cure::Cure( const Cure& copy )
: AMateria("cure")
{
	(void) copy;
	plog(C_ctor);
	
}

Cure& Cure::operator=( const Cure& copy ){
	plog(C_op);
	if (this != &copy) {
		AMateria::operator=(copy);
	}
	return *this;
}

Cure::~Cure( void ){
	plog(D_stor);
}

const std::string& Cure::getType( void ) const{
	return this->type;
}

AMateria* Cure::clone( void ) const {
	plog(F_clone);
	// I need to check the allocate errors
	Cure *tmp = new Cure(*this);
	return tmp;
}

void Cure::use( ICharacter& target ){
	plog(F_use);
	std::string tmp = target.getName();
	std::cout << "* heals " << tmp << "’s wounds *\n";
	//std::cout << "* heals " << target.getName() << "’s wounds *\n";
}
/*
void Cure::use( const std::string& target ) const {
	plog(F_use);
	std::cout << "* heals " << target << "’s wounds *\n";
}
*/

