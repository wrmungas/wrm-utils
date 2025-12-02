#include "wrm/memory.h"


bool wrm_Tree_init(wrm_Tree *tree, wrm_Pool *src, size_t offset, size_t child_limit, bool auto_reserve)
{
    if(!tree || !src) { return false; }

    tree->src = src;
    tree->child_limit = child_limit;
    tree->offset = offset;

    if(!wrm_Pool_init(&tree->child_lists, src->cap, child_limit * sizeof(u32), auto_reserve)) {
        return false;
    } 

    return true;
}

bool wrm_Tree_hasChild(wrm_Tree *tree, u32 parent, u32 child)
{
    if(!tree) { return false; }
    wrm_Tree_Node *p = wrm_Tree_at(tree, parent);
    wrm_Tree_Node *c = wrm_Tree_at(tree, child);
    
    // ensure parent and child exist in src
    if(!p || !c) {
        return false;
    }

    if(p->child_cnt == 0) { return false; }
    if(p->child_cnt == 1) { return p->children == child; }
    u32 *children = wrm_Pool_at(&tree->child_lists, p->children);

    for(u8 i = 0; i < p->child_cnt; i++) {
        if(children[i] == child) { return true; }
    }
    
    return false;
}

bool wrm_Tree_makeRoot(wrm_Tree *tree, u32 node)
{
    if(!tree) { return false; }
    wrm_Tree_Node *n = wrm_Tree_at(tree, node);
    if(!n) { return false; }

    // if n has a parent, orphan it

    if(n->has_parent && wrm_Tree_hasChild(tree, n->parent, node)) {
        return wrm_Tree_removeChild(tree, n->parent, node);
    }
    n->has_parent = false;
    return true;
}

bool wrm_Tree_addChild(wrm_Tree *tree, u32 parent, u32 child)
{
    if(!tree) { return false; }
    wrm_Tree_Node *p = wrm_Tree_at(tree, parent);
    wrm_Tree_Node *c = wrm_Tree_at(tree, child);

    // ensure parent and child exist in src
    if(!p || !c) {
        return false;
    }

    if(c->has_parent) { // if child tree node already has a parent, must dissociate those properly first
        return false;
    }
    // ensure parent does not already have child and has room for it
    if(p->child_cnt == tree->child_limit || wrm_Tree_hasChild(tree, parent, child)) {
        return false;
    }


    c->parent = parent;
    c->has_parent = true;

    if(p->child_cnt == 0) {
        p->child_cnt++;
        p->children = child;
        return true;
    }

    u32 *children;

    if(p->child_cnt == 1) {
        wrm_Option_Handle result = wrm_Pool_getSlot(&tree->child_lists);
        if(!result.exists) { return false; }
        u32 tmp = p->children;
        p->children = result.val;

        children = wrm_Pool_at(&tree->child_lists, p->children);
        children[0] = tmp;
    }
    else {
        children = wrm_Pool_at(&tree->child_lists, p->children);
    }

    children[p->child_cnt++] = child;
    return true;
}

bool wrm_Tree_removeChild(wrm_Tree *tree, u32 parent, u32 child)
{
    if(!tree) { return false; }
    wrm_Tree_Node *p = wrm_Tree_at(tree, parent);
    wrm_Tree_Node *c = wrm_Tree_at(tree, child);

    // ensure parent and child exist in src
    if(!p || !c) {
        return false;
    }

    if(p->child_cnt == 0) {
        return false; // cannot remove if no children
    }

    // cannot remove if child and parent are not already associated
    if(!wrm_Tree_hasChild(tree, parent, child) || !c->has_parent) {
        return false; 
    }
    
    if(p->child_cnt == 1) {
        p->child_cnt = 0;  
        c->has_parent = false;  
        return true;
    }

    u32 *children = wrm_Pool_at(&tree->child_lists, p->children);

    // find the child
    u8 idx;
    for(idx = 0; idx < p->child_cnt; idx++) {
        if(children[idx] == child) break;
    }
    if(idx == tree->child_limit) {
        return false; // could not find child
    }

    // dissociate, shift child list data over
    p->child_cnt--;
    c->has_parent = false;
    while(idx < p->child_cnt) {
        children[idx] = children[idx + 1];
        idx++;
    }

    // free list if we only have one child
    if(p->child_cnt == 1) {
        wrm_Pool_freeSlot(&tree->child_lists, p->children);
        p->children = children[0];
    }
    
    return true;
}

void wrm_Tree_delete(wrm_Tree *tree)
{
    if(!tree || !tree->src ) { return; }
    
    wrm_Pool_delete(&tree->child_lists, NULL); // no special cleanup needed
    tree->src = NULL;
    tree->child_limit = 0;
    tree->offset = 0;
}

void wrm_Tree_debugNode(wrm_Tree_Node *tn, wrm_Tree *tree) {
    if(!tn) return;
    printf("{ has_parent: %s, ", tn->has_parent ? "true" : "false");

    if(tn->has_parent) {
        printf("parent: %u, ", tn->parent);
    }
    printf("child_count: %u", tn->child_cnt);
    if(tn->child_cnt > 0) {
       printf(", children: %u", tn->children);
    }
    if(tn->child_cnt > 1) {
        u32 *children = wrm_Pool_at(&tree->child_lists, tn->children);
        printf("{ ");
        for(u8 i = 0; i < tn->child_cnt; i++) {
            printf("%u ", children[i]);
        }
        printf("}");
    }
    printf(" }");
}

// ensure compiler emits symbol

wrm_Tree_Node *wrm_Tree_at(wrm_Tree *tree, u32 idx);

