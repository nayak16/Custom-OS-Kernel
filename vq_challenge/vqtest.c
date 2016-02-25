/** @file vqtest.c
 *  @brief A very simple test suite for variable queues.
 *
 *  @author Ryan Pearl <rpearl>
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "variable_queue.h"

typedef struct node {
	Q_NEW_LINK(node) link;
	int data;
} node_t;

Q_NEW_HEAD(list_t, node);

#define LIST_LEN 5

void test_init() {
	list_t list;

	Q_INIT_HEAD(&list);

	assert(!Q_GET_FRONT(&list));
	assert(!Q_GET_TAIL(&list));
}

void test_insert() {
	list_t list;

	Q_INIT_HEAD(&list);

	node_t node;
	Q_INIT_ELEM(&node, link);

	node.data = 1;

	Q_INSERT_TAIL(&list, &node, link);

	assert(Q_GET_TAIL(&list) == &node);
	assert(Q_GET_FRONT(&list) == &node);

	assert(!Q_GET_NEXT(&node, link));
	assert(!Q_GET_PREV(&node, link));
}

void test_remove() {
	list_t list;

	Q_INIT_HEAD(&list);

	node_t node;
	Q_INIT_ELEM(&node, link);

	Q_INSERT_TAIL(&list, &node, link);

	assert(Q_GET_TAIL(&list) == &node);
	assert(Q_GET_FRONT(&list) == &node);

	assert(!Q_GET_NEXT(&node, link));
	assert(!Q_GET_PREV(&node, link));

	Q_REMOVE(&list, &node, link);

	assert(!Q_GET_FRONT(&list));
	assert(!Q_GET_TAIL(&list));
}

void test_insert_fronts() {
	list_t list;

	Q_INIT_HEAD(&list);

	node_t nodes[LIST_LEN];

	int i;
	for (i = 0; i < LIST_LEN; i++) {
		Q_INIT_ELEM(&nodes[i], link);
		nodes[i].data = i;
		Q_INSERT_FRONT(&list, &nodes[i], link);
	}

	node_t *cur = Q_GET_FRONT(&list);
	for (i = 0; i < LIST_LEN; i++) {
		assert(cur);
		assert(cur->data == LIST_LEN - i - 1);
		cur = Q_GET_NEXT(cur, link);
	}

	assert(!cur);

	cur = Q_GET_TAIL(&list);
	for (i = LIST_LEN-1; i >= 0; i--) {
		assert(cur);
		assert(cur->data == LIST_LEN - i - 1);
		cur = Q_GET_PREV(cur, link);
	}
	assert(!cur);
}

void test_insert_tails() {
	list_t list;

	Q_INIT_HEAD(&list);

	node_t nodes[LIST_LEN];

	int i;
	for (i = 0; i < LIST_LEN; i++) {
		Q_INIT_ELEM(&nodes[i], link);
		nodes[i].data = i;
		Q_INSERT_TAIL(&list, &nodes[i], link);
	}

	node_t *cur = Q_GET_FRONT(&list);
	for (i = 0; i < LIST_LEN; i++) {
		assert(cur);
		assert(cur->data == i);
		node_t *next = Q_GET_NEXT(cur, link);
		assert(!next || Q_GET_PREV(next, link) == cur);
		cur = next;
	}

	assert(!cur);

	cur = Q_GET_TAIL(&list);
	for (i = LIST_LEN-1; i >= 0; i--) {
		assert(cur);
		assert(cur->data == i);
		node_t *prev = Q_GET_PREV(cur, link);
		assert(!prev || Q_GET_NEXT(prev, link) == cur);
		cur = prev;
	}
	assert(!cur);
}

void test_removes() {
	list_t list;

	Q_INIT_HEAD(&list);

	node_t nodes[LIST_LEN];

	int i;
	for (i = 0; i < LIST_LEN; i++) {
		Q_INIT_ELEM(&nodes[i], link);
		Q_INSERT_FRONT(&list, &nodes[i], link);
	}

	assert(Q_GET_FRONT(&list) == &nodes[LIST_LEN-1]);
	assert(Q_GET_TAIL(&list) == &nodes[0]);

	node_t *cur = Q_GET_FRONT(&list);
	while (cur) {
		node_t *next = Q_GET_NEXT(cur, link);
		Q_REMOVE(&list, cur, link);
		cur = next;
		i++;
	}
}

void test_insert_after() {
	list_t list;

	Q_INIT_HEAD(&list);

	node_t nodes[LIST_LEN];

	Q_INIT_ELEM(&nodes[0], link);
	nodes[0].data = 0;
	Q_INSERT_FRONT(&list, &nodes[0], link);

	int i;
	for (i = 1; i < LIST_LEN; i++) {
		Q_INIT_ELEM(&nodes[i], link);
		Q_INSERT_AFTER(&list, &nodes[i-1], &nodes[i], link);
		nodes[i].data = i;
		assert(Q_GET_NEXT(&nodes[i-1], link) == &nodes[i]);
		assert(Q_GET_PREV(&nodes[i], link) == &nodes[i-1]);
	}
	assert(Q_GET_FRONT(&list) == &nodes[0]);
	assert(Q_GET_TAIL(&list) == &nodes[LIST_LEN-1]);

	node_t *cur = Q_GET_FRONT(&list);
	for (i = 0; i < LIST_LEN; i++) {
		assert(cur);
		assert(cur->data == i);
		cur = Q_GET_NEXT(cur, link);
	}
	assert(!cur);
}

void test_insert_before() {
	list_t list;

	Q_INIT_HEAD(&list);

	node_t nodes[LIST_LEN];

	Q_INIT_ELEM(&nodes[0], link);
	nodes[0].data = 0;
	Q_INSERT_FRONT(&list, &nodes[0], link);

	int i;
	for (i = 1; i < LIST_LEN; i++) {
		Q_INIT_ELEM(&nodes[i], link);
		Q_INSERT_BEFORE(&list, &nodes[i-1], &nodes[i], link);
		nodes[i].data = i;
		assert(Q_GET_PREV(&nodes[i-1], link) == &nodes[i]);
		assert(Q_GET_NEXT(&nodes[i], link) == &nodes[i-1]);
	}
	assert(Q_GET_TAIL(&list) == &nodes[0]);
	assert(Q_GET_FRONT(&list) == &nodes[LIST_LEN-1]);

	node_t *cur = Q_GET_FRONT(&list);
	for (i = 0; i < LIST_LEN; i++) {
		assert(cur);
		assert(cur->data == LIST_LEN - i - 1);
		cur = Q_GET_NEXT(cur, link);
	}
	assert(!cur);
}

#define RUN_TEST(t) do {\
	printf("Running "#t"()...");\
	t();\
	printf(" OK.\n");\
} while(0)

int main() {
	RUN_TEST(test_init);
	RUN_TEST(test_insert);
	RUN_TEST(test_insert_fronts);
	RUN_TEST(test_insert_tails);
	RUN_TEST(test_insert_before);
	RUN_TEST(test_insert_after);
	RUN_TEST(test_remove);
	return 0;
}
