/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/09 11:43:21 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/12/09 12:49:03 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Serializer.hpp"
#include <iostream>

struct Data {
    int         data1;
    size_t      data2;
    char        data3[4];
    std::string data4;

    Data() : data1(1), data2(2), data4("test") {
        data3[0] = 'a';
        data3[1] = 'b';
        data3[2] = 'c';
        data3[3] = 'd';
    }
};


void data_helper_print( const Data& data1 ) {
	std::cout << "\n====DATA====\n"
			  << data1.data1 << '\n'
			  << data1.data2 << '\n'
			  << data1.data3[0] << ' '
			  << data1.data3[1] << ' '
			  << data1.data3[2] << ' '
			  << data1.data3[3] << '\n'
			  << data1.data4 << '\n'
			  << "============\n\n";
}

int main(void) {
	Data test1;
	
	data_helper_print(test1);
	
	uintptr_t ptr = Serializer::serialize(&test1);
	
	std::cout << "ptr정수  " << ptr << " ptr주소  "
			  << "0x" << std::hex << ptr << '\n';
	
	Data* dptr = Serializer::deserialize(ptr);
	
	std::cout << "dptr정수 " << std::dec << ptr; 
	std::cout << " dptr주소 0x" << std::hex << ptr << '\n';

	data_helper_print(*dptr);
	
	return 0;	
}





























