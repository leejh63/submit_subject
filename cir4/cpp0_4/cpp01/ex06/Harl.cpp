/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Harl.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/16 11:31:52 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/16 11:31:52 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Harl.hpp"

// private
void Harl::debug( void ) {
	std::cout << "[ DEBUG ]\nI love having extra bacon "
			  << "for my 7XL-double-cheese-"
			  << "triple-pickle-special-ketchup burger."
			  << "I really do!\n"; 
}

void Harl::info( void ) {
	std::cout << "[ INFO ]\nI cannot believe adding extra "
			  << "bacon costs more money. "
			  << "You didn’t put enough bacon in my burger! "
			  << "If you did, I wouldn’t be asking for more!\n"; 
}

void Harl::warning( void ) {
	std::cout << "[ WARNING ]\nI think I deserve to have some extra bacon for free. "
			  << "I’ve been coming for years, "
			  << "whereas you started working here just last month.\n";
}

void Harl::error( void ) {
	std::cout << "[ ERROR ]\nThis is unacceptable! "
			  << "I want to speak to the manager now.\n";
}

// public
void Harl::complain( std::string level ) {
	const std::string claim_level[] = {
		"DEBUG", "INFO", "WARNING", "ERROR"
	};
	
	for (int i = 0; i < 4; ++i) {
		if (claim_level[i] == level) {
			switch (i) {
				case 0 : debug();
				case 1 : info();
				case 2 : warning();
				case 3 : error(); return;
				//default : return;
			}
		}
	}
	std::cout << "[ Probably complaining about insignificant problems ]\n";
}
