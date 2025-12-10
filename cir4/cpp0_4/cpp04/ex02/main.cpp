#include <iostream>
#include "Animal.hpp"
#include "Dog.hpp"
#include "Cat.hpp"

int main() {
	/*/
	Animal* test = new Animal;
	//*/
    std::cout << "\n=== [ 기본 생성 테스트 ] ===\n";
    Cat c1;
    Dog d1;

    c1.makeSound();
    d1.makeSound();

    std::string test0 = c1.getType();
    std::cout << "Cat Type: " << test0 << "\n";
    std::string test00 = d1.getType();
    std::cout << "Dog Type: " << test00 << "\n";

    std::cout << "\n=== [ set/get Brain 테스트 ] ===\n";
    c1.setNewid(0, "sleep on keyboard");
    c1.setNewid(1, "chase red dot");
    d1.setNewid(0, "protect the house");
    d1.setNewid(1, "bark at mailman");

    std::string test01 = c1.getOldid(50);
    std::cout << "c1 Brain[50]: " << test01 << "\n";

    std::string test1 = c1.getOldid(0);
    std::cout << "c1 Brain[0]: " << test1 << "\n";
    std::string test2 = c1.getOldid(1);
    std::cout << "c1 Brain[1]: " << test2 << "\n";
    std::string test3 = d1.getOldid(0);
    std::cout << "d1 Brain[0]: " << test3 << "\n";
    std::string test4 = d1.getOldid(1);
    std::cout << "d1 Brain[1]: " << test4 << "\n";

    std::cout << "\n=== [ 복사 생성자 테스트 (깊은 복사) ] ===\n";
    Cat c2(c1);
    Dog d2(d1);

    std::string test5 = c1.getOldid(0);
    std::string test6 = c2.getOldid(0);
    std::cout << "Before modify c1[0]: " << test5 << " | c2[0]: " << test6 << "\n";

    std::string test7 = d1.getOldid(0);
    std::string test8 = d2.getOldid(0);
    std::cout << "Before modify d1[0]: " << test7 << " | d2[0]: " << test8 << "\n";

    c1.setNewid(0, "knock down water cup");
    d1.setNewid(0, "dig a hole in garden");

    std::string test9 = c1.getOldid(0);
    std::string test10 = c2.getOldid(0);
    std::cout << "After modify c1[0]: " << test9 << " | c2[0]: " << test10 << "\n";

    std::string test11 = d1.getOldid(0);
    std::string test12 = d2.getOldid(0);
    std::cout << "After modify d1[0]: " << test11 << " | d2[0]: " << test12 << "\n";

    std::cout << "\n=== [ 대입 연산자 테스트 ] ===\n";
    Cat c3;
    Dog d3;

    c3 = c1;
    d3 = d1;

    std::string test13 = c3.getOldid(0);
    std::cout << "c3 Brain[0]: " << test13 << "\n";
    std::string test14 = d3.getOldid(0);
    std::cout << "d3 Brain[0]: " << test14 << "\n";

	std::cout << "\n=== [ 자기 대입 테스트 ] ===\n";
	Cat* aliasC3 = &c3;
	Dog* aliasD3 = &d3;

	c3 = *aliasC3;   // 실제로는 자기 대입, 경고 없음
	d3 = *aliasD3;   // 실제로는 자기 대입, 경고 없음

	std::cout << "Self assignment test done.\n";

    std::cout << "\n=== [ 다형성 테스트 (Animal 포인터 배열) ] ===\n";
    const int N = 4;
    Animal* animals[N];

    for (int i = 0; i < N; ++i) {
        if (i % 2)
            animals[i] = new Dog();
        else
            animals[i] = new Cat();
    }

    std::cout << "\n-- makeSound() 확인 --\n";
    for (int i = 0; i < N; ++i)
        animals[i]->makeSound();

    std::cout << "\n-- delete (가상 소멸자 확인) --\n";
    for (int i = 0; i < N; ++i)
        delete animals[i];

    std::cout << "\n=== [ 테스트 종료 ] ===\n";
    return 0;
}

