/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Form.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/05 20:24:21 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/11/05 20:24:22 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FORM_HPP
#define FORM_HPP

# include <string>
# include <iostream>
# include <stdexcept>

class Bureaucrat;

class Form {
private:
	const std::string name_;
	const int sign_grade_;
	const int exec_grade_;
	bool sign_;
	
	static int check_grade( int grade );
public:
	Form( void );
	Form( const std::string& paper );
	Form( const std::string& docu, int req_to_sign, int req_to_exec );
	Form( const Form& copy );
	Form& operator=( const Form& copy );
	~Form( void );
	
	void beSigned( const Bureaucrat& signer);
	const std::string getName( void ) const ;
	
	const std::string state( void ) const ;


	class GradeTooHighException : public std::exception {
		public: GradeTooHighException( void );
		const char* what( void ) const throw();
	};

	class GradeTooLowException : public std::exception {
	public: GradeTooLowException( void );
		const char* what( void ) const throw();
	};

	class AlreadySignedException : public std::exception {
	public: AlreadySignedException( void );
		const char* what( void ) const throw();
    };
};

std::ostream& operator<<( std::ostream& os, const Form& obj );

#endif

























