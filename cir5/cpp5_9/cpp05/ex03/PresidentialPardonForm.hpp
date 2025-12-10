/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PresidentialPardonForm.hpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/12 18:36:42 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/11/12 18:36:43 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef PRESIDENTIALPARDONFORM_HPP
#define PRESIDENTIALPARDONFORM_HPP

#include "AForm.hpp"
# include <string>
# include <iostream>

class Bureaucrat;

class PresidentialPardonForm : public AForm {
private:
	std::string target_;

PresidentialPardonForm( void );
public:
	const std::string getClass( void ) const ;
	PresidentialPardonForm( const std::string& target );
	PresidentialPardonForm( const PresidentialPardonForm& copy );
	PresidentialPardonForm& operator=( const PresidentialPardonForm& copy );
	virtual ~PresidentialPardonForm( void );
	void execute( const Bureaucrat& executor) const;
};

#endif
