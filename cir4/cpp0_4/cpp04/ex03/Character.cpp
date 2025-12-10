#include "Character.hpp"
#include "AMateria.hpp"
#include <iostream>

void Character::plog( int num ) const {
	if (skip) { return; }
	std::cout << "Character's : " <<  msg[num] << "\n";
}

Character::Character( const std::string& name )
: _name(name)
{
    plog(S_ctor);
    for (int i = 0; i < 4; ++i) { slot[i] = NULL; }
}

Character::Character( const Character& copy )
: _name(copy._name)
{
    plog(C_ctor);
    for (int i = 0; i < 4; ++i) {
        this->slot[i] = (copy.slot[i]) ? copy.slot[i]->clone() : NULL;
    }
}

Character& Character::operator=( const Character& copy ) {
    plog(C_op);
    this->_name = copy._name;
    for (int i = 0; i < 4; ++i) {
        if (this->slot[i]) { delete this->slot[i]; }
        this->slot[i] = (copy.slot[i]) ? copy.slot[i]->clone() : NULL;
    }
    return *this;
}

Character::~Character( void ) {
    plog(D_stor);
    for (int i = 0; i < 4; ++i){ if(slot[i]) { delete slot[i]; } }
}

std::string const & Character::getName( void ) const{
    plog(F_getName);
    return this->_name;
}

void Character::equip( AMateria* m ){
    plog(F_eqp);
    if (!m) { plog(N_othing); return; }
    for (int i = 0; i < 4; ++i){
        if(!slot[i]) { slot[i] = m; return; }
    }
    std::cout << "you should delete the AMateria\n";
    plog(N_othing);
}

void Character::unequip( int idx ){
    plog(F_ueqp);
    if (0 > idx || 3 < idx) { plog(N_othing); return; }
    if (this->slot[idx]) {
        std::cout << "*What should I do? I don't have any ideas..*\n";
        //tmp
        delete this->slot[idx];
        this->slot[idx] = NULL;
        return;
    }
    else { plog(N_othing); return; }
}

void Character::use( int idx, ICharacter& target ){
    plog(F_use);
    if (0 > idx || 3 < idx) { plog(N_othing); return; }
    if (this->slot[idx]) { this->slot[idx]->use(target); }
    else { plog(N_othing); return; }
}

