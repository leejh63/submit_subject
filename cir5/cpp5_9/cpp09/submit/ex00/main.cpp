/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/06 11:21:17 by Jaeholee          #+#    #+#             */
/*   Updated: 2026/01/06 11:21:18 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "BitcoinExchange.hpp"

#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: ./btc <input_file>\n";
        return 1;
    }

    try {
        BitcoinExchange ex;
        ex.loadDatabase("data.csv");
        ex.processInput(argv[1]);
    } catch (const std::exception& e) {
        std::cout << e.what() << "\n";
        return 1;
    }
    return 0;
}
