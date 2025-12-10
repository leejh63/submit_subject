/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Intern.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/12 19:22:30 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/11/12 19:22:34 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef INTERN_HPP
#define INTERN_HPP

#include <string>
//#include <exception>

class AForm;

class Intern {
private:
Intern( const Intern& copy );
Intern& operator=( const Intern& copy );

public:
	Intern( void );
	~Intern( void );

	AForm* makeForm( const std::string& paper, const std::string& target );
	
	class NoFormException : public std::exception {
		private:
			std::string msg_;
		public:
			NoFormException( const std::string& paper );
			~NoFormException( void ) throw() ;
			const char* what( void ) const throw();
	};

};

#endif

