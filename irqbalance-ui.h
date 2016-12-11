
#ifndef IRQBALANCE_UI_H
#define IRQBALANCE_UI_H

#include <stdio.h>
#include <stdint.h>
#include <glib.h>
#include <glib-unix.h>

#define SOCKET_PATH "/var/run/irqbalance.sock"

#define STATS "stats"
#define SET_SLEEP "settings sleep "
#define BAN_IRQS "settings ban irqs "
#define SETUP "setup"

/* Typedefs */

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
    char is_banned;
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

typedef struct cpu_ban {
    uint64_t number;
    char is_banned;
} cpu_ban_t;

typedef struct setup {
    uint64_t sleep;
    GList *banned_irqs;
    GList *banned_cpus;
} setup_t;

/* Function prototypes */

int init_connection();
int send_settings(char *data);
char * get_data(char *string);
void parse_setup(char *setup_data);
void parse_into_tree(char *data);
int main();


#endif /* IRQBALANCE_UI_H */
