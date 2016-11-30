
#ifndef IRQBALANCE_UI_H
#define IRQBALANCE_UI_H

#include <glib.h>
#include <glib-unix.h>

#define SOCKET_PATH "/var/run/irqbalance.sock"

GList *tree = NULL;

/* typedefs */

typedef enum node_type {
    OBJ_TYPE_CPU,
    OBJ_TYPE_CACHE,
    OBJ_TYPE_PACKAGE,
    OBJ_TYPE_NODE
} node_type_e;

typedef struct irq {
    uint64_t vector;
    uint64_t load;
    uint64_t diff;
} irq_t;

typedef struct cpu_node {
    node_type_e type;
    int number;
    uint64_t load;
    uint8_t is_powersave;
    struct cpu_node *parent;
    GList *children;
    GList *irqs;
} cpu_node_t;


/* helper functions */

void for_each_irq(GList *list,
        void (*fp)(irq_t *node, void *data), void *data)
{
    GList *entry, *next;
    entry = g_list_first(list);
    while(entry) {
        next = g_list_next(entry);
        fp(entry->data, data);
        entry = next;
    }
}

void for_each_node(GList *list,
        void (*fp)(cpu_node_t *node, void *data), void *data)
{
    GList *entry, *next;
    entry = g_list_first(list);
    while(entry) {
        next = g_list_next(entry);
        fp(entry->data, data);
        entry = next;
    }
}


/* programmer debugging functions */

void dump_irq(irq_t *irq, void *data __attribute__((unused)))
{
    printf("IRQ %lu\n", irq->vector);
}

void dump_node(cpu_node_t *node, void *data __attribute__((unused)))
{
    printf("TYPE %d NUMBER %d\n", node->type, node->number);
    if(g_list_length(node->irqs) > 0) {
        for_each_irq(node->irqs, dump_irq, NULL);
    }
    if(g_list_length(node->children) > 0) {
        for_each_node(node->children, dump_node, NULL);
    }
}

void dump_tree()
{
    for_each_node(tree, dump_node, NULL);
}

#endif /* IRQBALANCE_UI_H */
