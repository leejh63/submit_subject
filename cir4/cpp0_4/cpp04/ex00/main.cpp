#include "Animal.hpp"
#include "Dog.hpp"
#include "Cat.hpp"

#include "WrongAnimal.hpp"
#include "WrongCat.hpp"
#include "WrongDog.hpp"

#include <iostream>

int main() {
    std::cout << "\n=== [ Basic polymorphism test ] ===\n";
    const Animal* ani = new Animal();
    const Animal* dog = new Dog();
    const Animal* cat = new Cat();

    std::cout << "\n[Type check]\n";
    std::cout << "Animal type: " << ani->getType() << std::endl;
    std::cout << "Dog type: " << dog->getType() << std::endl;
    std::cout << "Cat type: " << cat->getType() << std::endl;

    std::cout << "\n[Sound test]\n";
    ani->makeSound();
    cat->makeSound();
    dog->makeSound();

    std::cout << "\n[Destructor test]\n";
    delete ani;
    delete dog;
    delete cat;

    std::cout << "\n=== [ Wrong class test ] ===\n";
    const WrongAnimal* wani = new WrongAnimal();
    const WrongAnimal* wcat = new WrongCat();
    const WrongAnimal* wdog = new WrongDog();

    std::cout << "\n[Type check]\n";
    std::cout << "WrongAnimal type: " << wani->getType() << std::endl;
	std::cout << "WrongDog type: " << wdog->getType() << std::endl;
    std::cout << "WrongCat type: " << wcat->getType() << std::endl;
	
    std::cout << "\n[Sound test]\n";
    wani->makeSound();
    wdog->makeSound();
    wcat->makeSound();

    std::cout << "\n[Destructor test]\n";
    delete wani;
    delete wdog;
    delete wcat;

    std::cout << "\n=== [ Deep test for copy ] ===\n";
    Dog dog1;
    Dog dog2 = dog1;
    dog2.makeSound();

    std::cout << "\n=== [ Self assignment test ] ===\n";
    Dog d_other;
    Dog* test = &d_other;
    *test = *test;

    return 0;
}

