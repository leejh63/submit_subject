#include "Animal.hpp"

namespace {
	//
	bool skip = false;
	/*/
	bool skip = true;
	//*/
}

void Animal::p_animal( int num ) const {
	const static char* script[] = {
		"I am real virtual Animal!",
		"I am virtual Animal!",
		"Now I'm the virtual Animal! Not you anymore!",
		"Oh... I really don't want to be you...",
		"It's me! virtual!",
		"Do I exist? I don’t think so.",
		"Sound? Why ask me? I'm a virtual.."
	};
	if (skip) { return; }
	std::cout << script[num] << "\n";
}

const std::string& Animal::getType( void ) const { return this->type; }

Animal::Animal( void )
:   type("unknown")
    {
        p_log("Default Constructor", "Animal");
        p_animal(0);
}

Animal::Animal( const std::string& type )
:   type(type)
    {
        p_log("String Constructor", "Animal");
        p_animal(1);
}

Animal::Animal( const Animal& copy )
:   type(copy.type)
    {
        p_log("Copy Constructor", "Animal");
        p_animal(2);
}

Animal& Animal::operator=( const Animal& copy ) {
    p_log("Assign Constructor", "Animal");
    if (this != &copy) {
        this->type = copy.type;
        p_animal(3);
    }
    else {
        p_animal(4);
    }
    return *this;
}

Animal::~Animal( void ) {
    p_log("Destructor", "Animal");
    p_animal(5);
}

void Animal::makeSound( void ) const {
    p_log("makeSound", "Animal");
    p_animal(6);
}

void Animal::p_log( const std::string& log_msg, const std::string& _who) const {
    std::cout << "[" << getType() << "] call the [" << _who << " " << log_msg << "]\n";
}

