//This program was written by Chan (z5397239)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define INVALID_PRIORITY -1

#define MAX_TASK_LENGTH 200
#define MAX_CATEGORY_LENGTH 40
#define MAX_STRING_LENGTH 1024

// You *should* #define each command
// Stage 1
#define COMMAND_ADD_TASK 'a'
#define COMMAND_PRINT_TASKS 'p'
#define COMMAND_UPDATE_TASK_PRIORITY 'i'
#define COMMAND_COUNT_TASKS 'n'

// Stage 2
#define COMMAND_COMPLETE_TASK 'c'
#define COMMAND_PRINT_COMPLETE_TASKS 'P'
#define COMMAND_EXPECTED_COMPLETE_TIME 'e'

// Stage 3
#define COMMAND_DELETE_TASK 'd'
#define COMMAND_FINISH_DAY 'f'
#define COMMAND_REPEATABLE_TASK 'r'

// Stage 4
#define COMMAND_MATCH_TASKS 'm'
#define COMMAND_DELETE_MATCH_INCOMPLETE_TASKS '^'
#define COMMAND_SORT_INCOMPLETE_TASKS 's'

enum priority {
    LOW, MEDIUM, HIGH
};

struct task {
    char task_name[MAX_TASK_LENGTH];
    char category[MAX_CATEGORY_LENGTH];
    enum priority priority;
    int repeat;

    struct task *next;
};

struct completed_task {
    struct task *task;
    int start_time;
    int finish_time;
    struct completed_task *next;
};

struct todo_list {
    struct task *tasks;
    struct completed_task *completed_tasks;
};

////////////////////////////////////////////////////////////////////////////////
///////////////////// YOUR FUNCTION PROTOTYPES GO HERE /////////////////////////
////////////////////////////////////////////////////////////////////////////////

void command_loop(struct todo_list *todo);
void free_todo_list(struct todo_list *todo);
void add_task(struct todo_list *todo, char task_name[MAX_TASK_LENGTH],
        char task_category[MAX_CATEGORY_LENGTH], enum priority prio, int repeat);
void print_tasks(struct todo_list *todo);
void update_task_priority(struct todo_list *todo,
        char task_name[MAX_TASK_LENGTH],
        char task_category[MAX_CATEGORY_LENGTH]);
void count_tasks(struct todo_list *todo);
struct task* find_task(struct todo_list *todo, char task_name[MAX_TASK_LENGTH],
        char task_category[MAX_CATEGORY_LENGTH]);
struct task* remove_task(struct todo_list *todo,
        char task_name[MAX_TASK_LENGTH],
        char task_category[MAX_CATEGORY_LENGTH]);
void add_complete_task(struct todo_list *todo, struct task *task_found,
        int start_time, int finish_time);
int get_completion_time(struct todo_list *todo,
        char task_category[MAX_CATEGORY_LENGTH]);
void sort(struct todo_list *todo);
void finish_day(struct todo_list *todo);
int match(char name[MAX_TASK_LENGTH], char pattern[MAX_TASK_LENGTH]);

////////////////////////////////////////////////////////////////////////////////
//////////////////////// PROVIDED HELPER PROTOTYPES ////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void parse_add_task_line(char buffer[MAX_STRING_LENGTH],
        char task_name[MAX_TASK_LENGTH],
        char task_category[MAX_CATEGORY_LENGTH], enum priority *prio);
void parse_task_category_line(char buffer[MAX_STRING_LENGTH],
        char task_name[MAX_TASK_LENGTH],
        char task_category[MAX_CATEGORY_LENGTH]);
void parse_complete_task_line(char buffer[MAX_STRING_LENGTH],
        char task_name[MAX_TASK_LENGTH],
        char task_category[MAX_CATEGORY_LENGTH], int *start_time,
        int *finish_time);

enum priority string_to_priority(char priority[MAX_STRING_LENGTH]);
void remove_newline(char input[MAX_STRING_LENGTH]);
void trim_whitespace(char input[MAX_STRING_LENGTH]);
void print_one_task(int task_num, struct task *task);
void print_completed_task(struct completed_task *completed_task);

