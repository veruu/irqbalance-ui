
#include <string.h>
#include "ui.h"


const char *node_type_to_str[] = {"CPU\0",
                                  "CACHE DOMAIN\0",
                                  "CPU PACKAGE\0",
                                  "NUMA NODE\0"};


void close_window(int sig)
{
    endwin();
    exit(EXIT_SUCCESS);
}


/*void show(cpu_node_t *node, void *data __attribute__((unused)))
{
    char copy_to[100];
    snprintf(copy_to, 100, "TYPE %d NUMBER %d\n", node->type, node->number);
    attrset(COLOR_PAIR(2));
    addstr(copy_to);
    //addstr("TYPE %d NUMBER %d\n", node->type, node->number);
    if(g_list_length(node->children)) {
        for_each_node(node->children, show, NULL);
    }
    refresh();
}*/

void settings()
{

}

void setup_irqs()
{

}

void display_matrix()
{

}

void display_tree_node_irqs(irq_t *irq, void *data)
{
    char indent[32] = "      \0";
    snprintf(indent + strlen(indent), 32 - strlen(indent), "%s", (char *)data);
    attrset(COLOR_PAIR(3));
    char copy_to[256];
    snprintf(copy_to, 256, "%sIRQ %lu, IRQs since last rebalance %lu\n",
            indent, irq->vector, irq->diff);
    addstr(copy_to);
}

void display_tree_node(cpu_node_t *node, void *data __attribute__((unused)))
{
    char *spaces = "   \0";
    char indent[32] = "\0";
    char *asciitree = "--\0";
    for(int i = node->type; i <= OBJ_TYPE_NODE; i++) {
        snprintf(indent + strlen(indent), 32 - strlen(indent), "%s", spaces);
        if(i != OBJ_TYPE_NODE) {
            snprintf(indent + strlen(indent), 32 - strlen(indent), "   ");
        }
    }
    snprintf(indent + strlen(indent), 32 - strlen(indent),
            "%s", (char *)asciitree);
    char copy_to[1024];
    char *numa_available = "\0";
    if((node->type == OBJ_TYPE_NODE) && (node->number == -1)) {
        numa_available = " (This machine is not NUMA-capable)";
    }
    snprintf(copy_to, 1024, "%s%s, number %d%s\n",
            indent, node_type_to_str[node->type], node->number, numa_available);
    attrset(COLOR_PAIR(2));
    addstr(copy_to);
    if(g_list_length(node->irqs) > 0) {
        for_each_irq(node->irqs, display_tree_node_irqs, indent);
    }
    if(g_list_length(node->children)) {
        for_each_node(node->children, display_tree_node, data);
    }
    refresh();
}

void add_banned_cpu(uint64_t *banned_cpu, void *data)
{
    snprintf(data + strlen(data), 1024 - strlen(data), "%lu, ", *banned_cpu);
}

void display_tree()
{
    char banned_cpus[1024] = "Banned CPU numbers: \0";
    for_each_banned_cpu(setup.banned_cpus, add_banned_cpu, banned_cpus);
    snprintf(banned_cpus + strlen(banned_cpus) - 2,
             1024 - strlen(banned_cpus), "\n");
    attrset(COLOR_PAIR(1));
    addstr(banned_cpus);
    for_each_node(tree, display_tree_node, NULL);
}

void init()
{
    signal(SIGINT, close_window);
    initscr();
    scrollok(stdscr, TRUE);
    keypad(stdscr, TRUE);
    nonl();
    cbreak();
    echo();
    if(has_colors()) {
        start_color();
        init_pair(1, COLOR_RED, COLOR_BLACK);
        init_pair(2, COLOR_YELLOW, COLOR_BLACK);
        init_pair(3, COLOR_GREEN, COLOR_BLACK);
    }

    //display_matrix();
    display_tree();
}
