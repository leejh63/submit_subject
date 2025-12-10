/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AForm.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/05 20:24:21 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/11/05 20:24:22 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef AFORM_HPP
#define AFORM_HPP

# include <string>
# include <iostream>
# include <stdexcept>

class Bureaucrat;

class AForm {
private:
	const std::string name_;
	const int sign_grade_;
	const int exec_grade_;
	bool sign_;
	
	static int check_grade( int grade );

protected:
	virtual void execute( const Bureaucrat& executor ) const = 0;

public:
	AForm( void );
	AForm( const AForm& copy );
	AForm& operator=( const AForm& copy );
	virtual ~AForm( void );

	AForm( const std::string& paper );
	AForm( const std::string& docu, int req_to_sign, int req_to_exec );
	
	void beSigned( const Bureaucrat& signer );
	void beExecuted( const Bureaucrat& executor ) const ;

	const std::string getName( void ) const ;
	virtual const std::string getClass( void ) const ;

	const std::string state( void ) const ;


	class GradeTooHighException : public std::exception {
		public:
			GradeTooHighException( void );
			const char* what( void ) const throw();
	};

	class GradeTooLowException : public std::exception {
		public: 
			GradeTooLowException( void );
			const char* what( void ) const throw();
	};

	class AlreadySignedException : public std::exception {
		public: 
			AlreadySignedException( void );
			const char* what( void ) const throw();
	};

	class NeedToSignException : public std::exception {
		public:
			NeedToSignException( void );
			const char* what( void ) const throw();
	};
};

std::ostream& operator<<( std::ostream& os, const AForm& obj );

#endif

























