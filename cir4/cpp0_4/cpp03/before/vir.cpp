#include <iostream>
#include <cstdint>

struct Base {
    virtual void speak() { std::cout << "Base::speak\n"; }
    virtual void run()   { std::cout << "Base::run\n"; }
    virtual ~Base()      { std::cout << "Base::~Base\n"; }
};

struct Derived : Base {
    void speak() override { std::cout << "Derived::speak\n"; }
    void run()   override { std::cout << "Derived::run\n"; }
    ~Derived() override   { std::cout << "Derived::~Derived\n"; }
};

template <typename T>
void dump_vptr_vtable(T& obj) {
    void** vptr = *reinterpret_cast<void***>(&obj);
    std::cout << "obj @" << &obj << "\n";
    std::cout << "vptr=" << vptr << "\n";
    // Itanium C++ ABI 전제(GCC/Clang, Linux/WSL/macOS):
    // vtable[-2] = typeinfo, vtable[-1] = offset-to-top, vtable[0..] = 가상함수들
    std::cout << "vtable[-2](typeinfo)=" << *(vptr - 2) << "\n";
    std::cout << "vtable[-1](offset)  =" << *(vptr - 1) << "\n";
    for (int i = 0; i < 4; ++i)
        std::cout << "vtable[" << i << "]=" << vptr[i] << "\n";
}

int main() {
    Derived d;
    Base* p = &d;
    std::cout << "--- dynamic calls ---\n";
    p->speak();
    p->run();
    std::cout << "--- dump vptr/vtable ---\n";
    dump_vptr_vtable(d);
}

