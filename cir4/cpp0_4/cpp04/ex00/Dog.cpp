#include "Dog.hpp"

namespace {
	//
	bool skip = false;
	/*/
	bool skip = true;
	//*/
}

void Dog::p_dog( int num ) const {
	const static char* script[] = {
		"Oh! What a lovely cute dog!",
		"Double the cuteness! and Double the trouble!",
		"Same dog. Totally same. Can't believe it? Me neither!",
		"It is not a joke this is same dog! Totally same!",
		"Dog is too old...",
	};
	if (skip) { return; }
	std::cout << script[num] << "\n";
}

const std::string& Dog::getType( void ) const { return this->type; }

Dog::Dog( void )
:   Animal("Dog")
    {
        p_log("Default Constructor", "Dog");
        p_dog(0);
}

Dog::Dog( const Dog& copy )
:   Animal(copy.type)
    {
        p_log("Copy Constructor", "Dog");
        p_dog(1);
}

Dog& Dog::operator=( const Dog& copy ) {
    p_log("Assign Constructor", "Dog");
    if (this != &copy) {
        Animal::operator=(copy);
        p_dog(2);
    }
    else {
        p_dog(3);
    }
    return *this;
}

Dog::~Dog( void ) {
    p_log("Destructor", "Dog");
    p_dog(4);
}

void Dog::makeSound( void ) const {
    p_log("makeSound", "Dog");
   	std::cout << "Mung-Mung!\n";
}
