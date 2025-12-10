/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bureaucrat.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/05 18:31:37 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/11/05 18:31:39 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BUREAUCRAT_HPP
#define BUREAUCRAT_HPP
# include <string>
# include <iostream>
# include <stdexcept>

class Bureaucrat {
private:
	const static int limit_[2];
	const std::string name_;
	int grade_;
public:
	Bureaucrat( void );
	Bureaucrat( const std::string& name );
	Bureaucrat( const std::string& name, int grade );
	Bureaucrat( const Bureaucrat& copy );
	Bureaucrat& operator=( const Bureaucrat& copy );
	~Bureaucrat( void );
	
	std::string getName( void ) const;
	int getGrade( void ) const;

	void increGrade( void );
	void decreGrade( void );

	class GradeTooHighException : public std::exception {
	public:
		GradeTooHighException( void ) throw();
		const char* what( void ) const throw();
	};
	class GradeTooLowException : public std::exception {
	public:
		GradeTooLowException( void ) throw();
		const char* what( void ) const throw();
	};
};

std::ostream& operator<<( std::ostream& os, const Bureaucrat& obj);

#endif





