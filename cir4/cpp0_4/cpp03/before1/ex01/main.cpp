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
#include "ScavTrap.hpp"
#include <iostream>

int main( void ) {
    std::cout << "=== 생성자 테스트 ===\n";
    ScavTrap a("Alice");
    ScavTrap b("Bob");
    ScavTrap c(a);            // 복사 생성자
    b = a;                    // 대입 연산자

    std::cout << "\n=== 기본 공격/피해 테스트 ===\n";
    a.attack("Bob");
    b.takeDamage(20);
    b.beRepaired(10);

    std::cout << "\n=== 에너지 부족 테스트 ===\n";
    for (int i = 0; i < 55; ++i)
        a.attack("Dummy");    // 에너지 다 소모될 때까지 공격

    std::cout << "\n=== 게이트 키퍼 모드 테스트 ===\n";
    c.guardGate();            // 모드 진입
    c.attack("Intruder");     // 게이트 모드 중 공격 가능 여부 확인

    std::cout << "\n=== 데미지 누적 및 사망 테스트 ===\n";
    b.takeDamage(200);        // HP 0 만들기
    b.attack("Alice");        // 사망 후 공격 불가
    b.beRepaired(10);         // 사망 후 회복 불가
    b.guardGate();            // 사망 후 모드 진입 불가

    std::cout << "\n=== 소멸자 테스트 ===\n";
    return 0;
}

