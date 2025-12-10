/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Harl.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/16 11:31:52 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/16 11:31:52 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Harl.hpp"
#include <iomanip>

// private
void Harl::debug( void ) {
	std::cout << "[ DEBUG ]\nI love having extra bacon "
			  << "for my 7XL-double-cheese-"
			  << "triple-pickle-special-ketchup burger."
			  << "I really do!\n"; 
}

void Harl::info( void ) {
	std::cout << "[ INFO ]\nI cannot believe adding extra "
			  << "bacon costs more money. "
			  << "You didn’t put enough bacon in my burger! "
			  << "If you did, I wouldn’t be asking for more!\n"; 
}

void Harl::warning( void ) {
	std::cout << "[ WARNING ]\nI think I deserve to have some extra bacon for free. "
			  << "I’ve been coming for years, "
			  << "whereas you started working here just last month.\n";
}

void Harl::error( void ) {
	std::cout << "[ ERROR ]\nThis is unacceptable! "
			  << "I want to speak to the manager now.\n";
}

void Harl::dump_from_claim_level(uintptr_t addr) {
    // 1️⃣ &claim_level[4]에서 level과 this 주소 추출
    const void** fake_slot = reinterpret_cast<const void**>(addr);
    
    // 몇 개 보여줄지: 적당히 4개(첫 qword, 둘째 qword(this), 그 다음 등)
    const int N = 2; // 필요하면 늘려라
    std::cout << "dump at " << std::hex << addr << ":\n";
    for (int i = 0; i < N; ++i) {
        const void* v = fake_slot[i];
        // 주소와 8바이트 값을 헥사로 출력
        std::cout << " [" << i << "] " 
                  << std::setw(2) << i << ": "
                  << std::hex << std::setw(18) << v
                  << "  (as uint64 = 0x" 
                  << std::setw(16) << reinterpret_cast<uint64_t>(v) << ")\n";
    }
    std::cout << std::dec; // 복구
    
    const std::string* level_ptr = reinterpret_cast<const std::string*>(fake_slot[0]);
    Harl* this_ptr = reinterpret_cast<Harl*>(const_cast<void*>(fake_slot[1]));

    // 2️⃣ level 문자열 출력
    std::cout << "[via claim_level[4]] level: " << level_ptr->c_str() << "\n";

    // 3️⃣ this 객체의 주소 확인
    std::cout << "[via claim_level[4]] this: " << this_ptr << "\n";
	this_ptr->complain("INFO");
    // 4️⃣ (예시) this 객체 멤버 접근 — Harl 내부 구조를 정확히 알고 있어야 함
    // 예: 만약 Harl 클래스에 std::string name; 이 있다고 가정하면
    // std::cout << this_ptr->name << "\n";

    // 5️⃣ (예시) 수정도 가능하지만 절대 안전하지 않음
    // this_ptr->name = "injected";
}

//
void Harl::use_for( const std::string& level ) {
	const std::string claim_level[] = {
		"DEBUG", "INFO", "WARNING", "ERROR"
	};
	
	void (Harl::*func[]) ( void ) = {
		&Harl::debug, 
		&Harl::info, 
		&Harl::warning, 
		&Harl::error
	};
	std::cout << &Harl::use_for << "<< &Harl::use_for\n";
	std::cout << &level << "<< level\n";
	for (int i = 0; i < 5; ++i) {
		std::cout << &claim_level[i] << "<< &claim_level[" << i << "]\n";
		if (i == 4) { dump_from_claim_level(reinterpret_cast<uintptr_t>(&claim_level[i])); }
		if (claim_level[i] == level) {
			(this->*func[i])(); return;
		}	
	}
	std::cout << "Harl completely ignored you\n";
}
/*/
void Harl::use_switch( const std::string& level ) {
	const std::string claim_level[] = {
		"DEBUG", "INFO", "WARNING", "ERROR"
	};
	
	for (int i = 0; i < 4; ++i) {
		if (claim_level[i] == level) {
			switch (i) {
				case 0 : debug();
				case 1 : info();
				case 2 : warning();
				case 3 : error(); return;
				//default : return;
			}
		}
	}
	std::cout << "Harl completely ignored you\n";
}
//*/

// public
void Harl::complain( std::string level ) {
	//
	use_for(level);
	/*/
	use_switch(level);
	//*/
}
