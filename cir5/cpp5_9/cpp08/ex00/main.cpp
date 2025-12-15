/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/12 17:24:11 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/12/12 17:24:12 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <vector>
#include <list>
#include "easyfind.hpp"

int main() {
    try {
        std::vector<int> v;
        v.push_back(10);
        v.push_back(20);
        v.push_back(30);

        std::vector<int>::iterator vit = easyfind(v, 20);
        std::cout << "found the value in vector: " << *vit << "\n";
        
		*vit = 60;
		vit = easyfind(v, 60);
        std::cout << "found the new value in vector: " << *vit << "\n";
        
        // 못 찾는 케이스
        easyfind(v, 999);
    } catch (const std::exception& e) {
        std::cout << "exception: " << e.what() << "\n";
    }

    try {
        std::list<int> lst;
        lst.push_back(1);
        lst.push_back(2);
        lst.push_back(3);

        std::list<int>::iterator lit = easyfind(lst, 3);
        std::cout << "found the value in list: " << *lit << "\n";
        *lit = 5;
        lit = easyfind(lst, 5);
        std::cout << "found the new value in list: " << *lit << "\n";
    } catch (const std::exception& e) {
        std::cout << "exception: " << e.what() << "\n";
    }

    try {
        const std::vector<int> cv(3, 7); // [7,7,7]
        std::vector<int>::const_iterator cit = easyfind(cv, 7);
        std::cout << "found the value in const vector: " << *cit << "\n";
    } catch (const std::exception& e) {
        std::cout << "exception: " << e.what() << "\n";
    }

    return 0;
}

