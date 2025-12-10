/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bureaucrat.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/05 18:31:22 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/11/05 18:31:23 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Bureaucrat.hpp"

namespace util_{

	//
	bool skip = true;
	/*/
	bool skip = false;
	//*/
	void log( const std::string& func ) {
		if (skip) { return; }
		std::cout << "[Bureaucrat] : " << func << "\n";
	}
}

const int Bureaucrat::limit_[2] = {1, 150};

Bureaucrat::GradeTooHighException::GradeTooHighException( void ) throw() {}

const char* Bureaucrat::GradeTooHighException::what( void ) const throw() {
	return "Grade is too HIGH!";
}

Bureaucrat::GradeTooLowException::GradeTooLowException( void ) throw() {}

const char* Bureaucrat::GradeTooLowException::what( void ) const throw() {
	return "Grade is too LOW!"; 
}


Bureaucrat::Bureaucrat( void ) : 
	name_("maldan"),
	grade_(150)
{
	util_::log("Defalut Constructor");
}

Bureaucrat::Bureaucrat( const std::string& name ) : 
	name_(name),
	grade_(75) 
{
	util_::log("string Constructor");
}

Bureaucrat::Bureaucrat( const std::string& name, int grade ) :
	name_(name)
{
	util_::log("string,int Constructor");
	if (grade > 150) { throw GradeTooLowException(); }
	if (grade < 1) { throw GradeTooHighException(); }
	this->grade_ = grade;
}

Bureaucrat::Bureaucrat( const Bureaucrat& copy )  :
	name_(copy.name_),
	grade_(copy.grade_)
{
	util_::log("Copy Constructor");
}

Bureaucrat& Bureaucrat::operator=( const Bureaucrat& copy ) {
	util_::log("= Operator");
	if (this != &copy) {
		this->grade_ = copy.grade_;
	}
	return *this;
}

Bureaucrat::~Bureaucrat( void ) {
	util_::log("Destructor");
}

std::string Bureaucrat::getName( void ) const {
	util_::log("getName");
	return this->name_;
}

int Bureaucrat::getGrade( void ) const {
	util_::log("getGrade");
	return this->grade_;
}

void Bureaucrat::increGrade( void ) {
	util_::log("increGrade");
	if (this->grade_ <= 1) { throw GradeTooHighException(); }
	this->grade_ -= 1;
}

void Bureaucrat::decreGrade( void ) {
	util_::log("decreGrade");
	if (this->grade_ >= 150) { throw GradeTooLowException(); }
	this->grade_ += 1;
}

std::ostream& operator<<( std::ostream& os, const Bureaucrat& obj) {
	util_::log("<< Operator");
	std::string tmp_n = obj.getName();
	int tmp_g = obj.getGrade();
	os << tmp_n << ", bureaucrat grade " << tmp_g << ".\n";
	return os;
}











































