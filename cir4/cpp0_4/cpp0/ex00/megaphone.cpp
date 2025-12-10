/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   megaphone.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/13 11:17:58 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/13 11:18:00 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <iostream>
#include <cctype>

void silence( void ) {
	std::cout << "* LOUD AND UNBEARABLE FEEDBACK NOISE *\n";
}

int main(int argc, char* argv[]) {
	if (argc < 2) { silence(); return 0; }
	std::string wordset;
	for (std::size_t i = 1; argv[i] != NULL; ++i) {
		for (std::size_t ii = 0; argv[i][ii] != '\0'; ++ii) {
			unsigned char tmp = argv[i][ii];
			argv[i][ii] = std::toupper(tmp);
		}
		wordset += argv[i];
	}
	if (wordset.empty()) { silence(); return 0; }
	std::cout << wordset << "\n";
	return 0;
}
