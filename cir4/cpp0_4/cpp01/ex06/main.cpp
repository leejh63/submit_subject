/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/16 11:31:44 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/16 11:31:45 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Harl.hpp"

int trim_level(std::string& level) {
	const static char* trimword = " \t\n\v\f\r";
	std::size_t start, last;
	if (level.empty()) { return 1; }
	start = level.find_first_not_of(trimword);
	if (start == std::string::npos)  { return 2; }
	last = level.find_last_not_of(trimword);
	level = level.substr(start, last - start + 1);
	return 0;
}

int main( int argc, char* argv[] ) {
	if (argc != 2) { std::cout << "[ Probably complaining about insignificant problems ]\n"; return 0;}
	std::string level = argv[1];
	trim_level(level);
	Harl whiner;
	whiner.complain(level);
	return 0;
}
