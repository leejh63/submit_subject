/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Intern.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/12 19:22:26 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/11/12 19:22:27 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Intern.hpp"
#include "ShrubberyCreationForm.hpp"
#include "RobotomyRequestForm.hpp"
#include "PresidentialPardonForm.hpp"

namespace {
	namespace util_{

		//
		bool skip = true;
		/*/
		bool skip = false;
		//*/
		void log( const std::string& func ) {
			if (skip) { return; }
			std::cout << "[INTR] : " << func << "\n";
		}
	}
	AForm* m_tree( const std::string& target ) {  
		std::cout << "Intern creates ShrubberyCreationForm\n";
		return new ShrubberyCreationForm(target);
	}
	
	AForm* m_robo( const std::string& target ) {  
		std::cout << "Intern creates RobotomyRequestForm\n";
		return new RobotomyRequestForm(target);
	}
	
	AForm* m_pard( const std::string& target ) {  
		std::cout << "Intern creates PresidentialPardonForm\n";
		return new PresidentialPardonForm(target);
	}
	
	AForm* m_null( const std::string& target ) { (void)target; return NULL; }
	
	//AForm* (*fn_[3])( const std::string& target ) = { &m_tree, &m_robo, &m_pard };
	AForm* (*fn_[5])( const std::string& target ) = { &m_null, &m_tree, &m_robo, NULL, &m_pard };
}


Intern::NoFormException::NoFormException( const std::string& paper ) :
	msg_("Intern couldn't find form: " + paper + "\n")
{
	util_::log("NoFormException Constructor");
}

Intern::NoFormException::~NoFormException( void ) throw() {
	util_::log("Destructor");
}

const char* Intern::NoFormException::what( void ) const throw() {
	util_::log("NoFormException what");
	return  msg_.c_str(); 
}


Intern::Intern( void ) {
	util_::log("Default Constructor");
}

Intern::~Intern( void ) {
	util_::log("Destructor");
}

/*
AForm* Intern::makeForm( const std::string& paper, const std::string& target ) {
	util_::log("makeForm");
	const std::string order[3] = { "shrubbery creation", "robotomy request", "presidential pardon" };
	for (int i = 0; i < 3; ++i) {
		if (order[i] == paper) {
			return fn_[i](target);
		}
	}
	throw NoFormException(paper);
}
*/
/*
AForm* (*fn_[5])( const std::string& target ) = { &m_null, &m_tree, &m_robo, NULL, &m_pard };
*/
AForm* Intern::makeForm( const std::string& paper, const std::string& target ) {
	util_::log("makeForm");
	AForm* test = fn_[(int)(paper == "shrubbery creation")
					+ ((int)(paper == "robotomy request") << 1)
					+ ((int)(paper == "presidential pardon") << 2)](target);
	return (test) ? test : throw NoFormException(paper);
}






















