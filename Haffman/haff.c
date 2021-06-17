#define ASCII_SIZE 256

#include <stdio.h>
#include <time.h>

#include "priority_queue.h"

struct node {
    int sym;
    size_t freq;
    struct haff_code *code;
    struct node *left;
    struct node *right;
};

struct haff_code {
    int len;
    int *val;
};

int *copy_array(const int *, int);
void set_codes(struct node *, struct haff_code *);
int *char_to_bites(char);

void encode(FILE *input, FILE *output) {
    struct node **node_array = calloc(ASCII_SIZE, sizeof(struct node *));
    int node_count = 0;
    int sym_count = 0;

    fseek(input, 0, SEEK_END);
    int sizefile = ftell(input);
    printf("Изначальный размер файла: %d\n", sizefile);
    rewind(input);


    clock_t start, end, algorithmStart, algorithmEnd;
    double time;

    int sym;
    struct node *curr_node;

    printf("Алгоритм кодирования Хаффмана...\n");
    algorithmStart = clock();
    start = clock();
    while ((sym = getc(input)) != EOF) {
        curr_node = node_array[sym];
        if (!curr_node) {
            curr_node = malloc(sizeof(struct node));
            curr_node->sym = sym;
            curr_node->freq = 0;
            curr_node->code = 0;
            curr_node->left = 0;
            curr_node->right = 0;
            node_array[sym] = curr_node;
            node_count++;
        }
        curr_node->freq += 1;
        sym_count++;
    }
    end = clock();
    time = end - start;
    printf("Чтение файла завершено за = %.2f\n", time);

    for (int i = 0; i < ASCII_SIZE; i++) {
        curr_node = node_array[i];
        if (curr_node)
            push(curr_node->freq, curr_node);
    }

    start = clock();
    while (get_queue_size() != 1) {
        struct node *first_min_node = pop();
        struct node *second_min_node = pop();
        struct node *new_node = malloc(sizeof(struct node));

        new_node->sym = -1;
        new_node->freq = first_min_node->freq + second_min_node->freq;
        new_node->left = first_min_node;
        new_node->right = second_min_node;

        push(new_node->freq, new_node);
    }
    end = clock();
    time = end - start;
    printf("Время, затраченное на построение дерева Хаффмана = %.2f\n", time);

    struct node *root = pop();

    struct haff_code *code = malloc(sizeof(struct haff_code));
    code->len = 0;
    code->val = calloc(ASCII_SIZE, sizeof(int));

    start = clock();
    set_codes(root, code);
    end = clock();
    time = end - start;
    printf("Формирование кодов завершено за %.2f\n", time);

    fwrite(&node_count, sizeof(int), 1, output);
    fwrite(&sym_count, sizeof(int), 1, output);
    for (int i = 0; i < ASCII_SIZE; i++) {
        curr_node = node_array[i];
        if (curr_node) {
            fwrite(&curr_node->sym, sizeof(char), 1, output);
            fwrite(&curr_node->freq, sizeof(int), 1, output);
        }
    }

    rewind(input);
    char write_code = 0;
    int code_size = 0;
    start = clock();
    while ((sym = getc(input)) != EOF) {
        curr_node = node_array[sym];
        int len = curr_node->code->len;
        int *curr_code = curr_node->code->val;
        for (int i = 0; i < len; i++) {
            if (curr_code[i])
                write_code = (write_code << 1) | 1;
            else 
                write_code = write_code << 1;
            
            code_size++;
            if (code_size == 8) {
                fwrite(&write_code, sizeof(char), 1, output);
                write_code = 0;
                code_size = 0;
            }
        }
    }
    
    if (code_size != 0) {
        write_code <<= (8 - code_size);
        fwrite(&write_code, sizeof(char), 1, output);
    }

    end = clock();
    time = end - start;
    printf("Запись данных в файл завершена. Прошло %.2f\n", time);

    free(node_array);
    algorithmEnd = clock();
    time = algorithmEnd - algorithmStart;
    printf("Алгоритм кодирования завершен за %.2f\n", time);

    fseek(output, 0, SEEK_END);
    sizefile = ftell(output);
    printf("Размер архива равен: %d\n", sizefile);
}

