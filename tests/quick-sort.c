#include <assert.h>
#include <stdlib.h>
#include "list.h"

#include "common.h"
static uint16_t values[1000];

#include <stdio.h>

/*
static void list_qsort(struct list_head *head)
{
    struct list_head list_less, list_greater;
    struct listitem *pivot;
    struct listitem *item = NULL, *is = NULL;

    if (list_empty(head) || list_is_singular(head))
        return;


    INIT_LIST_HEAD(&list_less);
    INIT_LIST_HEAD(&list_greater);

    pivot = list_first_entry(head, struct listitem, list);
    list_del(&pivot->list);

    list_for_each_entry_safe (item, is, head, list) {
        printf("pivot: %d list for each: %d\n", pivot->i, item->i);
        getchar();
        if (cmpint(&item->i, &pivot->i) < 0)
            list_move_tail(&item->list, &list_less);
        else
            list_move(&item->list, &list_greater);
    }

    list_qsort(&list_less);
    list_qsort(&list_greater);

    list_add(&pivot->list, head);
    list_splice(&list_less, head);
    list_splice_tail(&list_greater, head);
}

*/

#define TEST(name, LIST) do{\
            printf("%s: ", #name);\
            struct listitem *item_r = NULL, *is_r = NULL;\
            list_for_each_entry_safe(item_r, is_r, LIST, list)\
                printf("%d ", item_r->i);\
            printf("\n");\
            getchar();\
} while (0)

#define MAX 512
static void list_qsort_iterative(struct list_head *head) {
    if (list_empty(head) || list_is_singular(head))
        return;    
    struct list_head stack[MAX];
    for (int i = 0;i < MAX;i++) INIT_LIST_HEAD(&stack[i]);
    int top = -1;
    list_splice_init(head, &stack[++top]);

    struct list_head partition;
    INIT_LIST_HEAD(&partition);

    while (top >= 0) {
        INIT_LIST_HEAD(&partition);
        list_splice_init(&stack[top--], &partition);
        if (!list_empty(&partition) && !list_is_singular(&partition)) {
            struct list_head list_less, list_greater;
            struct listitem *pivot;
            struct listitem *item = NULL, *is = NULL;
            INIT_LIST_HEAD(&list_less);
            INIT_LIST_HEAD(&list_greater);
            pivot = list_first_entry(&partition, struct listitem, list);
            list_del(&pivot->list);
            INIT_LIST_HEAD(&pivot->list);

            list_for_each_entry_safe (item, is, &partition, list) {
                list_del(&item->list);
                if (cmpint(&item->i, &pivot->i) < 0) 
                    list_move(&item->list, &list_less);
                else 
                    list_move(&item->list, &list_greater);
            }
            
            list_move_tail(&pivot->list, &list_less);
            if(!list_empty(&list_greater))
                list_splice_tail(&list_greater, &stack[++top]);
            if(!list_empty(&list_less))
                 list_splice_tail(&list_less, &stack[++top]);
        } else {
            top++;
            list_splice_tail(&partition, &stack[top]);
            while (top >= 0 && list_is_singular(&stack[top])) {
                struct listitem *temp = list_first_entry(&stack[top], struct listitem, list);
                list_del(&temp->list);
                INIT_LIST_HEAD(&stack[top--]);
                list_add_tail(&temp->list, head);
            }
        }
    }
}

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
    
    //qsort(values, ARRAY_SIZE(values), sizeof(values[0]), cmpint);
    list_qsort_iterative(&testlist);
    //list_qsort(&testlist);
    
    //printf("\nafter:\n");
    //list_for_each_entry_safe(item, is, &testlist, list)
    //    printf("item: %d\n", item->i);


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

#include <sys/random.h>
#include <stddef.h>
#include <time.h>
uint16_t self_random(void) {
    /*
    char rand_array[64] = {0};
    getrandom(rand_array, 64, GRND_RANDOM);
    initstate( 0, rand_array, ARRAY_SIZE(rand_array));
    setstate(rand_array);
    printf("random %ld\n", random());
    return (uint16_t)random() % 1024;*/
    random_shuffle_array(values, (uint16_t) ARRAY_SIZE(values));
    time_t seed;
    return values[values[(time(&seed) & values[7] ) % ARRAY_SIZE(values)]];
}

#define time_diff(start, end) (end.tv_nsec - start.tv_nsec < 0 ? (1000000000 + end.tv_nsec - start.tv_nsec): (end.tv_nsec - start.tv_nsec) )
#define len 10000
#define times 1000
void analysis(void) {
    FILE *ptr = NULL;
    ptr = fopen("time_quick.txt", "w");
    if(!ptr) return;
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
            item->i = self_random();
        }
        clock_gettime(CLOCK_MONOTONIC, &time_start);
        list_qsort_iterative(&testlist);
        clock_gettime(CLOCK_MONOTONIC, &time_end);  
        during = time_diff(time_start, time_end);
        fprintf(ptr, "%d %f\n" , time_i, during);
    }
    fclose(ptr);

}

int main() {
    analysis();
    return 0;
}