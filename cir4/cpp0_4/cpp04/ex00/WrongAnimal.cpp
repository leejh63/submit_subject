#include "WrongAnimal.hpp"

namespace {
	//
	bool skip = false;
	/*/
	bool skip = true;
	//*/
}

void WrongAnimal::p_wanimal( int num ) const {
	const static char* script[] = {
		"[W] I am real virtual Animal!",
		"[W] I am virtual Animal!",
		"[W] Now I'm the virtual Animal! Not you anymore!",
		"[W] Oh... I really don't want to be you...",
		"[W] It's me! virtual!",
		"[W] Do I exist? I don’t think so.",
		"[W] Sound? Why ask me? I'm a virtual.."
	};
	if (skip) { return; }
	std::cout << script[num] << "\n";
}

const std::string& WrongAnimal::getType( void ) const { return this->type; }

WrongAnimal::WrongAnimal( void )
:   type("[w] unknown")
    {
        p_log("Default Constructor", "WrongAnimal");
        p_wanimal(0);
}

WrongAnimal::WrongAnimal( const std::string& type )
:   type(type)
    {
        p_log("String Constructor", "WrongAnimal");
        p_wanimal(1);
}

WrongAnimal::WrongAnimal( const WrongAnimal& copy )
:   type(copy.type)
    {
        p_log("Copy Constructor", "WrongAnimal");
        p_wanimal(2);
}

WrongAnimal& WrongAnimal::operator=( const WrongAnimal& copy ) {
    p_log("Assign Constructor", "WrongAnimal");
    if (this != &copy) {
        this->type = copy.type;
        p_wanimal(3);
    }
    else {
        p_wanimal(4);
    }
    return *this;
}

WrongAnimal::~WrongAnimal( void ) {
    p_log("Destructor", "WrongAnimal");
    p_wanimal(5);
}

void WrongAnimal::makeSound( void ) const {
    p_log("makeSound", "WrongAnimal");
    p_wanimal(6);
}

void WrongAnimal::p_log( const std::string& log_msg, const std::string& _who) const {
    std::cout << "[" << getType() << "] call the [" << _who << " " << log_msg << "]\n";
}

