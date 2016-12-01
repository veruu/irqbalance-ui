
#include <glib.h>
#include <glib-unix.h>
#include <stdio.h>
#include "helpers.h"


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

