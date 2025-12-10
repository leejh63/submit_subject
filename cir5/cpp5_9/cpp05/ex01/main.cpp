/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/05 18:31:19 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/11/05 18:31:20 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "Bureaucrat.hpp"
#include "Form.hpp"

int main() {
    std::cout << "========== FORM / BUREAUCRAT TEST ==========\n\n";

    try {
        std::cout << "[Test 1] 잘못된 Form 생성 (등급 0)\n";
        Form f1("InvalidZero", 0, 50);   // signGrade가 너무 높음
    }
    catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << "\n\n";
    }

    try {
        std::cout << "[Test 2] 잘못된 Form 생성 (등급 200)\n";
        Form f2("Invalid200", 200, 50);  // signGrade가 너무 낮음
    }
    catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << "\n\n";
    }

    try {
        std::cout << "[Test 3] 음수 등급 Form 생성\n";
        Form f3("NegativeGrade", -10, -5);
    }
    catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << "\n\n";
    }

    Bureaucrat alice("Alice", 50);
    Bureaucrat bob("Bob", 120);
    Bureaucrat ceo("CEO", 1);

    Form permit("Permit", 100, 50);
    Form contract("Contract", 40, 30);

    std::cout << "[Test 4] 정상 객체 상태 출력\n";
    std::cout << permit << "\n";
    std::cout << contract << "\n";

    std::cout << "[Test 5] Bob이 Permit 서명 시도 (실패 예상)\n";
    bob.signForm(permit);
    std::cout << permit << "\n";

    std::cout << "[Test 6] Alice가 Permit 서명 시도 (성공 예상)\n";
    alice.signForm(permit);
    std::cout << permit << "\n";

    std::cout << "[Test 7] Alice가 이미 서명된 문서에 다시 서명 시도\n";
    alice.signForm(permit);

    std::cout << "[Test 8] Alice가 Contract 서명 시도 (실패 예상)\n";
    alice.signForm(contract);

    std::cout << "[Test 9] CEO가 Contract 서명 시도 (성공 예상)\n";
    ceo.signForm(contract);
    std::cout << contract << "\n";

    std::cout << "\n========== TEST END ==========\n";
}

