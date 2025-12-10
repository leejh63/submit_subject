/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AForm.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/05 20:24:18 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/11/05 20:24:19 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Bureaucrat.hpp"
#include "AForm.hpp"
#include <sstream>

namespace {
	namespace util_{

		//
		bool skip = true;
		/*/
		bool skip = false;
		//*/
		void log( const std::string& func ) {
			if (skip) { return; }
			std::cout << "[AForm] : " << func << "\n";
		}
	}
}

const std::string AForm::getClass( void ) const { return "AForm"; }

int AForm::check_grade( int grade ) {
	util_::log("check_grade");
	if (grade > 150) { throw GradeTooLowException(); }
	if (grade < 1) { throw GradeTooHighException(); }
	return grade;
}

AForm::AForm( void ) :
	name_("empty_paper"),
	sign_grade_(150),
	exec_grade_(150),
	sign_(false)
{
	util_::log("Default Constructor");
}

AForm::AForm( const std::string& paper ) :
	name_(paper),
	sign_grade_(75),
	exec_grade_(75),
	sign_(false)
{
	util_::log("string Constructor");
}

AForm::AForm( const std::string& paper, int req_to_sign, int req_to_exec ) :
	name_(paper),
	sign_grade_(check_grade(req_to_sign)),
	exec_grade_(check_grade(req_to_exec)),
	sign_(false)
{
	util_::log("string,req_to_sign/exec Constructor");
}

AForm::AForm( const AForm& copy ) :
	name_(copy.name_),
	sign_grade_(copy.sign_grade_),
	exec_grade_(copy.exec_grade_),
	sign_(copy.sign_)
{
	util_::log("Copy Constructor");
}

AForm& AForm::operator=( const AForm& copy ) {
	util_::log("= operator");
	//Is it correct to copy the signed state?
	if (this != &copy) {
		this->sign_ = copy.sign_;
	}
	return *this;
}

AForm::~AForm( void ) {
	util_::log("Destructor");
}

void AForm::beSigned( const Bureaucrat& signer ) {
	util_::log("beSigned");
	int check_grade = signer.getGrade();
	if (this->sign_) { throw AlreadySignedException(); }
	if (check_grade > sign_grade_) { throw GradeTooLowException(); }
	this->sign_ = true;
}

void AForm::beExecuted( const Bureaucrat& executor ) const {
	util_::log("beExecuted");
	int check_grade = executor.getGrade();
	if (!this->sign_) { throw NeedToSignException(); }
	if (check_grade > exec_grade_) { throw GradeTooLowException(); }
	execute(executor);
}

const std::string AForm::getName( void ) const  {
	util_::log("getName");
	return this->name_;
}

AForm::AlreadySignedException::AlreadySignedException( void )
{
	util_::log("AlreadySignedException Construct");
}

AForm::NeedToSignException::NeedToSignException( void )
{
	util_::log("NeedToSignException Construct");
}

AForm::GradeTooHighException::GradeTooHighException( void )
{
	util_::log("GradeTooHighException Construct");
}

AForm::GradeTooLowException::GradeTooLowException( void )
{
	util_::log("GradeTooLowException Construct");
}

const char* AForm::GradeTooHighException::what( void ) const throw() {
	util_::log("GradeTooHighException what");
	return "Grade is too HIGH!";
};

const char* AForm::GradeTooLowException::what( void ) const throw() {
	util_::log("GradeTooLowException what");
	return "Grade is too LOW!";
};

const char* AForm::AlreadySignedException::what( void ) const throw() {
	util_::log("AlreadySignedException what");
	return "is already signed!";
};

const char* AForm::NeedToSignException::what( void ) const throw() {
	util_::log("NeedToSignException what");
	return "needs to be signed!";
};

const std::string AForm::state( void ) const {
	util_::log("state");
    std::ostringstream ss;
    ss << "name       : " << this->name_       << '\n'
       << "sign_grade : " << this->sign_grade_ << '\n'
       << "exec_grade : " << this->exec_grade_ << '\n'
       << "is_sign    : " << ((this->sign_) ? "Yes" : "No") << '\n';
    return ss.str();
}

std::ostream& operator<<( std::ostream& os, const AForm& obj ) {
	util_::log("<< operator");
	os << obj.state();
	return os;
}


































