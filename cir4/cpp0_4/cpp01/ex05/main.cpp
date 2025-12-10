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

int main( void ) {
	Harl whiner;
	whiner.complain("quite"); std::cout << "\n";
	whiner.complain("DEBUG"); std::cout << "\n";
	whiner.complain("INFO"); std::cout << "\n";
	whiner.complain("WARNING"); std::cout << "\n";
	whiner.complain("ERROR"); std::cout << "\n";
	return 0;
}
