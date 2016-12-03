
#include <glib.h>
#include <glib-unix.h>
#include <stdio.h>
#include <stdlib.h>
#include "helpers.h"


char * hex_to_bitmap(char hex_digit) {
    uint8_t digit;
    if((hex_digit >= '0') && (hex_digit <= '9')) {
        digit = hex_digit - '0';
    } else if((hex_digit >= 'a') && (hex_digit <= 'f')) {
        digit = hex_digit - 'a' + 10;
    } else if((hex_digit >= 'A') && (hex_digit <= 'F')) {
        digit = hex_digit - 'A' + 10;
    } else return "0000\0";

    char *bitmap = malloc(5 * sizeof(char));
    bitmap[4] = '\0';
    for(int i = 3; i >= 0; i--) {
        bitmap[i] = digit % 2 ? '1' : '0';
        digit /= 2;
    }
    return bitmap;
}

void for_each_banned_cpu(GList *list,
        void (*fp)(uint64_t *number, void *data), void *data)
{
    GList *entry, *next;
    entry = g_list_first(list);
    while(entry) {
        next = g_list_next(entry);
        fp(entry->data, data);
        entry = next;
    }
}

void for_each_irq(GList *list,
        void (*fp)(irq_t *irq, void *data), void *data)
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

