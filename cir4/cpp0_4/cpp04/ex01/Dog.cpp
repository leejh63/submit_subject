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
		"Now he has a name!",
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
        brain = new Brain("bone");
        p_dog(0);
}

Dog::Dog( const Dog& copy )
:   Animal(copy.type)
    {
        p_log("Copy Constructor", "Dog");
        brain = new Brain("bone");
        p_dog(2);
}

Dog& Dog::operator=( const Dog& copy ) {
    p_log("Assign Operator", "Dog");
    if (this != &copy) {
    	Animal::operator=(copy); 
        *this->brain = *copy.brain;
        p_dog(3);
    }
    else {
        p_dog(4);
    }
    return *this;
}

Dog::~Dog( void ) {
    p_log("Destructor", "Dog");
    p_dog(5);
    delete brain;
}

void Dog::makeSound( void ) const {
    p_log("makeSound", "Dog");
   	std::cout << "Mung-Mung!\n";
}

const std::string& Dog::getOldid( const int mem ) const {
	p_log("getOldid", "Dog");
	return this->brain->getIdeas(mem);
}

void Dog::setNewid( const int mem, const std::string& novel) const {
	p_log("setNewid", "Dog");
	this->brain->setIdeas(mem, novel);
	return ;
}
