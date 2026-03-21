#ifndef EMB_SLIST_H
#define EMB_SLIST_H

typedef struct emb_slist_node
{
    struct emb_slist_node *next;
} emb_slist_node_t;

static inline void emb_slist_push_front(emb_slist_node_t **head, emb_slist_node_t *node)
{
    if ((head == 0) || (node == 0))
        return;

    node->next = *head;
    *head = node;
}

static inline emb_slist_node_t *emb_slist_pop_front(emb_slist_node_t **head)
{
    emb_slist_node_t *node;

    if ((head == 0) || (*head == 0))
        return 0;

    node = *head;
    *head = node->next;
    node->next = 0;
    return node;
}

#endif
