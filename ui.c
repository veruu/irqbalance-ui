
#include <string.h>
#include "ui.h"


GList *all_cpus = NULL;
GList *all_irqs = NULL;

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

char * check_control_in_sleep_input(int max_len, int column_offest, int line_offset)
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
        char *input = check_control_in_sleep_input(20, column_offest, 0);
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

void get_banned_cpu(uint64_t *cpu, void *data __attribute__((unused)))
{
    cpu_ban_t *new = malloc(sizeof(cpu_ban_t));
    new->number = *cpu;
    new->is_banned = 1;
    all_cpus = g_list_append(all_cpus, new);
}

void print_cpu_line(cpu_ban_t *cpu, void *data)
{
    int *line_offset = data;
    mvprintw(*line_offset, 0, "CPU %lu", cpu->number);
    mvprintw(*line_offset, 20, "%s", cpu->is_banned ? "YES" : "NO ");
    (*line_offset)++;
}

void print_all_cpus()
{
    if(all_cpus == NULL) {
        for_each_node(tree, get_cpu, NULL);
        for_each_banned_cpu(setup.banned_cpus, get_banned_cpu, NULL);
        all_cpus = g_list_sort(all_cpus, sort_all_cpus);
    }
    int *line = malloc(sizeof(int));
    *line = 3; /* 0 for sleep interval setup, 1 free, 2 for header */
    attrset(COLOR_PAIR(2));
    mvprintw(2, 0, "NUMBER              IS BANNED");
    attrset(COLOR_PAIR(3));
    for_each_cpu(all_cpus, print_cpu_line, line);
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

int toggle_cpu(GList *cpu_list, int cpu_number)
{
    GList *entry = g_list_first(cpu_list);
    cpu_ban_t *entry_data = (cpu_ban_t *)(entry->data);
    while(entry_data->number != cpu_number) {
        entry = g_list_next(entry);
        entry_data = (cpu_ban_t *)(entry->data);
    }
    if(((cpu_ban_t *)(entry->data))->is_banned) {
        ((cpu_ban_t *)(entry->data))->is_banned = 0;
    } else {
        ((cpu_ban_t *)(entry->data))->is_banned = 1;
    }
    return ((cpu_ban_t *)(entry->data))->is_banned;
}

void get_new_cpu_ban_values(cpu_ban_t *cpu, void *data)
{
    char *mask_data = (char *)data;
    if(cpu->is_banned) {
        snprintf(mask_data + strlen(mask_data), 1024 - strlen(mask_data),
                 "%lu,", cpu->number);
    }
}

void get_cpu(cpu_node_t *node, void *data __attribute__((unused)))
{
    if(node->type == OBJ_TYPE_CPU) {
        cpu_ban_t *new = malloc(sizeof(cpu_ban_t));
        new->number = node->number;
        new->is_banned = 0;
        all_cpus = g_list_append(all_cpus, new);
    }
    if(g_list_length(node->children) > 0) {
        for_each_node(node->children, get_cpu, NULL);
    }
}

void handle_cpu_banning()
{
    GList *tmp = g_list_copy_deep(all_cpus, copy_cpu_ban, NULL);
    char info[128];
    attrset(COLOR_PAIR(5));
    snprintf(info, 128, "%s\n%s",
             "Move up and down the list, toggle ban with Enter.",
             "Press ESC for discarding and <S> for saving the values.");
    mvaddstr(LINES - 3, 0, info);
    move(3, 20);
    curs_set(1);
    refresh();
    int position = 3;
    char processing = 1;
    while(processing) {
        int direction = getch();
        switch(direction) {
        case KEY_UP:
            if(position > 3) {
                position--;
                move(position, 20);
            }
            break;
        case KEY_DOWN:
            if(position <= g_list_length(all_cpus) + 1) {
                position++;
                move(position, 20);
            }
            break;
        case '\n':
        case '\r': {
            attrset(COLOR_PAIR(3));
            int banned = toggle_cpu(tmp, position - 3);
            if(banned) {
                mvprintw(position, 20, "YES");
            } else {
                mvprintw(position, 20, "NO ");
            }
            move(position, 20);
            refresh();
            break;
        }
        case 27:
            processing = 0;
            curs_set(0);
            /* Forget the changes */
            tmp = g_list_copy_deep(all_cpus, copy_cpu_ban, NULL);
            print_all_cpus();
            attrset(COLOR_PAIR(0));
            mvprintw(LINES - 3, 0, "                      \
                                                        ");
            attrset(COLOR_PAIR(5));
            mvprintw(LINES - 2, 0,
                "Press <S> for changing sleep setup, <C> for CPU ban setup.  ");
            move(LINES - 1, COLS - 1);
            refresh();
            break;
        case 's':
            processing = 0;
            all_cpus = tmp;
            curs_set(0);
            print_all_cpus();
            attrset(COLOR_PAIR(0));
            mvprintw(LINES - 3, 0, "                      \
                                                        ");
            attrset(COLOR_PAIR(5));
            mvprintw(LINES - 2, 0,
                "Press <S> for changing sleep setup, <C> for CPU ban setup.  ");
            attrset(COLOR_PAIR(3));
            move(LINES - 1, COLS - 1);
            refresh();
            char settings_string[1024] = "settings cpus \0";
            for_each_cpu(all_cpus, get_new_cpu_ban_values, settings_string);
            if(!strcmp("settings cpus \0", settings_string)) {
                strncpy(settings_string + strlen(settings_string),
                        "NULL", 1024 - strlen(settings_string));
            }
            send_settings(settings_string);
            break;
        case 'q':
            processing = 0;
            close_window(0);
            break;
        case KEY_F(3):
            processing = 0;
            display_tree();
            break;
        case KEY_F(5):
            processing = 0;
            setup_irqs();
            break;
        default:
            break;
        }
    }
}

void print_irq_line(irq_t *irq, void *data)
{
    int *line_offset = data;
    mvprintw(*line_offset, 0, "IRQ %lu", irq->vector);
    mvprintw(*line_offset, 20, "%s", irq->is_banned ? "YES" : "NO ");
    (*line_offset)++;

}

void print_all_irqs()
{
    int *line = malloc(sizeof(int));
    *line = 1;
    attrset(COLOR_PAIR(2));
    mvprintw(0, 0, "NUMBER              IS BANNED");
    attrset(COLOR_PAIR(3));
    for_each_irq(all_irqs, print_irq_line, line);
}

int toggle_irq(GList *irq_list, int position)
{
    GList *entry = g_list_first(irq_list);
    int irq_node = 0;
    while(irq_node != position) {
        entry = g_list_next(entry);
        irq_node++;
    }
    if(((irq_t *)(entry->data))->is_banned) {
        ((irq_t *)(entry->data))->is_banned = 0;
    } else {
        ((irq_t *)(entry->data))->is_banned = 1;
    }
    return ((irq_t *)(entry->data))->is_banned;
}

void get_new_irq_ban_values(irq_t *irq, void *data)
{
    char *ban_list = (char *)data;
    if(irq->is_banned) {
        snprintf(ban_list + strlen(ban_list), 1024 - strlen(ban_list),
                 " %lu", irq->vector);
    }
}

void copy_irqs_from_nodes(cpu_node_t *node, void *data __attribute__((unused)))
{
    if(g_list_length(node->irqs) > 0) {
        GList *new = g_list_copy_deep(node->irqs, copy_irq, NULL);
        all_irqs = g_list_concat(all_irqs, new);
    }
    if(g_list_length(node->children) > 0) {
        for_each_node(node->children, copy_irqs_from_nodes, all_irqs);
    }                                                                           
}

void get_all_irqs()
{
    all_irqs = g_list_copy_deep(setup.banned_irqs, copy_irq, NULL);
    for_each_node(tree, copy_irqs_from_nodes, NULL);
}

void handle_irq_banning()
{
    GList *tmp = g_list_copy_deep(all_irqs, copy_irq, NULL);
    char info[128];
    attrset(COLOR_PAIR(5));
    snprintf(info, 128, "%s\n%s",
             "Move up and down the list, toggle ban with Enter.",
             "Press ESC for discarding and <S> for saving the values.");
    mvaddstr(LINES - 3, 0, info);
    move(1, 20);
    curs_set(1);
    refresh();
    int position = 1;
    char processing = 1;
    while(processing) {
        int direction = getch();
        switch(direction) {
        case KEY_UP:
            if(position > 1) {
                position--;
                move(position, 20);
            }
            break;
        case KEY_DOWN:
            if(position < g_list_length(all_irqs)) {
                position++;
                move(position, 20);
            }
            break;
        case '\n':
        case '\r': {
            attrset(COLOR_PAIR(3));
            int banned = toggle_irq(tmp, position - 1);
            if(banned) {
                mvprintw(position, 20, "YES");
            } else {
                mvprintw(position, 20, "NO ");
            }
            move(position, 20);
            refresh();
            break;
        }
        case 27:
            processing = 0;
            curs_set(0);
            /* Forget the changes */
            tmp = g_list_copy_deep(all_irqs, copy_irq, NULL);
            print_all_irqs();
            attrset(COLOR_PAIR(0));
            mvprintw(LINES - 3, 0, "                      \
                                                        ");
            attrset(COLOR_PAIR(5));
            mvprintw(LINES - 2, 0, "Press <I> for setting up IRQ banning.\
                            ");
            move(LINES - 1, COLS - 1);
            refresh();
            break;
        case 's':
            processing = 0;
            all_irqs = tmp;
            curs_set(0);
            print_all_irqs();
            attrset(COLOR_PAIR(0));
            mvprintw(LINES - 3, 0, "                      \
                                                        ");
            attrset(COLOR_PAIR(5));
            mvprintw(LINES - 2, 0, "Press <I> for setting up IRQ banning.\
                            ");
            attrset(COLOR_PAIR(3));
            move(LINES - 1, COLS - 1);
            refresh();
            char settings_string[1024] = "settings ban irqs\0";
            for_each_irq(all_irqs, get_new_irq_ban_values, settings_string);
            if(!strcmp("settings ban irqs\0", settings_string)) {
                strncpy(settings_string + strlen(settings_string),
                        " NONE", 1024 - strlen(settings_string));
            }
            send_settings(settings_string);
            break;
        case 'q':
            processing = 0;
            close_window(0);
            break;
        case KEY_F(3):
            processing = 0;
            display_tree();
            break;
        case KEY_F(4):
            processing = 0;
            settings();
            break;
        default:
            break;
        }
    }
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

void close_window(int sig)
{
    endwin();
    exit(EXIT_SUCCESS);
}

void settings()
{
    clear();
    char *setup_data = get_data(SETUP);
    parse_setup(setup_data);

    char info[128] = "Current sleep interval between rebalancing: \0";
    uint8_t sleep_input_offset = strlen(info);
    snprintf(info + strlen(info), 128 - strlen(info), "%lu\n", setup.sleep);
    attrset(COLOR_PAIR(1));
    addstr(info);
    print_all_cpus();

    int user_input = 1;
    while(user_input) {
        snprintf(info, 128,
                "Press <S> for changing sleep setup, <C> for CPU ban setup.  ");
        attrset(COLOR_PAIR(5));
        mvaddstr(LINES - 2, 0, info);
        show_footer();
        refresh();
        int c = getch();
        switch(c) {
        case 's': {
            snprintf(info, 128, "Press ESC for discarding your input.");
            mvaddstr(LINES - 1, 0, info);
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
            handle_cpu_banning();
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

    get_all_irqs();
    all_irqs = g_list_sort(all_irqs, sort_all_irqs);
    print_all_irqs();
    char info[512];
    snprintf(info, 512, "Press <I> for setting up IRQ banning.");
    attrset(COLOR_PAIR(5));
    mvaddstr(LINES - 2, 0, info);
    show_footer();
    refresh();

    int user_input = 1;
    while(user_input) {
        int c = getch();
        switch(c) {
        case 'i':
            handle_irq_banning();
            break;
        case 'q':
            user_input = 0;
            close_window(0);
            break;
        case KEY_F(3):
            user_input = 0;
            display_tree();
            break;
        case KEY_F(4):
            user_input = 0;
            settings();
            break;
        default:
            break;
        }
    }
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
    char *setup_data = get_data(SETUP);
    parse_setup(setup_data);
    char *irqbalance_data = get_data(STATS);
    parse_into_tree(irqbalance_data);
    display_banned_cpus();
    for_each_node(tree, display_tree_node, NULL);
    show_footer();
    refresh();
    int processing = 1;
    while(processing) {
        int choice = getch();
        mvprintw(0,0, "%d",choice);
        refresh();
	    switch(choice) {
            case 'q':
                processing = 0;
                close_window(0);
                break;
            case KEY_F(4):
                processing = 0;
                settings();
                break;
            case KEY_F(5):
                processing = 0;
                setup_irqs();
                break;
            default:
                break;
        }
    }
}
