#include "Cat.hpp"

namespace {
	//
	bool skip = false;
	/*/
	bool skip = true;
	//*/
}

void Cat::p_cat( int num ) const {
    const static char* script[] = {
        "Meow? Did you just call me?",
        "Another copy of me? One cat is more than enough.",
        "You say it’s the same cat? Yeah right, whatever.",
        "Fine, fine… it’s still me, satisfied?",
        "Getting old... but naps are still top-tier.",
    };
    if (skip) { return; }
    std::cout << script[num] << "\n";
}

const std::string& Cat::getType( void ) const { return this->type; }

Cat::Cat( void )
:   Animal("Cat")
    {
        p_log("Default Constructor", "Cat");
        p_cat(0);
}

Cat::Cat( const Cat& copy )
:   Animal(copy.type)
    {
        p_log("Copy Constructor", "Cat");
        p_cat(1);
}

Cat& Cat::operator=( const Cat& copy ) {
    p_log("Assign Constructor", "Cat");
    if (this != &copy) {
        Animal::operator=(copy);
        p_cat(2);
    }
    else {
        p_cat(3);
    }
    return *this;
}

Cat::~Cat( void ) {
    p_log("Destructor", "Cat");
    p_cat(4);
}

void Cat::makeSound( void ) const {
    p_log("makeSound", "Cat");
   	std::cout << "YaYong-YaYong!\n";
}
