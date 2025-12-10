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

#include "Bureaucrat.hpp"
#include "ShrubberyCreationForm.hpp"
#include "RobotomyRequestForm.hpp"
#include "PresidentialPardonForm.hpp"
#include <iostream>

int main() {

    std::cout << "=== Create Bureaucrats ===" << std::endl;
    Bureaucrat high("Zaphod", 1);
    Bureaucrat mid("Trillian", 50);
    Bureaucrat low("Arthur", 150);
    std::cout << std::endl;

    std::cout << "=== Create Forms ===" << std::endl;
    ShrubberyCreationForm shrub("home");
    RobotomyRequestForm robot("Marvin");
    PresidentialPardonForm pardon("Ford Prefect");
    std::cout << std::endl;

    std::cout << "=== Try to sign with low grade ===" << std::endl;
    low.signForm(shrub);  // 실패
    std::cout << std::endl;

    std::cout << "=== mid signs shrubbery form ===" << std::endl;
    mid.signForm(shrub);  // 성공
    std::cout << std::endl;

    std::cout << "=== Execute shrub form ===" << std::endl;
    mid.executeForm(shrub);  // 실행 가능
    std::cout << std::endl;

    std::cout << "=== high signs and executes robot form ===" << std::endl;
    high.signForm(robot);
    for (int i = 0; i < 10; ++i) {
	    high.executeForm(robot);  // 여러 번 실행 시도 (랜덤 성공/실패)
	    std::cout << std::endl;
    }
    std::cout << std::endl;

    std::cout << "=== high signs and executes pardon form ===" << std::endl;
    high.signForm(pardon);
    high.executeForm(pardon);
    std::cout << std::endl;

    std::cout << "=== low tries to execute high-level form ===" << std::endl;
    low.executeForm(pardon); // 실패 (등급 부족)

    return 0;
}
