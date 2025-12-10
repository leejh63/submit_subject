#include "WrongCat.hpp"

namespace {
	//
	bool skip = false;
	/*/
	bool skip = true;
	//*/
}

void WrongCat::p_wcat( int num ) const {
    const static char* script[] = {
        "[w] Meow? Did you just call me?",
        "[w] Another copy of me? One cat is more than enough.",
        "[w] You say it’s the same cat? Yeah right, whatever.",
        "[w] Fine, fine… it’s still me, satisfied?",
        "[w] Getting old... but naps are still top-tier.",
    };
    if (skip) { return; }
    std::cout << script[num] << "\n";
}

const std::string& WrongCat::getType( void ) const { return this->type; }

WrongCat::WrongCat( void )
:   WrongAnimal("WrongCat")
    {
        p_log("Default Constructor", "WrongCat");
        p_wcat(0);
}

WrongCat::WrongCat( const WrongCat& copy )
:   WrongAnimal(copy.type)
    {
        p_log("Copy Constructor", "WrongCat");
        p_wcat(1);
}

WrongCat& WrongCat::operator=( const WrongCat& copy ) {
    p_log("Assign Constructor", "WrongCat");
    if (this != &copy) {
        WrongAnimal::operator=(copy);
        p_wcat(2);
    }
    else {
        p_wcat(3);
    }
    return *this;
}

WrongCat::~WrongCat( void ) {
    p_log("Destructor", "WrongCat");
    p_wcat(4);
}

void WrongCat::makeSound( void ) const {
    p_log("makeSound", "WrongCat");
   	std::cout << "[w] YaYong-YaYong!\n";
}
