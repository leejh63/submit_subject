import random
import sys

def generate_random_integers(n):
    if n <= 0:
        print("양의 정수를 입력해주세요.")
        return
    
    # 정수형 범위 내에서 랜덤 값 생성 (-2^31 ~ 2^31-1)
    random_numbers = []
    for i in range(n):
        random_num = random.randint(-2147483648, 2147483647)
        random_numbers.append(random_num)
    
    # "...a b c d..." 형태로 출력
    result = " ".join(map(str, random_numbers))
    print(result)

# 프로그램 실행
if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("사용법: python 파일명.py <정수개수>")
        print("예시: python random_gen.py 5")
        sys.exit(1)
    
    try:
        n = int(sys.argv[1])
        generate_random_integers(n)
    except ValueError:
        print("올바른 정수를 입력해주세요.")
        sys.exit(1)
