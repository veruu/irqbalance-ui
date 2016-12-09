
#ifndef UI_H
#define UI_H

#include <glib.h>
#include <glib-unix.h>
#include <curses.h>
#include <form.h>
#include <ncurses.h>
#include <signal.h>

#include "irqbalance-ui.h"
#include "helpers.h"

extern GList *tree;
extern setup_t setup;

void show_footer();
void add_banned_cpu(uint64_t *banned_cpu, void *data);
void display_banned_cpus();
char * check_control_in_sleep_input(int max_len,
                                    int column_offest,
                                    int line_offset);
int get_valid_sleep_input(int column_offest);
int toggle_cpu(GList *cpu_list, int cpu_number);
void get_new_ban_values(cpu_ban_t *cpu, void *data);
void handle_banning();
void init();
void get_cpu();
void print_cpu_line(cpu_ban_t *cpu, void *data;);
void print_all_cpus();
void close_window(int sig);
void settings();
void setup_irqs();
void display_tree_node_irqs(irq_t *irq, void *data);
void display_tree_node(cpu_node_t *node, void *data);
void display_tree();


#endif /* UI_H */
