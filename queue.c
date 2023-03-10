#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *new = malloc(sizeof(struct list_head));
    if (!new)
        return NULL;

    INIT_LIST_HEAD(new);
    return new;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;

    struct list_head *node = l->next;
    while (node != l) {
        struct list_head *del = node;
        node = node->next;
        list_del(del);
        q_release_element(list_entry(del, element_t, list));
    }
    free(l);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head || !s)
        return false;

    element_t *new = malloc(sizeof(element_t));
    if (!new)
        return false;

    int str_size = strlen(s);
    new->value = malloc(str_size + 1);

    if (!new->value) {
        free(new);
        return false;
    }
    strncpy(new->value, s, str_size);
    *(new->value + str_size) = '\0';
    list_add(&new->list, head);

    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head || !s)
        return false;
    return q_insert_head(head->prev, s);
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *node = list_first_entry(head, element_t, list);
    list_del(&node->list);

    if (bufsize) {
        strncpy(sp, node->value, bufsize - 1);
        *(sp + bufsize - 1) = '\0';
    }
    return node;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    return q_remove_head(head->prev->prev, sp, bufsize);
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;

    struct list_head *front = head->next, *back = head->prev;

    while (front != back && front->next != back) {
        front = front->next;
        back = back->prev;
    }

    list_del(back);
    q_release_element(list_entry(back, element_t, list));
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head || list_empty(head) || list_is_singular(head))
        return true;
    element_t *cur_node, *next_node;
    bool flag = false;
    list_for_each_entry_safe (cur_node, next_node, head, list) {
        bool equal = &next_node->list != head &&
                     !strcmp(cur_node->value, next_node->value);
        if (flag || equal) {
            list_del(&cur_node->list);
            q_release_element(cur_node);
            flag = equal;
        }
    }
    if (flag) {
        list_del(&cur_node->list);
        q_release_element(cur_node);
    }

    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head)
        return;
    struct list_head *first = head->next, *second = first->next;
    while (first != head && second != head) {
        list_del_init(first);
        list_add(first, second);
        first = first->next;
        second = first->next;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *cur = head->next, *next = cur->next;
    while (cur != head) {
        list_del_init(cur);
        list_add(cur, head);
        cur = next;
        next = cur->next;
    }
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (q_size(head) <= 1)
        return;
    struct list_head *curr, *next, *tmp_head_from = head, tmp_head_to;
    int i = 1;
    INIT_LIST_HEAD(&tmp_head_to);
    list_for_each_safe (curr, next, head) {
        if (i == k) {
            list_cut_position(&tmp_head_to, tmp_head_from, curr);
            q_reverse(&tmp_head_to);
            list_splice_init(&tmp_head_to, tmp_head_from);
            tmp_head_from = next->prev;
            i = 0;
        }
        i++;
    }
}

void restructure_list(struct list_head *head)
{
    struct list_head *curr = head, *next = curr->next;
    while (next) {
        next->prev = curr;
        curr = next;
        next = next->next;
    }
    curr->next = head;
    head->prev = curr;
}

/*Merge two sorted list in asceding order*/
static struct list_head *mergelist(struct list_head *list1,
                                   struct list_head *list2)
{
    struct list_head *res = NULL;
    struct list_head **indirect = &res;
    for (struct list_head **node = NULL; list1 && list2;
         *node = (*node)->next) {
        element_t *list1_entry = list_entry(list1, element_t, list);
        element_t *list2_entry = list_entry(list2, element_t, list);
        node = strcmp(list1_entry->value, list2_entry->value) < 0 ? &list1
                                                                  : &list2;
        *indirect = *node;
        indirect = &(*indirect)->next;
    }
    *indirect = (struct list_head *) ((size_t) list1 | (size_t) list2);
    return res;
}

static struct list_head *mergesort(struct list_head *head)
{
    if (!head || !head->next)
        return head;

    struct list_head *fast = head, *slow = head;
    while (fast && fast->next) {
        fast = fast->next->next;
        slow = slow->next;
    }

    struct list_head *mid = slow;
    slow->prev->next = NULL;

    struct list_head *left = mergesort(head);
    struct list_head *right = mergesort(mid);
    return mergelist(left, right);
}
/* Sort elements of queue in ascending order */
void q_sort(struct list_head *head)
{
    if (q_size(head) <= 1)
        return;
    // change circular doubly-linked list to singular linked list
    head->prev->next = NULL;
    head->next = mergesort(head->next);
    /* restructure the doubly-linked list */
    restructure_list(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;
    element_t *entry, *next;
    list_for_each_entry_safe (entry, next, head, list) {
        if (&next->list != head && strcmp(entry->value, next->value) < 0) {
            list_del(&entry->list);
            q_release_element(entry);
        }
    }
    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending order */
int q_merge(struct list_head *head)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}
