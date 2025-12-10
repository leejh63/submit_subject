/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cat.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/23 13:32:11 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/23 13:32:13 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
        "YaYong? Did you just call me?",
        "Oh great, now I’ve got a name. Happy?",
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
        brain = new Brain("catnip");
        p_cat(0);
}

Cat::Cat( const Cat& copy )
:   Animal(copy)
    {
        p_log("Copy Constructor", "Cat");
        brain = new Brain(*copy.brain);
        p_cat(2);
}

Cat& Cat::operator=( const Cat& copy ) {
    p_log("Assign Operator", "Cat");
    if (this != &copy) {
    	Animal::operator=(copy); 
        *this->brain = *copy.brain;
        p_cat(3);
    }
    else {
        p_cat(4);
    }
    return *this;
}

Cat::~Cat( void ) {
    p_log("Destructor", "Cat");
    p_cat(5);
    delete brain;
}

void Cat::makeSound( void ) const {
    p_log("makeSound", "Cat");
   	std::cout << "YaYong-YaYong!\n";
}

const std::string& Cat::getOldid( const int mem ) const {
	p_log("getOldid", "Cat");
	return this->brain->getIdeas(mem);
}

void Cat::setNewid( const int mem, const std::string& novel) const {
	p_log("setNewid", "Cat");
	this->brain->setIdeas(mem, novel);
	return ;
}
