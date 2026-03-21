#ifndef EMB_CRITICAL_H
#define EMB_CRITICAL_H

/*
 * 프로젝트마다 IRQ 전역 차단/복구 방식이 다르므로,
 * 필요하면 빌드 설정이나 플랫폼 헤더에서 아래 매크로를 override 해서 쓴다.
 * 기본값은 no-op 이다.
 */

typedef unsigned int EmbCriticalState;

#ifndef EMB_ENTER_CRITICAL
#define EMB_ENTER_CRITICAL(state) do { (void)(state); } while (0)
#endif

#ifndef EMB_EXIT_CRITICAL
#define EMB_EXIT_CRITICAL(state)  do { (void)(state); } while (0)
#endif

#endif
