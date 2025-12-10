/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RobotomyRequestForm.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/12 10:53:32 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/11/12 10:53:33 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RobotomyRequestForm.hpp"
#include "Bureaucrat.hpp"
#include <cstdlib>
#include <ctime>

namespace {
	namespace util_{

		//
		bool skip = true;
		/*/
		bool skip = false;
		//*/
		void log( const std::string& func ) {
			if (skip) { return; }
			std::cout << "[RRF] : " << func << "\n";
		}
	}
}

const std::string RobotomyRequestForm::getClass( void ) const { return "RRF"; }

RobotomyRequestForm::RobotomyRequestForm( const std::string& target ) :
	AForm("RobotomyRequestForm", 72, 45),
	target_(target)
{
	util_::log("string Constructor");
}

RobotomyRequestForm::RobotomyRequestForm( const RobotomyRequestForm& copy ) :
	AForm(copy),
	target_(copy.target_)
{
	util_::log("Copy Constructor");
}

RobotomyRequestForm& RobotomyRequestForm::operator=( const RobotomyRequestForm& copy ){
	util_::log("= Operator");
	if (this != &copy) {
		AForm::operator=(copy);
		this->target_ = copy.target_;
	}
	return *this;
}

RobotomyRequestForm::~RobotomyRequestForm( void ){
	util_::log("Destructor");
}

const std::string RobotomyRequestForm::succ( void ) const {
	return "SUCCESS! Now! " + this->target_ + " obeys only me. Go make some money, scrap heap!\n";
}

const std::string RobotomyRequestForm::fail( void ) const {
	return "FAILURE!  " + this->target_ + " has lost something... but that's not my problems!\n";
}

void RobotomyRequestForm::execute( const Bureaucrat& executor) const{
	util_::log("execute");
	std::string exec_name = executor.getName();
	
	std::cout << "OH! " << this->getClass() << " makes somthing!\n"
			  << "drilling..drilling...drilling....drilling.....drilling\n";
	std::srand(std::clock());
	std::string finish = ( std::rand() % 2 ) ? fail() : succ();
	std::cout << finish;
}




