int task_compare(struct task *t1, struct task *t2);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int main(void) {
    // Stage 1.1
    // You should initialize the `todo` variable below. You will need
    // to use the malloc() function to allocate memory for it!
    struct todo_list *todo;

    printf("Welcome to CS ToDo!\n");

    todo = malloc(sizeof(struct todo_list));
    todo->tasks = NULL;
    todo->completed_tasks = NULL;

    command_loop(todo);

    printf("All done!\n");
    return 0;
}

/**
 * The central loop that executes commands until the program is completed.
 *
 * Parameters:
 *     todo - The todo list to execute commands on.
 *
 * Returns:
 *     Nothing
 */
void command_loop(struct todo_list *todo) {
    printf("Enter Command: ");
    char command;
    while (scanf(" %c", &command) == 1) {
        // Create a string to scan the entire command input into.
        char buffer[MAX_STRING_LENGTH];
        fgets(buffer, MAX_STRING_LENGTH, stdin);

        if (command == COMMAND_ADD_TASK) {
            // Create variables for each part of the command being scanned in
            // (name of task, category of task and priority of task)
            char task_name[MAX_TASK_LENGTH];
            char task_category[MAX_CATEGORY_LENGTH];
            enum priority task_priority;

            parse_add_task_line(buffer, task_name, task_category,
                    &task_priority);
            add_task(todo, task_name, task_category, task_priority, 0);

        } else if (command == COMMAND_PRINT_TASKS) {
            print_tasks(todo);
        } else if (command == COMMAND_UPDATE_TASK_PRIORITY) {
            char task_name[MAX_TASK_LENGTH];
            char task_category[MAX_CATEGORY_LENGTH];

            parse_task_category_line(buffer, task_name, task_category);
            update_task_priority(todo, task_name, task_category);
        } else if (command == COMMAND_COUNT_TASKS) {
            int items = 0;
            struct task *node = todo->tasks;
            while(node){
                items++;
                node = node->next;
            }
            printf("There are %d items on your list!\n", items);
        } else if (command == COMMAND_COMPLETE_TASK) {
            // Create strings for `task`/`category` and ints for times, then populate
            // them using the contents of `buffer`.
            char task_name[MAX_TASK_LENGTH];
            char category[MAX_CATEGORY_LENGTH];
            int start_time;
            int finish_time;
            parse_complete_task_line(buffer, task_name, category, &start_time,
                    &finish_time);

            struct task *task_found = find_task(todo, task_name, category);
            if (task_found == NULL) {
                printf("Could not find task '%s' in category '%s'.\n",
                        task_name, category);
            } else {
                task_found = remove_task(todo, task_name, category);
                add_complete_task(todo, task_found, start_time, finish_time);
            }
        } else if (command == COMMAND_PRINT_COMPLETE_TASKS) {
            struct completed_task *node = todo->completed_tasks;

            printf("==== Completed Tasks ====\n");
            if (node == NULL) {
                printf("No tasks have been completed today!\n");
            }
            while (node) {
                print_completed_task(node);
                node = node->next;
            }
            printf("=========================\n");

        } else if (command == COMMAND_EXPECTED_COMPLETE_TIME) {
            struct task *node = todo->tasks;
            printf("Expected completion time for remaining tasks:\n\n");
            int id = 1;
            while (node) {
                int time = get_completion_time(todo, node->category);
                print_one_task(id, node);
                printf("Expected completion time: %d minutes\n", time);
                printf("\n");

                id++;
                node = node->next;
            }
        } else if (command == COMMAND_DELETE_TASK) {

            char task_name[MAX_TASK_LENGTH];
            char category[MAX_CATEGORY_LENGTH];
            parse_task_category_line(buffer, task_name, category);

            struct task *task_found = find_task(todo, task_name, category);
            if (task_found == NULL) {
                printf("Could not find task '%s' in category '%s'.\n",
                        task_name, category);
            } else {
                task_found = remove_task(todo, task_name, category);
                free(task_found);
            }
        } else if (command == COMMAND_FINISH_DAY) {
            finish_day(todo);
        } else if (command == COMMAND_REPEATABLE_TASK) {
            char task_name[MAX_TASK_LENGTH];
            char category[MAX_CATEGORY_LENGTH];
            parse_task_category_line(buffer, task_name, category);

            struct task *task_found = find_task(todo, task_name, category);
            if (task_found == NULL) {
                printf("Could not find task '%s' in category '%s'.\n",
                        task_name, category);
            } else {
                if (task_found->repeat) {
                    task_found->repeat = 0;
                } else {
                    task_found->repeat = 1;
                }
            }
        } else if (command == COMMAND_MATCH_TASKS) {
            trim_whitespace(buffer);

            struct task *node = todo->tasks;
            printf("Tasks matching pattern '%s':\n", buffer);
            int id = 0;
            while (node) {
                if (match(node->task_name, buffer)) {
                    print_one_task(id, node);
                    id++;
                }
                node = node->next;
            }

        } else if (command == COMMAND_DELETE_MATCH_INCOMPLETE_TASKS) {
            trim_whitespace(buffer);

            while (1) {
                int found = 0;
                struct task *node = todo->tasks;
                printf("Tasks matching pattern '%s':\n", buffer);
                while (node) {
                    if (match(node->task_name, buffer)) {
                        remove_task(todo, node->task_name, node->category);
                        found = 1;
                        break;
                    }
                    node = node->next;
                }

                if (!found) {
                    break;
                }
            }
        } else if (command == COMMAND_SORT_INCOMPLETE_TASKS) {
            sort(todo);
        }

        printf("Enter Command: ");
    }

    free_todo_list(todo);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////// YOUR HELPER FUNCTIONS ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////
struct task* find_task(struct todo_list *todo, char task_name[MAX_TASK_LENGTH],
        char task_category[MAX_CATEGORY_LENGTH]) {

    struct task *node = todo->tasks;
    while (node) {
        if (strcmp(task_name, node->task_name) == 0
                && strcmp(task_category, node->category) == 0) {
            return node;
        }
        node = node->next;
    }
    return NULL;
}

struct task* remove_task(struct todo_list *todo,
        char task_name[MAX_TASK_LENGTH],
        char task_category[MAX_CATEGORY_LENGTH]) {

    struct task *task_found = find_task(todo, task_name, task_category);
    if (!task_found) {
        return NULL;
    }

    if (task_found == todo->tasks) {
        todo->tasks = todo->tasks->next;
    } else {
        struct task *node = todo->tasks;
        while (node->next != task_found) {
            node = node->next;
        }
        node->next = task_found->next;
    }
    return task_found;
}

void add_complete_task(struct todo_list *todo, struct task *task_found,
        int start_time, int finish_time) {

    if (start_time == -1) {
        start_time = 0;
        struct completed_task *comp = todo->completed_tasks;
        while (comp) {
            if (comp->finish_time > start_time) {
                start_time = comp->finish_time;
            }
            comp = comp->next;
        }
    }

    struct completed_task *new_comp = malloc(sizeof(struct completed_task));
    new_comp->start_time = start_time;
    new_comp->finish_time = finish_time;
    new_comp->next = NULL;
    new_comp->task = task_found;
    task_found->next = NULL;

    new_comp->next = todo->completed_tasks;
    todo->completed_tasks = new_comp;

    /*
     if (todo->completed_tasks == NULL) {
     todo->completed_tasks = new_comp;
     } else {
     struct completed_task *tail = todo->completed_tasks;
     while (tail->next) {
     tail = tail->next;
     }
     tail->next = new_comp;
     }
     */
}

int get_completion_time(struct todo_list *todo,
        char task_category[MAX_CATEGORY_LENGTH]) {
    int total = 0;
    int count = 0;

    struct completed_task *comp = todo->completed_tasks;
    while (comp) {
        struct task *task_node = comp->task;
        while (task_node) {
            if (strcmp(task_category, task_node->category) == 0) {
                total += comp->finish_time - comp->start_time;
                count++;
            }
            task_node = task_node->next;
        }
        comp = comp->next;
    }

    if (count == 0) {
        return 100;
    } else {
        double avg = total * 1.0 / count;
        return (int) avg;
    }
}

void sort(struct todo_list *todo) {
    struct task *iter1 = todo->tasks;
    struct task *iter2 = todo->tasks;

    while (iter1) {
        while (iter2 && iter2->next) {
            if (task_compare(iter2, iter2->next) > 0) {
                struct task temp;
                strcpy(temp.task_name, iter2->task_name);
                strcpy(temp.category, iter2->category);
                temp.priority = iter2->priority;
                temp.repeat = iter2->repeat;

                strcpy(iter2->task_name, iter2->next->task_name);
                strcpy(iter2->category, iter2->next->category);
                iter2->priority = iter2->next->priority;
                iter2->repeat = iter2->next->repeat;

                strcpy(iter2->next->task_name, temp.task_name);
                strcpy(iter2->next->category, temp.category);
                iter2->next->priority = temp.priority;
                iter2->next->repeat = temp.repeat;
            }
            iter2 = iter2->next;
        }
        iter1 = iter1->next;
    }
}

void finish_day(struct todo_list *todo) {
    struct completed_task *comp = todo->completed_tasks;
    while (comp) {
        struct task *task_node = comp->task;
        while (task_node) {
            if (task_node->repeat) {
                add_task(todo, task_node->task_name, task_node->category,
                        task_node->priority, 1);
            }
            task_node = task_node->next;
        }
        comp = comp->next;
    }

    struct completed_task *node_completed_task = todo->completed_tasks;
    while (node_completed_task) {
        struct completed_task *temp_completed_task = node_completed_task;
        node_completed_task = node_completed_task->next;

        struct task *node_task = temp_completed_task->task;
        while (node_task) {
            struct task *temp_task = node_task;
            node_task = node_task->next;
            free(temp_task);
        }

        free(temp_completed_task);
    }
    todo->completed_tasks = NULL;
}

int match(char name[MAX_TASK_LENGTH], char pattern[MAX_TASK_LENGTH]) {
    char temp1[MAX_TASK_LENGTH];
    char temp2[MAX_TASK_LENGTH];

    int len_name = strlen(name);
    int len_pattern = strlen(pattern);

    int i = 0;
    int j = 0;

    while (i < len_name && j < len_pattern) {
        if (pattern[j] == '*') {
            strcpy(temp2, pattern + j + 1);

            for (int k = 0; k < len_name; k++) {
                strcpy(temp1, name + i + k);
                if (match(temp1, temp2)) {
                    return 1;
                }
            }
        } else if (pattern[j] == '?') {
            strcpy(temp2, pattern + j + 1);
            strcpy(temp1, name + i + 1);
            if (match(temp1, temp2)) {
                return 1;
            }
        } else if (pattern[j] == '[') {
            int j2 = j;
            while (pattern[j2] != ']') {
                j2++;
            }

            strcpy(temp1, name + i);
            for (int k = j + 1; k < j2; k++) {
                temp2[0] = pattern[k];
                strcpy(temp2 + 1, pattern + j2 + 1);
                if (match(temp1, temp2)) {
                    return 1;
                }
            }
            j = j2 + 1;
        } else {
            if (name[i] == pattern[j]) {
                i++;
                j++;
            } else {
                return 0;
            }
        }
    }

    return 1;
}
void free_todo_list(struct todo_list *todo) {
    struct task *node_task = todo->tasks;
    while (node_task) {
        struct task *temp_task = node_task;
        node_task = node_task->next;
        free(temp_task);
    }

    struct completed_task *node_completed_task = todo->completed_tasks;
    while (node_completed_task) {
        struct completed_task *temp_completed_task = node_completed_task;
        node_completed_task = node_completed_task->next;

        node_task = temp_completed_task->task;
        while (node_task) {
            struct task *temp_task = node_task;
            node_task = node_task->next;
            free(temp_task);
        }

        free(temp_completed_task);
    }

    free(todo);
}

void add_task(struct todo_list *todo, char task_name[MAX_TASK_LENGTH],
        char task_category[MAX_CATEGORY_LENGTH], enum priority prio, int repeat) {
    struct task *new_node = malloc(sizeof(struct task));
    strcpy(new_node->task_name, task_name);
    strcpy(new_node->category, task_category);
    new_node->priority = prio;
    new_node->repeat = repeat;
    new_node->next = NULL;

    if (todo->tasks == NULL) {
        todo->tasks = new_node;
    } else {
        struct task *pre = todo->tasks;
        while (pre->next) {
            pre = pre->next;
        }

        pre->next = new_node;
    }

    /*
     if (todo->tasks == NULL) {
     todo->tasks = t;
     } else if (t->priority < todo->tasks->priority) {
     t->next = todo->tasks;
     todo->tasks = t;
     } else {
     struct task *pre = todo->tasks;
     struct task *cur = todo->tasks->next;
     while (cur && t->priority >= cur->priority) {
     pre = cur;
     cur = cur->next;
     }

     pre->next = t;
     t->next = cur;
     }
     */
}

void print_tasks(struct todo_list *todo) {
    printf("==== Your ToDo List ====\n");
    int id = 1;
    struct task *cur = todo->tasks;
    while (cur) {
        print_one_task(id, cur);
        cur = cur->next;
        id++;
    }

    if (id == 1) {
        printf("All tasks completed, you smashed it!\n");
    }
    printf("====   That's it!   ====\n");
}

void update_task_priority(struct todo_list *todo,
        char task_name[MAX_TASK_LENGTH],
        char task_category[MAX_CATEGORY_LENGTH]) {

    struct task *cur = todo->tasks;
    while (cur) {
        if (strcmp(cur->task_name, task_name) == 0
                && strcmp(cur->category, task_category) == 0) {
            cur->priority++;
            cur->priority %= 3;
            break;
        }
        cur = cur->next;
    }

    if (!cur) {
        printf("Could not find task '%s' in category '%s'.\n",
                task_name, task_category);
    }
}

void count_tasks(struct todo_list *todo) {
    struct task *cur = todo->tasks;
    int num = 0;
    while (cur) {
        cur = cur->next;
        num++;
    }

    printf("There are %d items on your list!\n", num);
}

////////////////////////////////////////////////////////////////////////////////
/////////////////////// PROVIDED HELPER FUNCTIONS //////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/**
 * Helper Function
 * You DO NOT NEED TO UNDERSTAND THIS FUNCTION, and will not need to change it.
 *
 * Given a raw string in the format: [string1] [string2] [enum priority]
 * This function will extract the relevant values into the given variables.
 * The function will also remove any newline characters.
 *
 * For example, if given: "finish_assignment_2 assignment2 high"
 * The function will copy the string:
 *     "finish_assignment_2" into the `task_name` array
 * Then copy the string:
 *     "assignment2" into the `task_category` array
 * And finally, copy over the enum:
 *     "high" into the memory that `prio` is pointing at.
 *
 * Parameters:
 *     buffer        - A null terminated string in the following format
 *                     [string1] [string2] [enum priority]
 *     task_name     - A character array for the [string1] to be copied into
 *     task_category - A character array for the [string2] to be copied into
 *     prio          - A pointer to where [enum priority] should be stored
 *
 * Returns:
 *     None
 */
void parse_add_task_line(char buffer[MAX_STRING_LENGTH],
        char task_name[MAX_TASK_LENGTH],
        char task_category[MAX_CATEGORY_LENGTH], enum priority *prio) {
    remove_newline(buffer);

// Extract value 1 as string
    char *name_str = strtok(buffer, " ");
    if (name_str != NULL) {
        strcpy(task_name, name_str);
    }

// Extract value 2 as string
    char *category_str = strtok(NULL, " ");
    if (category_str != NULL) {
        strcpy(task_category, category_str);
    }

// Extract value 3 as string
    char *prio_str = strtok(NULL, " ");
    if (prio_str != NULL) {
        *prio = string_to_priority(prio_str);
    }

    if (name_str == NULL || category_str == NULL || prio_str == NULL
            || *prio == INVALID_PRIORITY) {
        // If any of these are null, there were not enough words.
        printf("Could not properly parse line: '%s'.\n", buffer);
    }
}

/*
 * Helper Function
 * You DO NOT NEED TO UNDERSTAND THIS FUNCTION, and will not need to change it.
 *
 * See `parse_add_task_line` for explanation - This function is very similar,
 * with only the exclusion of an `enum priority`.
 */
void parse_task_category_line(char buffer[MAX_STRING_LENGTH],
        char task_name[MAX_TASK_LENGTH],
        char task_category[MAX_CATEGORY_LENGTH]) {
    remove_newline(buffer);

// Extract value 1 as string
    char *name_str = strtok(buffer, " ");
    if (name_str != NULL) {
        strcpy(task_name, name_str);
    }

// Extract value 2 as string
    char *category_str = strtok(NULL, " ");
    if (category_str != NULL) {
        strcpy(task_category, category_str);
    }

    if (name_str == NULL || category_str == NULL) {
        // If any of these are null, there were not enough words.
        printf("Could not properly parse line: '%s'.\n", buffer);
    }
}

/*
 * Helper Function
 * You DO NOT NEED TO UNDERSTAND THIS FUNCTION, and will not need to change it.
 *
 * See `parse_add_task_line` for explanation - This function is very similar,
 * with only the exclusion of an `enum priority` and addition of start/end times
 */
void parse_complete_task_line(char buffer[MAX_STRING_LENGTH],
        char task_name[MAX_TASK_LENGTH],
        char task_category[MAX_CATEGORY_LENGTH], int *start_time,
        int *finish_time) {
    remove_newline(buffer);

// Extract value 1 as string
    char *name_str = strtok(buffer, " ");
    if (name_str != NULL) {
        strcpy(task_name, name_str);
    }

// Extract value 2 as string
    char *category_str = strtok(NULL, " ");
    if (category_str != NULL) {
        strcpy(task_category, category_str);
    }

// Extract value 2 as string
    char *start_str = strtok(NULL, " ");
    if (start_str != NULL) {
        *start_time = atoi(start_str);
    }

// Extract value 2 as string
    char *finish_str = strtok(NULL, " ");
    if (finish_str != NULL) {
        *finish_time = atoi(finish_str);
    }

    if (name_str == NULL || category_str == NULL || start_str == NULL
            || finish_str == NULL) {
        // If any of these are null, there were not enough words.
        printf("Could not properly parse line: '%s'.\n", buffer);
    }
}

/**
 * Helper Function
 * You should not need to change this function.
 *
 * Given a raw string, will return the corresponding `enum priority`,
 * or INVALID_PRIORITY if the string doesn't correspond with the enums.
 *
 * Parameters:
 *     priority - string representing the corresponding `enum priority` value
 * Returns:
 *     enum priority
 */
enum priority string_to_priority(char priority[MAX_STRING_LENGTH]) {
    if (strcmp(priority, "low") == 0) {
        return LOW;
    } else if (strcmp(priority, "medium") == 0) {
        return MEDIUM;
    } else if (strcmp(priority, "high") == 0) {
        return HIGH;
    }

    return INVALID_PRIORITY;
}

/**
 * Helper Function
 * You should not need to change this function.
 *
 * Given an priority and a character array, fills the array with the
 * corresponding string version of the priority.
 *
 * Parameters:
 *     prio - the `enum priority` to convert from
 *     out  - the array to populate with the string version of `prio`.
 * Returns:
 *     Nothing
 */
void priority_to_string(enum priority prio, char out[MAX_STRING_LENGTH]) {
    if (prio == LOW) {
        strcpy(out, "LOW");
    } else if (prio == MEDIUM) {
        strcpy(out, "MEDIUM");
    } else if (prio == HIGH) {
        strcpy(out, "HIGH");
    } else {
        strcpy(out, "Provided priority was invalid");
    }
}

/*
 * Helper Function
 * You should not need to change this function.
 *
 * Given a raw string will remove and first newline it sees.
 * The newline character wil be replaced with a null terminator ('\0')
 *
 * Parameters:
 *     input - The string to remove the newline from
 *
 * Returns:
 *     Nothing
 */
void remove_newline(char input[MAX_STRING_LENGTH]) {
// Find the newline or end of string
    int index = 0;
    while (input[index] != '\n' && input[index] != '\0') {
        index++;
    }
// Goto the last position in the array and replace with '\0'
// Note: will have no effect if already at null terminator
    input[index] = '\0';
}

/*
 * Helper Function
 * You likely do not need to change this function.
 *
 * Given a raw string, will remove any whitespace that appears at the start or
 * end of said string.
 *
 * Parameters:
 *     input - The string to trim whitespace from
 *
 * Returns:
 *     Nothing
 */
void trim_whitespace(char input[MAX_STRING_LENGTH]) {
    remove_newline(input);

    int lower;
    for (lower = 0; input[lower] == ' '; ++lower)
        ;

    int upper;
    for (upper = strlen(input) - 1; input[upper] == ' '; --upper)
        ;

    for (int base = lower; base <= upper; ++base) {
        input[base - lower] = input[base];
    }

    input[upper - lower + 1] = '\0';
}

/**
 * Helper Function
 * You SHOULD NOT change this function.
 *
 * Given a task, prints it out in the format specified in the assignment.
 *
 * Parameters:
 *     task_num - The position of the task within a todo list
 *     task     - The task in question to print
 *
 * Returns:
 *     Nothing
 */
void print_one_task(int task_num, struct task *task) {
    char prio_str[MAX_STRING_LENGTH];
    priority_to_string(task->priority, prio_str);

    printf("  %02d. %-30.30s [ %s ] %s\n", task_num, task->task_name,
            task->category, prio_str);

    int i = 30;
    while (i < strlen(task->task_name)) {
        printf("      %.30s\n", task->task_name + i);
        i += 30;
    }
}

/**
 * Helper Function
 * You SHOULD NOT change this function.
 *
 * Given a completed task, prints it out in the format specified in the
 * assignment.
 *
 * Parameters:
 *     completed_task - The task in question to print
 *
 * Returns:
 *     Nothing
 */
void print_completed_task(struct completed_task *completed_task) {
    int start_hour = completed_task->start_time / 60;
    int start_minute = completed_task->start_time % 60;
    int finish_hour = completed_task->finish_time / 60;
    int finish_minute = completed_task->finish_time % 60;

    printf("  %02d:%02d-%02d:%02d | %-30.30s [ %s ]\n", start_hour,
            start_minute, finish_hour, finish_minute,
            completed_task->task->task_name, completed_task->task->category);

    int i = 30;
    while (i < strlen(completed_task->task->task_name)) {
        printf("      %.30s\n", (completed_task->task->task_name + i));
        i += 30;
    }
}

/**
 * Compares two tasks by precedence of category -> priority -> name and returns
 * an integer referring to their relative ordering
 * 
 * Parameters:
 *     t1 - The first task to compare
 *     t2 - The second task to compare
 *
 * Returns:
 *     a negative integer if t1 belongs before t2
 *     a positive integer if t1 belongs after t2
 *     0 if the tasks are identical (This should never happen in your program)
 */
int task_compare(struct task *t1, struct task *t2) {
    int category_diff = strcmp(t1->category, t2->category);
    if (category_diff != 0) {
        return category_diff;
    }

    int priority_diff = t2->priority - t1->priority;
    if (priority_diff != 0) {
        return priority_diff;
    }

    return strcmp(t1->task_name, t2->task_name);
}
