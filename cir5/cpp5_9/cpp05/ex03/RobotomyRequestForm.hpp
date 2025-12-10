/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RobotomyRequestForm.hpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/12 10:53:35 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/11/12 10:53:36 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROBOTOMYREQUESTFORM_HPP
#define ROBOTOMYREQUESTFORM_HPP

#include "AForm.hpp"
# include <string>
# include <iostream>

class Bureaucrat;

class RobotomyRequestForm : public AForm {
private:
	std::string target_;
	
	const std::string succ( void ) const ;
	const std::string fail( void ) const ;

RobotomyRequestForm( void );
public:
	const std::string getClass( void ) const ;
	RobotomyRequestForm( const std::string& target );
	RobotomyRequestForm( const RobotomyRequestForm& copy );
	RobotomyRequestForm& operator=( const RobotomyRequestForm& copy );
	virtual ~RobotomyRequestForm( void );
	void execute( const Bureaucrat& executor) const;
};

#endif










