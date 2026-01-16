/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bitmain.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/15 15:36:57 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/12/15 15:37:01 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "bit.hpp"
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: ./btc <input_file>\n";
        return 1;
    }

    try {
        BitcoinExchange ex;
        ex.loadDatabase("data.csv");     // 과제 기본: data.csv는 고정
        ex.processInput(argv[1]);        // 입력 파일은 argv[1]
    } catch (const std::exception& e) {
        std::cout << e.what() << "\n";
        return 1;
    }
    return 0;
}
