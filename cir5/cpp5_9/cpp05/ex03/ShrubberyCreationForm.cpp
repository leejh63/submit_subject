/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ShrubberyCreationForm.cpp                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/06 10:53:59 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/11/06 10:53:59 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ShrubberyCreationForm.hpp"
#include "Bureaucrat.hpp"
#include <fstream>

namespace {
	namespace util_{

		//
		bool skip = true;
		/*/
		bool skip = false;
		//*/
		void log( const std::string& func ) {
			if (skip) { return; }
			std::cout << "[SCF] : " << func << "\n";
		}
	}
}

const std::string ShrubberyCreationForm::getClass( void ) const { return "SCF"; }

ShrubberyCreationForm::ShrubberyCreationForm( const std::string& target ) :
	AForm("ShrubberyCreationForm", 145, 137),
	target_(target)
{
	util_::log("string Constructor");
}

ShrubberyCreationForm::ShrubberyCreationForm( const ShrubberyCreationForm& copy ) :
	AForm(copy),
	target_(copy.target_)
{
	util_::log("Copy Constructor");
}

ShrubberyCreationForm& ShrubberyCreationForm::operator=( const ShrubberyCreationForm& copy ) {
	util_::log("= Operator");
	if (this != &copy) {
		AForm::operator=(copy);
		this->target_ = copy.target_;
	}
	return *this;
}

ShrubberyCreationForm::~ShrubberyCreationForm( void ) {
	util_::log("Destructor");
}

void ShrubberyCreationForm::execute( const Bureaucrat& executor ) const {
	util_::log("execute");
	std::string exec_name = executor.getName();
	
    const std::string filename = target_ + "_shrubbery";
    std::ofstream ofs(filename.c_str(), std::ios::app);
    
	ofs << "=========" << exec_name << "'s tree!!==========\n"
		<< "           *         .              .        \n"
		<< "                       .         .         * \n"
		<< "      .       .   .        .         .       \n"
		<< "                 ccee88oo            *       \n"
		<< "            C8O8O8Q8PoOb o8oo                \n"
		<< "          dOB69QO8PdUOpugoO9bD               \n"
		<< "         CgggbU8OU qOp qOdoUOdcb             \n"
		<< "             6OuU  /p u gcoUodpP             \n"
		<< "               \\\\\\//  /douUP              \n"
		<< "                 \\\\\\////                  \n"
		<< "                  |||/\\                     \n"
		<< "           *      |||\\/          .          \n"
		<< "                  |||||              *       \n"
		<< "            .....//||||\\....                \n"
		<< "=============================================\n";
}













































