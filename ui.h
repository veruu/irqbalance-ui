
#ifndef UI_H
#define UI_H

#include <glib.h>
#include <glib-unix.h>
#include <curses.h>
#include <ncurses.h>
#include <signal.h>

#include "irqbalance-ui.h"
#include "helpers.h"

extern GList *tree;
extern setup_t setup;

void init();
void close_window(int sig);
void settings();
void setup_irqs();
//void show(cpu_node_t *node, void *data __attribute__((unused)));
void display_matrix();
void display_tree_node(cpu_node_t *node, void *data);
void display_tree();


#endif /* UI_H */
