/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/11 12:42:18 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/12/11 12:42:41 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <iostream>

// c++ 14에서 부터 가능
template <typename T>
T myValue;    // T 기본값으로 초기화

int main() {
    myValue<int> = 10;
    myValue<float> = 3.14f;
    myValue<std::string> = "hello";

    std::cout << myValue<int> << std::endl;         // 10
    std::cout << myValue<float> << std::endl;       // 3.14
    std::cout << myValue<std::string> << std::endl; // hello
}
