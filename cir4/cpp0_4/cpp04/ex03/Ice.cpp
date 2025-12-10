/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Ice.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/24 13:26:11 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/24 16:40:02 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Ice.hpp"
#include "ICharacter.hpp"
#include <iostream>

void Ice::plog( int num ) const {
	if (skip) { return; }
	std::cout << "Ice's : " <<  msg[num] << "\n";
}

Ice::Ice( void )
: AMateria("ice")
{
	plog(D_ctor);
}

Ice::Ice( const Ice& copy )
: AMateria("ice")
{
	(void) copy;
	plog(C_ctor);
	
}

Ice& Ice::operator=( const Ice& copy ){
	plog(C_op);
	if (this != &copy) {
		AMateria::operator=(copy);
	}
	return *this;
}
Ice::~Ice( void ){
	plog(D_stor);
}
const std::string& Ice::getType( void ) const{
	return this->type;
}
Ice* Ice::clone( void ) const {
	plog(F_clone);
	// I need to check the allocate errors
	Ice *tmp = new Ice(*this);
	return tmp;
}

void Ice::use( ICharacter& target ){
	plog(F_use);
	std::string tmp = target.getName();
	std::cout << "* shoots an ice bolt at " << tmp << " *\n";
	//std::cout << "* heals " << target.getName() << "’s wounds *\n";
}
/*
void Ice::use( const std::string& target ) const {
	plog(F_use);
	std::cout << "* heals " << target << "’s wounds *\n";
}
*/

