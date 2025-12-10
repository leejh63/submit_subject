/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ShrubberyCreationForm.hpp                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/06 10:54:01 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/11/06 10:54:02 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SHRUBBERYCREATIONFORM_HPP
#define SHRUBBERYCREATIONFORM_HPP

# include "AForm.hpp"
# include <string>
# include <iostream>

class Bureaucrat;

class ShrubberyCreationForm : public AForm {
private:
	std::string target_;

ShrubberyCreationForm( void );
public:
	const std::string getClass( void ) const ;
	ShrubberyCreationForm( const std::string& target );
	ShrubberyCreationForm( const ShrubberyCreationForm& copy );
	ShrubberyCreationForm& operator=( const ShrubberyCreationForm& copy );
	virtual ~ShrubberyCreationForm( void );
	void execute( const Bureaucrat& executor) const;
};

#endif
