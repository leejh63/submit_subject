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

#include <iostream>
#include <string>
#include "ClapTrap.hpp"
#include "FragTrap.hpp"

static void line(const char* title) {
    std::cout << "\n==================== " << title << " ====================\n";
}

static void drain_energy_with_attacks(FragTrap& ft, const std::string& tgt, int tries) {
    for (int i = 0; i < tries; ++i) {
        ft.attack(tgt); // EP 1씩 감소. EP=0 되면 더 못해야 정상(ClapTrap 규칙).
    }
}

int main() {
    line("CTOR/DTOR ORDER (스코프 종료 시 파괴 순서 확인)");
    {
        std::cout << "[블록 시작]\n";
        ClapTrap c("CT-X");        // ClapTrap 메시지
        FragTrap f("FT-X");        // ClapTrap → FragTrap 순으로 생성 메시지
        std::cout << "[블록 끝: 소멸자 체인 확인]\n";
    } // 여기서 f → ClapTrap 순으로 소멸

    line("BASIC ACTIONS");
    FragTrap ft("FRAG-1");
    ft.highFivesGuys();            // 긍정적 하이파이브 요청 출력
    ft.attack("dummy-1");          // FragTrap 고유 메시지(ClapTrap 규칙에 맞게 출력)
    ft.takeDamage(35);             // HP 감소
    ft.beRepaired(20);             // HP 회복(EP 1 소모)

    line("ENERGY DRAIN (EP 고갈 후 행동 금지 확인)");
    // FragTrap EP=100 가정. 넉넉히 120회 호출해 EP=0 이후 동작 금지 확인.
    drain_energy_with_attacks(ft, "energy-sink", 120);
    ft.attack("after-empty");      // 실패해야 정상
    ft.beRepaired(1);              // 실패해야 정상

    line("DEATH STATE (HP 0에서 행동 금지 확인)");
    ft.takeDamage(1000);           // 확실히 사망
    ft.attack("after-dead");       // 실패해야 정상
    ft.beRepaired(10);             // 실패해야 정상
    ft.highFivesGuys();            // 구현에 따라 막거나 메시지 출력만—일관되게 막는 쪽 권장

    line("COPY / ASSIGNMENT (정형적 복사/대입 검증)");
    FragTrap a("Alpha");
    FragTrap b("Beta");

    a.attack("X");                 // 상태 변화(EP 감소) 발생시켜 복사 후 비교 재료 생성
    a.beRepaired(1);

    line("COPY CONSTRUCT");
    FragTrap a_copy(a);            // 복사 생성자 호출 메시지 확인
    a_copy.attack("X-copy");

    line("ASSIGNMENT");
    b = a;                         // 대입 연산자 호출 메시지 확인
    b.attack("X-assigned");

    line("STRESS MIX");
    FragTrap g("GLAD");
    g.highFivesGuys();
    g.attack("t1");
    g.takeDamage(99);
    g.beRepaired(5);
    g.takeDamage(10);              // 죽일 수도 있음
    g.attack("t2");                // HP/EP 상태에 따라 성공/실패 확인

    line("END");
    return 0;
}
