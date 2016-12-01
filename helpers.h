
#ifndef HELPERS_H
#define HELPERS_H

#include "irqbalance-ui.h"

extern GList *tree;

/* helper functions */

void for_each_irq(GList *list, void (*fp)(irq_t *node, void *data), void *data);
void for_each_node(GList *list,
        void (*fp)(cpu_node_t *node, void *data), void *data);

/* programmer debugging functions */

void dump_irq(irq_t *irq, void *data __attribute__((unused)));
void dump_node(cpu_node_t *node, void *data __attribute__((unused)));
void dump_tree();


#endif /* HELPERS_H */