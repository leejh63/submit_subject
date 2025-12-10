/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/18 12:55:55 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/18 12:55:57 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ClapTrap.hpp"
#include <iostream>

int main( void ) {
    std::cout << "=== 생성자 테스트 ===\n";
    ClapTrap a("Alpha");
    ClapTrap b("Bravo");
    ClapTrap c(a);         // 복사 생성자
    b = a;                 // 대입 연산자

    std::cout << "\n=== 기본 동작 테스트 ===\n";
    a.attack("Bravo");
    b.takeDamage(5);
    b.beRepaired(3);

    std::cout << "\n=== 에너지 고갈 테스트 ===\n";
    for (int i = 0; i < 12; ++i)
        a.attack("Dummy");

    std::cout << "\n=== 사망 상태 테스트 ===\n";
    b.takeDamage(50);      // HP = 0
    b.attack("Alpha");     // 죽은 상태로 공격 시도
    b.beRepaired(10);      // 죽은 상태로 회복 시도

    std::cout << "\n=== 소멸자 테스트 ===\n";
    return 0;
}
