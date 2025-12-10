/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AMateria.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/24 10:41:53 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/24 16:38:32 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "AMateria.hpp"
#include "ICharacter.hpp"
#include <iostream>

//
bool skip = true;
/*/
bool skip = false;
//*/

const char* const msg[] = {
	"Default Constructor",
	"String Constructor",
	"Copy Constructor",
	"Destructor",
	"Copy Operator",
	"getType",
	"use",
	"clone",
	"equip",
	"unequip",
	"getName",
	"learnMateria",
	"createMateria",
	"*Nothing happened*"
};

void AMateria::plog( int num ) const {
	if (skip) { return; }
	std::cout << "AMateria's : " <<  msg[num] << "\n";
}

AMateria::AMateria( const std::string& type )
: type(type)
{
	plog(S_ctor);
}

AMateria::AMateria( const AMateria& copy )
: type(copy.type)
{
	plog(C_ctor);
}

AMateria& AMateria::operator=( const AMateria& copy ) {
/* 
The only reason to define this function is to let 
derived classes use the base class’s copy assignment operator. 
*/
	(void) copy;
	plog(C_op);
	return *this;
}

AMateria::~AMateria( void ){
	plog(D_stor);
}

const std::string& AMateria::getType( void ) const{
	plog(F_gtype);
	return type;
}
void AMateria::use( ICharacter& target ){
	plog(F_use);
	std::string tmp = target.getName();
	std::cout << tmp << " was confused because nothing happened.\n";
	//std::cout << target.getName() << " was confused because nothing happened.\n";
}

/*
void AMateria::use( const std::string& target ) const {
	plog(F_use);
	std::cout << target << " was confused because nothing happened.\n";
}
*/
