/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Form.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/05 20:24:18 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/11/05 20:24:19 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Bureaucrat.hpp"
#include "Form.hpp"
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
			std::cout << "[Form] : " << func << "\n";
		}
	}
}

int Form::check_grade( int grade ) {
	util_::log("check_grade");
	if (grade > 150) { throw GradeTooLowException(); }
	if (grade < 1) { throw GradeTooHighException(); }
	return grade;
}

Form::Form( void ) :
	name_("empty_paper"),
	sign_grade_(150),
	exec_grade_(150),
	sign_(false)
{
	util_::log("Defalut Constructor");
}

Form::Form( const std::string& paper ) :
	name_(paper),
	sign_grade_(75),
	exec_grade_(75),
	sign_(false)
{
	util_::log("string Constructor");
}

Form::Form( const std::string& paper, int req_to_sign, int req_to_exec ) :
	name_(paper),
	sign_grade_(check_grade(req_to_sign)),
	exec_grade_(check_grade(req_to_exec)),
	sign_(false)
{
	util_::log("string,req_to_sign/exec Constructor");
}

Form::Form( const Form& copy ) :
	name_(copy.name_),
	sign_grade_(copy.sign_grade_),
	exec_grade_(copy.exec_grade_),
	sign_(copy.sign_)
{
	util_::log("Copy Constructor");
}
Form& Form::operator=( const Form& copy ) {
	util_::log("= operator");
	//Is it correct to copy the signed state?
	if (this != &copy) {
		this->sign_ = copy.sign_;
	}
	return *this;
}
Form::~Form( void ) {
	util_::log("Destructor");
}

void Form::beSigned( const Bureaucrat& signer) {
	util_::log("beSigned");
	int check_grade = signer.getGrade();
	if (this->sign_) { throw AlreadySignedException(); }
	if (check_grade > sign_grade_) { throw GradeTooLowException(); }
	this->sign_ = true;
}

const std::string Form::getName( void ) const  {
	util_::log("getName");
	return this->name_;
}

Form::AlreadySignedException::AlreadySignedException( void ) {
	util_::log("AlreadySignedException Construct");
};

const char* Form::AlreadySignedException::what( void ) const throw() {
	util_::log("AlreadySignedException what");
	return "[F]Form is already signed!";
};

Form::GradeTooHighException::GradeTooHighException( void ) {
	util_::log("GradeTooHighException Construct");
};

const char* Form::GradeTooHighException::what( void ) const throw() {
	util_::log("GradeTooHighException what");
	return "[F]Grade is too HIGH!";
};

Form::GradeTooLowException::GradeTooLowException( void ) {
	util_::log("GradeTooLowException Construct");
};

const char* Form::GradeTooLowException::what( void ) const throw() {
	util_::log("GradeTooLowException what");
	return "[F]Grade is too LOW!";
};

const std::string Form::state( void ) const {
	util_::log("state");
    std::ostringstream ss;
    ss << "name       : " << this->name_       << '\n'
       << "sign_grade : " << this->sign_grade_ << '\n'
       << "exec_grade : " << this->exec_grade_ << '\n'
       << "is_sign    : " << ((this->sign_) ? "Yes" : "No") << '\n';
    return ss.str();
}

std::ostream& operator<<( std::ostream& os, const Form& obj ) {
	util_::log("<< operator");
	os << obj.state();
	return os;
}


































