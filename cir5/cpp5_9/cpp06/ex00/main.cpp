/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/22 12:58:30 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/11/22 12:58:31 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// main.cpp
#include "Converter.hpp"
#include <iostream>

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: ./convert <literal>" << std::endl;
        return 1;
    }
    Converter::convert(argv[1]);
    return 0;
}
