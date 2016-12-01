
#include <string.h>
#include "ui.h"


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

void display_matrix()
{

}

void display_tree_node(cpu_node_t *node, void *data)
{
    char spaces[32] = "\0";
    for(int i = node->type; i <= OBJ_TYPE_NODE; i++) {
        snprintf(spaces + strlen(spaces), 32, "%s", (char *)data);
    }
    char copy_to[1024];
    snprintf(copy_to, 1024, "%sTYPE %d NUMBER %d\n",
            spaces, node->type, node->number);
    attrset(COLOR_PAIR(2));
    addstr(copy_to);
    if(g_list_length(node->children)) {
        for_each_node(node->children, display_tree_node, data);
    }
    refresh();
}

void display_tree()
{
    char *indent = "    \0";
    for_each_node(tree, display_tree_node, indent);
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
