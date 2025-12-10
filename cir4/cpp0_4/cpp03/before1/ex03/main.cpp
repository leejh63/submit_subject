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

// main.cpp (for ex03: DiamondTrap)
// 빌드: c++ -Wall -Wextra -Werror -std=c++98 *.cpp -o ex03_test

#include <iostream>
#include "DiamondTrap.hpp"

static void line(const char* title) {
    std::cout << "\n==================== " << title << " ====================\n";
}

// 에너지 소모(EP=50인지 간접 확인): 60번 공격 호출 후, 뒤쪽은 "no energy"류 메시지가 떠야 정상.
static void drain_energy(DiamondTrap& d, const std::string& tgt, int tries) {
    for (int i = 0; i < tries; ++i) d.attack(tgt);
}
//*/
int main() {
    line("CTOR / DTOR ORDER");
    {
        std::cout << "[블록 시작]\n";
        DiamondTrap d("Dia-A");
        d.whoAmI();
        d.attack("테스트입니당");
        d.guardGate();
        d.attack("Not excute!");
        std::cout << "[블록 종료 → 소멸자 순서 확인]\n";
    }

    line("BASIC ACTIONS");
    DiamondTrap dia("Diamond");
    dia.whoAmI();           // "my name: Diamond, clap name: Diamond_clap_name"
    dia.attack("enemy-1");  // ScavTrap::attack 사용 (메시지로 확인)
    dia.takeDamage(30);     // HP 감소
    dia.beRepaired(20);     // EP 1 소모

    line("ENERGY DRAIN (EP=50 간접 검증)");
    // EP는 ScavTrap 기준 50. 앞에서 1 소모했을 수 있으니 넉넉하게 60회.
    drain_energy(dia, "battery", 60);
    dia.attack("after-empty");  // 에너지 없어서 실패 메시지 예상
    dia.beRepaired(1);          // 이것도 실패 메시지 예상

    line("DEATH STATE (HP=0에서 동작 금지)");
    dia.takeDamage(1000);       // 확실히 사망
    dia.attack("ghost");        // 실패해야 정상
    dia.beRepaired(10);         // 실패해야 정상
    dia.whoAmI();               // 이름 출력은 가능

    line("COPY CTOR / ASSIGNMENT");
    DiamondTrap a("Alpha");
    DiamondTrap b("Beta");

    std::cout << "[Copy Ctor]\n";
    DiamondTrap a_copy(a);
    a_copy.whoAmI();

    std::cout << "[Copy Assign]\n";
    b = a;
    b.whoAmI();

    line("MIXED PLAY");
    DiamondTrap g("Gem");
    g.attack("t1");
    g.takeDamage(49);
    g.beRepaired(5);
    g.attack("t2");
    g.takeDamage(1000);
    g.attack("t3");   // 사망 상태 → 실패 확인

    line("END");
    return 0;
}


