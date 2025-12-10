/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PresidentialPardonForm.cpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/12 18:36:45 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/11/12 18:36:46 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "PresidentialPardonForm.hpp"
#include "Bureaucrat.hpp"

namespace {
	namespace util_{

		//
		bool skip = true;
		/*/
		bool skip = false;
		//*/
		void log( const std::string& func ) {
			if (skip) { return; }
			std::cout << "[PPF] : " << func << "\n";
		}
	}
}

const std::string PresidentialPardonForm::getClass( void ) const { return "PPF"; }

PresidentialPardonForm::PresidentialPardonForm( const std::string& target ) :
	AForm("PresidentialPardonForm", 25, 5),
	target_(target)
{
	util_::log("string Constructor");
}

PresidentialPardonForm::PresidentialPardonForm( const PresidentialPardonForm& copy ) :
	AForm(copy),
	target_(copy.target_)
{
	util_::log("Copy Constructor");
}

PresidentialPardonForm& PresidentialPardonForm::operator=( const PresidentialPardonForm& copy ){
	util_::log("= Operator");
	if (this != &copy) {
		AForm::operator=(copy);
		this->target_ = copy.target_;
	}
	return *this;
}

PresidentialPardonForm::~PresidentialPardonForm( void ){
	util_::log("Destructor");
}

void PresidentialPardonForm::execute( const Bureaucrat& executor ) const{
	util_::log("execute");
	(void) executor;
	std::cout << this->target_ << " has been pardoned by Zaphod Beeblebrox.\n";
}