void decode(FILE *input, FILE *output) {
    int node_count;
    int sym_count;
    clock_t start, end, algorithmStart, algorithmEnd;
    double time;

    fseek(input, 0, SEEK_END);
    int sizefile = ftell(input);
    printf("Изначальный размер файла: %d\n", sizefile);
    rewind(input);

    printf("Алгоритм разархивации...\n");
    algorithmStart = clock();
    start = clock();

    fread(&node_count, sizeof(int), 1, input);
    fread(&sym_count, sizeof(int), 1, input);
    for (int i = 0; i < node_count; i++) {
        struct node *new_node = malloc(sizeof(struct node));
        fread(&new_node->sym, sizeof(char), 1, input);
        fread(&new_node->freq, sizeof(int), 1, input);
        push(new_node->freq, new_node);
    }
    end = clock();
    time = end - start;
    printf("Чтение данных для построения дерева Хаффмана завершено. %.2f\n", time);

    start = clock();
    while (get_queue_size() != 1) {
        struct node *first_min_node = pop();
        struct node *second_min_node = pop();
        struct node *new_node = malloc(sizeof(struct node));

        new_node->sym = -1;
        new_node->freq = first_min_node->freq + second_min_node->freq;
        new_node->left = first_min_node;
        new_node->right = second_min_node;

        push(new_node->freq, new_node);
    }

    struct node *root = pop();
    end = clock();
    time = end - start;
    printf("Построение дерева завершено. %.2f\n", time);


    char buff;
    struct node *curr_root = root;
    start = clock();
    while (sym_count) {
        fread(&buff, sizeof(char), 1, input);
        int *bytes = char_to_bites(buff);
        for (int i = 0; i < 8; i++) {
            if (bytes[i]) 
                curr_root = curr_root->right;
            else 
                curr_root = curr_root->left;
            
            if (curr_root->sym != -1) {
                fwrite(&curr_root->sym, sizeof(char), 1, output);
                curr_root = root;
                sym_count--;
                if (!sym_count) {
                    break;
                }
            }
        }
    }
    end = clock();
    algorithmEnd = clock();
    time = end - start;
    printf("Запись расшифрованных данных в файл. %f\n");
    time = algorithmEnd - algorithmStart;
    printf("Алгоритм разархивации завершен за %.2f\n", time);

    fseek(output, 0, SEEK_END);
    sizefile = ftell(output);
    printf("Размер полученного файла равен: %d\n", sizefile);
}

int *copy_array(const int *array, int size) {
    int *copy = calloc(ASCII_SIZE, sizeof(int));
    for (int i = 0; i < size; i++) 
        copy[i] = array[i];

    return copy;
}

void set_codes(struct node *curr_node, struct haff_code * prev_code) {
    if (curr_node->sym != -1) {
        curr_node->code = prev_code;
        return;
    }

    struct haff_code *left_code = malloc(sizeof(struct haff_code));
    left_code->len = prev_code->len;
    left_code->val = copy_array(prev_code->val, prev_code->len);
    *(left_code->val + left_code->len) = 0;
    left_code->len += 1;
    set_codes(curr_node->left, left_code);

    struct haff_code *right_code = malloc(sizeof(struct haff_code));
    right_code->len = prev_code->len;
    right_code->val = copy_array(prev_code->val, prev_code->len);
    *(right_code->val + right_code->len) = 1;
    right_code->len += 1;
    set_codes(curr_node->right, right_code);
}

int *char_to_bites(char c) {
    int *bites = calloc(8, sizeof(int));
    int bit;
    for (int i = 0; i < 8; i++) {
        if (c & (1 << i)) 
            bit = 1;
        else 
            bit = 0;

        bites[7 - i] = bit;
    }

    return bites;
}