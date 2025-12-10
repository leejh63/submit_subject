/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MateriaSource.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/24 19:00:40 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/24 19:00:44 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "AMateria.hpp"
#include "MateriaSource.hpp"
#include <iostream>

void MateriaSource::plog( int num ) const {
	if (skip) { return; }
	std::cout << "MateriaSource's : " <<  msg[num] << "\n";
}

MateriaSource::MateriaSource( void ){
	plog(S_ctor);
    for (int i = 0; i < 4; ++i) { magics[i] = NULL; }
}

MateriaSource::~MateriaSource( void ){
    plog(D_stor);
    for (int i = 0; i < 4; ++i){ if(magics[i]) { delete magics[i]; } }
}

void MateriaSource::learnMateria( AMateria* magic ) {
	plog(F_learnM);
	if (!magic) { plog(N_othing); return; }
	/* Should I create a new object from the pointer 
	   or just use the pointer I received?*/
	for (int i = 0; i < 4; ++i){ if(!this->magics[i]) {  this->magics[i] = magic; return; } }
	plog(N_othing);
}

AMateria* MateriaSource::createMateria( std::string const & type ) {
	plog(F_createM);
	for (int i = 0; i < 4; ++i){ 
		if ((this->magics[i]) && (this->magics[i]->getType() == type)) {
			return this->magics[i]->clone();
		} 
	}
	plog(N_othing);
	return NULL;
}
