#include "wrm/memory.h"

typedef struct Test {
    wrm_Tree_Node node;
} Test;

void printChildren(wrm_Handle idx, Test *test, wrm_Tree *tree)
{
    if(!test) { return; }

    printf("struct Test [%u]: { has_parent: %s, ", idx, test->node.has_parent ? "true" : "false");

    if(test->node.has_parent) {
        printf("parent: %u, ", test->node.parent);
    }
    printf("child_cnt: %u", test->node.child_cnt);
    if(test->node.child_cnt > 0) {
       printf(", children: %u", test->node.children);
    }
    if(test->node.child_cnt > 1) {
        u32 *children = wrm_Pool_at(&tree->child_lists, test->node.children);
        printf("{ ");
        for(u8 i = 0; i < test->node.child_cnt; i++) {
            printf("%u ", children[i]);
        }
        printf("}");
    }
    printf(" }\n");
}

int main(int argv, char **argc)
{
        
    wrm_Pool p;
    wrm_Stack s;
    
    if(!wrm_Pool_init(&p, 20, sizeof(Test), false) || !wrm_Stack_init(&s, 20, sizeof(Test), false)) {
        wrm_fail(1, "Test", "start", "failed to initialize stack and pool");
    }
    
    wrm_Option_Handle result;


    // test getting slots from non-growable pool
    for(int i = 0; i < 20; i++) {
        result = wrm_Pool_getSlot(&p);
        if(!result.exists) wrm_fail(1, "Test", "static pool", "failed to get a slot from the pool");
    }
    printf("Pool test: used_count %zu, capacity %zu\n", p.used_cnt, p.cap);
    if(p.used_cnt != p.cap) wrm_fail(1, "Test", "static pool", "pool slots used does not equal capacity");
    result = wrm_Pool_getSlot(&p);
    if(result.exists) wrm_fail(1, "Test", "static pool", "got slot when pool should be at capacity");
    if(!wrm_Pool_reserve(&p, 40)) wrm_fail(1, "Test", "static pool", "failed to reserve space in pool");
    result = wrm_Pool_getSlot(&p);
    if(!result.exists) wrm_fail(1, "Test", "static pool", "failed to get slot in pool after resize");

    // test pushing to non-growable stack
    for(int i = 0; i < 20; i++) {
        result = wrm_Stack_push(&s);
        if(!result.exists) wrm_fail(1, "Test", "static stack", "failed to push the top of the stack");
    }
    printf("Stack test: used %zu, capacity %zu\n", s.len, s.cap);
    if(s.len != s.cap) wrm_fail(1, "Test", "static stack", "stack len does not equal capacity");
    result = wrm_Stack_push(&s);
    if(result.exists) wrm_fail(1, "Test", "static stack", "pushed to stack when it should be at capacity");
    if(!wrm_Stack_reserve(&s, 40)) wrm_fail(1, "Test", "static stack", "failed to reserve space in stack");
    result = wrm_Stack_push(&s);
    if(!result.exists) wrm_fail(1, "Test", "static stack", "failed to push to stack after resize");

    // test deletion
    wrm_Pool_delete(&p, NULL);
    wrm_Stack_delete(&s, NULL);

    if(p.in_use || p.data || s.data) wrm_fail(1, "Test", "delete", "stack and pool not properly deleted");

    if(!wrm_Pool_init(&p, 20, sizeof(Test), true) || !wrm_Stack_init(&s, 20, sizeof(Test), true)) {
        wrm_fail(1, "Test", "static test start", "failed to re-initialize stack and pool");
    }
    
    // test getting slots from growable pool
    for(int i = 0; i < 21; i++) {
        result = wrm_Pool_getSlot(&p);
        if(!result.exists) wrm_fail(1, "Test", "growable pool", "failed to get a slot from the pool");
    }
    printf("Pool growth test: used_count %zu, capacity %zu\n", p.used_cnt, p.cap);

    // test pushing to growable stack
    for(int i = 0; i < 21; i++) {
        result = wrm_Stack_push(&s);
        if(!result.exists) wrm_fail(1, "Test", "growable stack", "failed to push the top of the stack");
    }
    printf("Stack growth test: used_count %zu, capacity %zu\n", s.len, s.cap);



    wrm_Tree t;
    if(!wrm_Tree_init(&t, &p, offsetof(Test, node), 8, true)) {
        wrm_fail(1, "Test", "tree start", "failed to initialize tree");
    }

    printf("Initial tree contents: \n");
    for(int i = 0; i < p.cap; i++) {
        Test *test = wrm_Pool_at(&p, i);
        if(test && !wrm_Tree_makeRoot(&t, i)) {
            wrm_fail(1, "Test", "set roots", "could not set element %u to root!", i);
        }
        printChildren(i, test, &t);
    }

    for(u8 i = 0; i < t.child_limit; i++) {
        if(!wrm_Tree_addChild(&t, 0, i + 1)) {
            wrm_fail(1, "Test", "add children", "could not add child %u to %u", i + 1, 0);
        }
    }
    if(wrm_Tree_addChild(&t, 0, t.child_limit + 1)) {
        wrm_fail(1, "Test", "add children", "should not be able to add another child to %u", 0);
    }
    printf("After adding:\n");
    for(int i = 0; i < p.cap; i++) {
        Test *test = wrm_Pool_at(&p, i);
        printChildren(i, test, &t);
    }

    for(u8 i = 0; i < t.child_limit; i++) {
        if(!wrm_Tree_removeChild(&t, 0, i + 1)) {
            wrm_fail(1, "Test", "remove children", "could not remove child %u from %u", i + 1, 0);
        }
    }
    printf("After removing:\n");
    for(int i = 0; i < p.cap; i++) {
        Test *test = wrm_Pool_at(&p, i);
        printChildren(i, test, &t);
    }
    if(wrm_Tree_removeChild(&t, 0, t.child_limit + 1)) {
        wrm_fail(1, "Test", "remove children", "should not be able to remove another child from %u", 0);
    }

    wrm_Tree_addChild(&t, 0, 1);
    if(wrm_Tree_addChild(&t, 0, 1)) wrm_fail(1, "Test", "add double children", "should not be able to add the same child twice");

    if(wrm_Tree_removeChild(&t, 0, 2)) wrm_fail(1, "Test", "remove non-existent child", "should not be able to remove child that parent doesn't have");

    if(wrm_Tree_addChild(&t, 0, 100)) wrm_fail(1, "Test", "add invalid", "should not be able to add an invalid child");
    if(wrm_Tree_addChild(&t, 100, 0)) wrm_fail(1, "Test", "add invalid", "should not be able to add an invalid parent");

    printf("SUCCESS\n");
}
