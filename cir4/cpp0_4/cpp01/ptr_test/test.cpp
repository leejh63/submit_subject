#include <cstring>
#include <iostream>

// 매우 위험: 구현에 따라 동작/크래시/보안문제 발생
void force_overwrite(std::string &s, const char *newbytes, std::size_t n) {
    // 객체 시작 주소에서 첫 8바이트를 '데이터 포인터'로 읽음 (구현 의존)
    void *data_ptr = *reinterpret_cast<void**>(&s);
    if (data_ptr) {
        // 예: 동일 길이일 때만 안전하다고 기대하며 덮음
        memcpy(data_ptr, newbytes, n);
    }
}

int main() {
	std::string test = "abcdd";
    std::cout << test << "\n";      // abcdd
    force_overwrite(test, "12345", 5);
    std::cout << test << "\n";      // 보일 수도 있음 ("12345"), 혹은 크래시/쓰레기
}
