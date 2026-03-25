/*
 * 역할별 펌웨어 이미지의 프로그램 시작점이다.
 * 실행 파일은 최대한 얇게 두고, 보드 초기화와
 * task 등록, 역할별 동작은 runtime 계층에서 맡는다.
 */
#include "runtime/runtime.h"

int main(void)
{
    if (Runtime_Init() != INFRA_STATUS_OK)
    {
        for (;;)
        {
        }
    }

    Runtime_Run();
    return 0;
}
