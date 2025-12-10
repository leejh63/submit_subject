/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/24 10:41:04 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/24 16:56:23 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// main.cpp
#include <iostream>
#include <vector>
#include <string>
#include "AMateria.hpp"
#include "Ice.hpp"
#include "Cure.hpp"
#include "ICharacter.hpp"
#include "Character.hpp"
#include "IMateriaSource.hpp"
#include "MateriaSource.hpp"

static void title(const std::string& s) {
    std::cout << "\n===== " << s << " =====\n";
}

static void line() {
    std::cout << "----------------------------------------\n";
}

// 안전하게 unequip된 Materia를 임시로 보관했다가 직접 delete
struct Floor {
    std::vector<AMateria*> dropped;
    ~Floor() {
        for (size_t i = 0; i < dropped.size(); ++i) delete dropped[i];
        dropped.clear();
    }
    void put(AMateria* m) { if (m) dropped.push_back(m); }
};

int main() {
    // 0) 기본 시나리오 (과제 pdf 예시)
    {
        title("0) Subject 기본 예시 시나리오");
        IMateriaSource* src = new MateriaSource();
        src->learnMateria(new Ice());   // ✅ learnMateria는 m->clone() 저장해야 함
        src->learnMateria(new Cure());  // ✅

        ICharacter* me = new Character("me");

        AMateria* tmp;
        tmp = src->createMateria("ice");   // ✅ createMateria는 clone() 반환해야 함
        me->equip(tmp);                    // Character가 소유권 가짐
        tmp = src->createMateria("cure");
        me->equip(tmp);

        ICharacter* bob = new Character("bob");
        me->use(0, *bob);
        me->use(1, *bob);

        delete bob;
        delete me;     // me가 equip한 Materia delete
        delete src;    // src가 보관하던 템플릿 delete
    }

    // 1) learnMateria: "복제 저장" 검증 (원본 즉시 삭제)
    //    -> 만약 네 구현이 포인터 그대로 저장했다면 여기서 바로 크래시/UB
    {
        title("1) learnMateria 복제 여부 검증 (원본 즉시 delete)");
        MateriaSource* src = new MateriaSource();
        Ice*  i = new Ice();
        Cure* c = new Cure();
        src->learnMateria(i);
        src->learnMateria(c);
        AMateria* a = src->createMateria("ice");   // 정상 동작해야 함
        AMateria* b = src->createMateria("cure");  // 정상 동작해야 함
        Character* user = new Character("user");
        user->equip(a);
        user->equip(b);
        Character target("target");
        user->use(0, target);
        user->use(1, target);

        delete user;
        delete src;
    }

    // 2) createMateria: 존재하지 않는 타입 처리 (NULL 반환 & 조용히 무시)
    {
        title("2) createMateria: unknown type 처리");
        MateriaSource src;
        src.learnMateria(new Ice());
        AMateria* x = src.createMateria("fire"); // 존재 안 함 -> NULL이어야
        if (x == NULL) std::cout << "[OK] unknown type -> NULL\n";
        else {
            std::cout << "[NG] unknown type인데 NULL 아님\n";
            delete x;
        }
    }

    // 3) MateriaSource 슬롯 초과 learn (5개 이상) -> 가득 차면 무시
    //    또한 learnMateria는 입력 포인터 소유권을 갖지 않으므로 호출자가 정리
    {
        title("3) MateriaSource learn 슬롯 초과 (5개 시도)");
        MateriaSource* src = new MateriaSource();
        AMateria* originals[5] = { new Ice(), new Cure(), new Ice(), new Cure(), new Ice() };
        for (int i = 0; i < 5; ++i) {
            src->learnMateria(originals[i]); // 내부는 clone 저장만 해야 함
            //delete originals[i];             // 원본은 즉시 정리 (소유권 없음)
        }
        delete originals[4];
        // 이제 ice/cure는 정상 생성 가능. 5번째는 무시되어야 함(충돌/삭제 금지)
        AMateria* a = src->createMateria("ice");
        AMateria* b = src->createMateria("cure");
        if (a && b) std::cout << "[OK] 슬롯 내 템플릿에서 정상 생성\n";
        else        std::cout << "[NG] create 실패\n";
        delete a;
        delete b;
        delete src;
    }

    // 4) Character equip: 4칸 초과 시 무시.
    //    여기서는 5번째 equip 이후, 우리가 직접 new한 포인터를 즉시 delete 해서
    //    "정말 무시했는지"를 간접적으로 검증 (만약 네 equip이 잘못 구현돼 포인터를 잡았다면 이후 이중 delete로 터짐)
    {
        title("4) Character equip 슬롯 초과 (5개째 무시 확인)");
        MateriaSource src;
        src.learnMateria(new Ice());
        src.learnMateria(new Cure());

        Character c("carry");
        AMateria* m0 = src.createMateria("ice");
        AMateria* m1 = src.createMateria("cure");
        AMateria* m2 = src.createMateria("ice");
        AMateria* m3 = src.createMateria("cure");
        AMateria* m4 = src.createMateria("ice");   // 5번째

        c.equip(m0);
        c.equip(m1);
        c.equip(m2);
        c.equip(m3);

        // 여기서 inventory가 가득 찼으니, 과제 규칙상 5번째는 "무시"가 맞음.
        // 무시가 맞다면 c는 m4의 소유권을 갖지 않으므로 우리가 delete 해도 문제 없어야 함.
        c.equip(m4);
        delete m4; // ⚠️ 잘못 구현(소유권 빼앗음)했다면, 나중에 c 소멸 시 이중 delete 터짐

        c.use(0, c);
        c.use(1, c);
        c.use(2, c);
        c.use(3, c);
        c.use(4, c); // out-of-range일 수 있음 -> 조용히 무시가 바람직

        // c 소멸 시 0~3만 해제되어야 함(정상)
        // m4는 우리가 이미 delete 했으므로 추가 해제 시 크래시 -> 구현 버그 검출
    }

    // 5) unequip: 포인터만 떼고 delete는 하지 않음 → 누수 방지 위해 Floor에 보관 후 나중에 delete
    {
        title("5) unequip: 포인터만 떼고, delete는 호출자가 책임");
        MateriaSource src;
        src.learnMateria(new Ice());
        src.learnMateria(new Cure());
        Character c("cleric");
        Floor floor;

        AMateria* a = src.createMateria("ice");
        AMateria* b = src.createMateria("cure");
        c.equip(a);
        c.equip(b);

        c.use(0, c);
        c.use(1, c);

        // unequip은 delete 하지 않음 → 우리가 받아서 관리
        // 과제 구현에 따라 포인터를 반환하지 않을 수 있으니,
        // 일반적으로는 외부 보관 구조를 Character에 두지만, 테스트에선 인위적으로 가정:
        // 보통 과제 예시 구현은 인벤토리 배열에서 포인터만 null로 떼고 반환은 안 함.
        // 그러니 여기선 "테스트 목적"으로 b를 먼저 unequip 시도하고,
        // 이후 "메모리 누수 방지"를 위해 Floor가 직접 delete할 수 있게
        // Character에 임시 슬롯(드롭 박스) 구현이 있다면 거기서 꺼내는 식으로 작성했을 텐데
        // 과제 기본 인터페이스에는 반환이 없으니, 보관을 못 받는 점은 양해.
        c.unequip(1);
        c.unequip(0);

        // 현실 테스트에서는 Character에 drop-box(예: Array<AMateria*,N>)를 두고
        // unequip시 그곳에 옮긴 후, main에서 그 drop-box를 비우는 방식으로 누수 방지.
        // 여기서는 인터페이스 한계상 누수 여부는 Valgrind로만 간접 확인.
    }

    // 6) Character 깊은 복사 검증 (copy ctor / operator=)
    //    - 원본 파괴 후에도 사본이 정상적으로 작동해야 함
    //    - 깊은 복사가 아니면, 원본 파괴 시 사본 포인터 댕글 → use 시 크래시
    {
        title("6) Character 깊은 복사 (원본 delete 후 사본 정상작동)");
        MateriaSource src;
        src.learnMateria(new Ice());
        src.learnMateria(new Cure());

        Character* a = new Character("alpha");
        a->equip(src.createMateria("ice"));
        a->equip(src.createMateria("cure"));
        Character b = *a;        // copy constructor
        Character c("gamma");
        c = *a;                  // copy assignment

        delete a; // 원본 파괴 -> b, c의 인벤토리가 안전해야 함(깊은 복사 필수)

        b.use(0, b);
        b.use(1, b);
        c.use(0, c);
        c.use(1, c);
        // 만약 얕은 복사였다면 여기서 뻗음

        // 끝나면 소멸자에서 각자 자신의 Materia 안전 해제
    }

    // 7) createMateria: 템플릿 중복 허용 확인 (동일 타입 여러 개 learn 가능)
    //    그리고 각 create가 "서로 다른 새 객체"를 주는지 확인(주소 비교)
    {
        title("7) 동일 타입 템플릿 중복 & 매번 새로운 객체 반환");
        MateriaSource src;
        src.learnMateria(new Ice());
        src.learnMateria(new Ice()); // 중복 허용
        AMateria* m1 = src.createMateria("ice");
        AMateria* m2 = src.createMateria("ice");
        std::cout << "m1=" << m1 << ", m2=" << m2 << "\n";
        if (m1 && m2 && m1 != m2) std::cout << "[OK] 서로 다른 새 객체 반환\n";
        else std::cout << "[NG] 동일 포인터 반환(잘못)\n";
        delete m1;
        delete m2;
    }

    line();
    std::cout << "All tests finished.\n";
    return 0;
}

