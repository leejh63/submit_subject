#include "WrongDog.hpp"

namespace {
	//
	bool skip = false;
	/*/
	bool skip = true;
	//*/
}

void WrongDog::p_wdog( int num ) const {
	const static char* script[] = {
		"[w] Oh! What a lovely cute dog!",
		"[w] Double the cuteness! and Double the trouble!",
		"[w] Same dog. Totally same. Can't believe it? Me neither!",
		"[w] It is not a joke this is same dog! Totally same!",
		"[w] Dog is too old...",
	};
	if (skip) { return; }
	std::cout << script[num] << "\n";
}

const std::string& WrongDog::getType( void ) const { return this->type; }

WrongDog::WrongDog( void )
:   WrongAnimal("WrongDog")
    {
        p_log("Default Constructor", "WrongDog");
        p_wdog(0);
}

WrongDog::WrongDog( const WrongDog& copy )
:   WrongAnimal(copy.type)
    {
        p_log("Copy Constructor", "WrongDog");
        p_wdog(1);
}

WrongDog& WrongDog::operator=( const WrongDog& copy ) {
    p_log("Assign Constructor", "WrongDog");
    if (this != &copy) {
        WrongAnimal::operator=(copy);
        p_wdog(2);
    }
    else {
        p_wdog(3);
    }
    return *this;
}

WrongDog::~WrongDog( void ) {
    p_log("Destructor", "WrongDog");
    p_wdog(4);
}

void WrongDog::makeSound( void ) const {
    p_log("makeSound", "WrongDog");
   	std::cout << "[w] Mung-Mung!\n";
}
