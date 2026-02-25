// ui_model.c
#include "ui_model.h"
#include <string.h>

void ui_model_init(ui_model_t *m)
{
    memset(m, 0, sizeof(*m));
    strncpy(m->msg, "ready", sizeof(m->msg) - 1);
}

void ui_model_set_msg(ui_model_t *m, const char *s)
{
    size_t n = strlen(s);
    if (n >= sizeof(m->msg)) n = sizeof(m->msg) - 1;
    memcpy(m->msg, s, n);
    m->msg[n] = '\0';
}
