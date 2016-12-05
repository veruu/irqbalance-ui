
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

void show_footer()
{
    char footer[COLS];
    snprintf(footer, COLS - 1,
             "q (QUIT)   F3 (TREE)   F4 (SETTINGS)   F5 (SETUP IRQS)");
    while(strlen(footer) != COLS - 1) {
        snprintf(footer + strlen(footer), COLS - strlen(footer), " ");
    }
    attrset(COLOR_PAIR(4));
    mvaddstr(LINES - 1, 0, footer);
}

void add_banned_cpu(uint64_t *banned_cpu, void *data)
{
    snprintf(data + strlen(data), 1024 - strlen(data), "%lu, ", *banned_cpu);
}

void display_banned_cpus()
{
    char banned_cpus[1024] = "Banned CPU numbers: \0";
    if(g_list_length(setup.banned_cpus) > 0) {
        for_each_banned_cpu(setup.banned_cpus, add_banned_cpu, banned_cpus);
        snprintf(banned_cpus + strlen(banned_cpus) - 2,
                 1024 - strlen(banned_cpus), "\n");
    } else {
        snprintf(banned_cpus + strlen(banned_cpus),
                 1024 - strlen(banned_cpus), "None\n");
    }
    attrset(COLOR_PAIR(1));
    addstr(banned_cpus);
}

char * check_control_in_input(int max_len, int column_offest, int line_offset)
{
    char *input_to = malloc(max_len * sizeof(char));
    int iteration = 0;
    while(iteration < max_len) {
        int new = getch();
		switch(new) {
        case '\r':
        case '\n':
            input_to[iteration] = '\0';
            return input_to;
        case 'q':
            close_window(0);
            break;
        case KEY_BACKSPACE:
            if(iteration > 0) {
                attrset(COLOR_PAIR(5));
                iteration--;
                mvaddch(line_offset, column_offest + iteration, ' ');
            }
            move(line_offset, column_offest + iteration);
            attrset(COLOR_PAIR(6));
            break;
        case 27:
            return NULL;
        default:
            input_to[iteration] = new;
            iteration++;
            break;
        }
    }
    return input_to;
}

int get_valid_sleep_input(int column_offest)
{
    uint64_t new_sleep = setup.sleep;
    while(1) {
        attrset(COLOR_PAIR(5));
        mvaddstr(0, column_offest, "                 ");
        attrset(COLOR_PAIR(6));
        refresh();
        move(0, column_offest);
        curs_set(1);
        char *input = check_control_in_input(20, column_offest, 0);
        if(input == NULL) {
            curs_set(0);
            attrset(COLOR_PAIR(1));
            mvaddstr(0, column_offest, "                 ");
            mvprintw(0, column_offest, "%lu", new_sleep);
            move(LINES, COLS);
            break;
        }
        attrset(COLOR_PAIR(1));
        mvaddstr(LINES - 2, 0, "                                         ");
        curs_set(0);
        refresh();
        char *error;
        new_sleep = strtol(input, &error, 10);
        if((*error == '\0') && (new_sleep >= 1)) {
            break;
        } else {
            new_sleep = setup.sleep;
            char message[128];
            snprintf(message, 128,
                     "Invalid input: %s                                       ",
                     input);
            attrset(COLOR_PAIR(4));
            mvaddstr(LINES - 2, 0, message);
            refresh();
        }
    }

    attrset(COLOR_PAIR(1));
    mvprintw(0, column_offest, "%lu                              ", new_sleep);

    return new_sleep;
}

void settings()
{
    clear();

    char info[128] = "Current sleep interval between rebalancing: \0";
    uint8_t sleep_input_offset = strlen(info);
    snprintf(info + strlen(info), 128 - strlen(info), "%lu\n", setup.sleep);
    attrset(COLOR_PAIR(1));
    addstr(info);
    // FIXME addstr info for all cpus if banned or not

    int user_input = 1;
    while(user_input) {
        snprintf(info, 128,
                 "Press <S> for changing sleep setup, <C> for CPU ban setup.");
        attrset(COLOR_PAIR(5));
        mvaddstr(LINES - 2, 0, info);
        show_footer();
        refresh();
        int c = getch();
        snprintf(info, 128, "Press ESC for discarding your input.\
                                                                  ");
        mvaddstr(LINES - 1, 0, info);
        switch(c) {
        case 's': {
            attrset(COLOR_PAIR(0));
            mvaddstr(LINES - 2, 0, "                      \
                                                        ");
            uint64_t new_sleep = get_valid_sleep_input(sleep_input_offset);
            if(new_sleep != setup.sleep) {
                setup.sleep = new_sleep;
                char settings_data[128];
                snprintf(settings_data, 128, "settings sleep %lu", new_sleep);
                send_settings(settings_data);
            }
            break;
        }
        case 'c':
            //setup ban, write to file, send

            break;
        /* We need to include window changing options as well because the
         * related char was eaten up by getch() already */
        case 'q':
            user_input = 0;
            close_window(0);
            break;
        case KEY_F(3):
            user_input = 0;
            display_tree();
            break;
        case KEY_F(5):
            user_input = 0;
            setup_irqs();
            break;
        default:
            break;
        }
    }
}

void setup_irqs()
{
    clear();

    char info[512];


    snprintf(info, 512, "Press <I> for setting up IRQ banning.");
    attrset(COLOR_PAIR(5));
    mvaddstr(LINES - 2, 0, info);

    show_footer();
    refresh();
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
}

void display_tree()
{
    clear();
    display_banned_cpus();
    for_each_node(tree, display_tree_node, NULL);
    show_footer();
    refresh();
}

void init()
{
    signal(SIGINT, close_window);
    initscr();
    keypad(stdscr, TRUE);
    curs_set(0);
    nonl();
    cbreak();
    echo();
    if(has_colors()) {
        start_color();
        init_pair(1, COLOR_RED, COLOR_BLACK);
        init_pair(2, COLOR_YELLOW, COLOR_BLACK);
        init_pair(3, COLOR_GREEN, COLOR_BLACK);
        init_pair(4, COLOR_WHITE, COLOR_BLUE);
        init_pair(5, COLOR_WHITE, COLOR_RED);
        init_pair(6, COLOR_RED, COLOR_WHITE);
        init_pair(7, COLOR_BLACK, COLOR_CYAN);
    }

    display_tree();
}
