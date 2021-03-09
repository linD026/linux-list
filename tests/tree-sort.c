#include <assert.h>
#include <stdlib.h>
#include "list.h"
#include "rb_tree_augmented.h"

#include "dtusage.h"
#include "common.h"
//static uint16_t values[1000];

#define u32 uint32_t

//l2t is list to tree. 
struct l2t_node {
    struct list_head *list_node;
    struct rb_node rb;
};

static void insert(struct l2t_node *node, struct rb_root_cached *root)
{
	struct rb_node **new = &root->rb_root.rb_node, *parent = NULL;
    struct listitem *key = list_entry(node->list_node, struct listitem, list);

	while (*new) {
		parent = *new;
		if (key->i < list_entry(rb_entry(parent, struct l2t_node, rb)->list_node, struct listitem, list)->i)
			new = &parent->rb_left;
		else
			new = &parent->rb_right;
	}

	rb_link_node(&node->rb, parent, new);
	rb_insert_color(&node->rb, &root->rb_root);
}

static void tree_sort(struct list_head *head, int nnodes) {
    struct l2t_node nodes[nnodes];
    struct rb_root_cached root = RB_ROOT_CACHED;
    struct listitem *item = NULL;
    struct listitem *is = NULL;
    
    // the sum of node of list need equal to nnodes
    int node_n = 0;
    list_for_each_entry_safe(item, is, head, list) {
        list_del(&item->list);
        INIT_LIST_HEAD(&item->list);
        nodes[node_n++].list_node = &item->list;
    }

    for (int i = 0; i < nnodes;i++) {
        insert(&nodes[i], &root);
    }

    INIT_LIST_HEAD(head);    
	struct rb_node *node;
    //inorder Traverse
    for (node = rb_first(&root.rb_root); node; node = rb_next(node)){
        struct l2t_node *temp = container_of(node, struct l2t_node, rb);
        struct listitem *temp_item = list_entry(temp->list_node, struct listitem, list);
        list_add_tail( &temp_item->list, head);
    }
}


#include <stdio.h>
#define TEST(name, LIST) do{\
            printf("%s: ", #name);\
            struct listitem *item_r = NULL, *is_r = NULL;\
            list_for_each_entry_safe(item_r, is_r, LIST, list)\
                printf("%d ", item_r->i);\
            printf("\n");\
            getchar();\
} while (0)


/*
int main(void)
{

    struct list_head testlist;
    struct listitem *item, *is = NULL;
    size_t i;

    random_shuffle_array(values, (uint16_t) ARRAY_SIZE(values));

    INIT_LIST_HEAD(&testlist);

    assert(list_empty(&testlist));

    for (i = 0; i < ARRAY_SIZE(values); i++) {
        item = (struct listitem *) malloc(sizeof(*item));
        assert(item);
        item->i = values[i];
        list_add_tail(&item->list, &testlist);
    }

    assert(!list_empty(&testlist));
    
    qsort(values, ARRAY_SIZE(values), sizeof(values[0]), cmpint);
    tree_sort(&testlist, ARRAY_SIZE(values));
    
    //TEST(testlist, &testlist);
    //printf("values  ");
    //for(int i = 0;i < ARRAY_SIZE(values);i++){
    //    printf("%d ", values[i]);
    //}
    //printf("\n");

    i = 0;
    list_for_each_entry_safe (item, is, &testlist, list) {
        assert(item->i == values[i]);
        list_del(&item->list);
        free(item);
        i++;
    }

    assert(i == ARRAY_SIZE(values));
    assert(list_empty(&testlist));
    return 0;
}
*/


#include <stddef.h>
#include <time.h>
/*
uint16_t self_random_arr(void) {
    random_shuffle_array(values, (uint16_t) ARRAY_SIZE(values));
    time_t seed;
    return values[values[(time(&seed) & values[7] ) % ARRAY_SIZE(values)]];
}*/

// Lagged Fibonacci generators
// Sn ≡ Sn−j ⋆ Sn−k (modm), 0 < j < k
static inline int self_random(int seed_f, int seed_s) {
  int sn_1 = 0;
  int sn_2 = 1;
  int sn = random() % 1024;
  int i = seed_f;
  while (i > 0) {
    sn = (sn_2 & sn_1) % (seed_s + 1);
    sn_1 = sn_1 + 3;
    sn_2 = sn_2 + 7;
    i--;
  }
  return sn;
}

uint16_t set_rand(void){
  time_t current_time;
  srandom(time(&current_time));
  return self_random(random() % 1024, random() % 1024);
}

#define time_diff(start, end) (end.tv_nsec - start.tv_nsec < 0 ? (1000000000 + end.tv_nsec - start.tv_nsec): (end.tv_nsec - start.tv_nsec) )
#define len 10000
#define times 100
void analysis(void) {
    FILE *ptr = NULL;
    ptr = fopen("time_tree.txt", "w");
    if(!ptr) return;//{post_signal; pthread_exit(NULL);}
    printf("len:%d  time:%d\n", len, times);

    struct list_head testlist;
    INIT_LIST_HEAD(&testlist);
    struct listitem *item, *is = NULL;
    size_t i;
    for (i = 0; i < len; i++) {
        item = (struct listitem *) malloc(sizeof(*item));
        list_add_tail(&item->list, &testlist);
    }
    
    struct timespec time_start;
    struct timespec time_end;
    double during;
    for (int time_i = 0;time_i < times;time_i++) {
        printf("%d\n", time_i);
        list_for_each_entry_safe(item, is, &testlist, list) {
            item->i = set_rand();
        }
        clock_gettime(CLOCK_MONOTONIC, &time_start);
        tree_sort(&testlist, len);
        clock_gettime(CLOCK_MONOTONIC, &time_end);  
        during = time_diff(time_start, time_end);
        fprintf(ptr, "%d %f\n" , time_i, during);
    }
    fclose(ptr);
    //post_signal;
    //pthread_exit(NULL);
}

int main(int argc, char **argv) {
  //init_signal;
  //go_work(analysis);
  //thread_record_dmemory();
  //end_work;
  analysis();
  return 0;
}
