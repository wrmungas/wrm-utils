#include "wrm/memory.h"



bool wrm_Tree_init(wrm_Tree *tree, void *src, u32 type, size_t tn_offset, size_t child_limit)
{
    if(!tree || !src) { return false; }

    size_t capacity;
    bool auto_reserve;
    switch(type) {
        case WRM_STACK:
            wrm_Stack *s = src;

            if(tn_offset > s->element_size - sizeof(wrm_Tree_Node)) { return false; }
            capacity = s->cap;
            auto_reserve = s->auto_reserve;
            break;
        case WRM_POOL:
            wrm_Pool *p = src;

            if(tn_offset > p->element_size - sizeof(wrm_Tree_Node)) { return false; }
            capacity = p->cap;
            auto_reserve = p->auto_reserve;
            break;
        default:
            return false;
    }

    tree->child_limit = child_limit;
    tree->tn_offset = tn_offset;

    if(!wrm_Pool_init(&tree->child_lists, capacity, child_limit * sizeof(u32), auto_reserve)) {
        return false;
    }

    return true;
}

bool wrm_Tree_hasChild(wrm_Tree *tree, u32 parent, u32 child)
{
    if(!tree) { return false; }
    wrm_Tree_Node *p = wrm_Tree_offsetAt(tree, parent, tree->tn_offset);
    wrm_Tree_Node *c = wrm_Tree_offsetAt(tree, child, tree->tn_offset);
    
    // ensure parent and child exist in src
    if(!p || !c) {
        return false;
    }

    if(p->child_count == 0) { return false; }
    if(p->child_count == 1) { return p->children == child; }
    u32 *children = wrm_Pool_at(&tree->child_lists, p->children);

    for(u8 i = 0; i < p->child_count; i++) {
        if(children[i] == child) { return true; }
    }
    
    return false;
}

void wrm_Tree_makeRoot(wrm_Tree *tree, u32 node)
{
    if(!tree) { return; }
    wrm_Tree_Node *n = wrm_Tree_offsetAt(tree->src, node, tree->tn_offset);
    if(!n) { return; }

    // if n has a parent, orphan it
    if(!n->root) {
        if(wrm_Tree_hasChild(tree, n->parent, node)) {
            wrm_Tree_removeChild(tree, n->parent, node);
        }
        n->root = true;
    }
}

bool wrm_Tree_addChild(wrm_Tree *tree, u32 parent, u32 child)
{
    if(!tree) { return false; }
    wrm_Tree_Node *p = wrm_Tree_offsetAt(tree, parent, tree->tn_offset);
    wrm_Tree_Node *c = wrm_Tree_offsetAt(tree, child, tree->tn_offset);

    // ensure parent and child exist in src
    if(!p || !c) {
        return false;
    }

    if(!c->root) { // if child tree node is not a root, it already has a parent: must dissociate those properly first
        return false;
    }
    // ensure parent does not already have child and has room for it
    if(p->child_count == tree->child_limit || wrm_Tree_hasChild(tree, parent, child)) {
        return false;
    }


    c->parent = parent;

    if(p->child_count == 0) {
        p->child_count++;
        p->children = child;
        c->root = false;
        return true;
    }

    u32 *children;

    if(p->child_count == 1) {
        wrm_Option_Handle result = wrm_Pool_getSlot(&tree->child_lists);
        if(!result.exists) { return false; }
        u32 tmp = p->children;
        p->children = result.val;

        children = wrm_Pool_at(&tree->child_lists, p->children);
        children[0] = tmp;
    }
    else {
        u32 *children = wrm_Pool_at(&tree->child_lists, p->children);
    }

    children[p->child_count++] = child;
    return true;
}

bool wrm_Tree_removeChild(wrm_Tree *tree, u32 parent, u32 child)
{
    if(!tree) { return false; }
    wrm_Tree_Node *p = wrm_Tree_offsetAt(tree, parent, tree->tn_offset);
    wrm_Tree_Node *c = wrm_Tree_offsetAt(tree, child, tree->tn_offset);

    // ensure parent and child exist in src
    if(!p || !c) {
        return false;
    }

    if(p->child_count == 0) {
        return false; // cannot remove if no children
    }

    // cannot remove if child and parent are not already associated
    if(!wrm_Tree_hasChild(tree, parent, child) || c->root) {
        return false; 
    }
    
    if(p->child_count == 1) {
        p->child_count == 0;
        c->root = true;
        return true;
    }

    u32 *children = wrm_Pool_at(&tree->child_lists, p->children);

    // find the child
    u8 idx;
    for(idx = 0; idx < p->child_count; idx++) {
        if(children[idx] == child) break;
    }
    if(idx == tree->child_limit) {
        return false; // could not find child
    }

    // dissociate, shift child list data over
    c->root = true;
    p->child_count--;
    while(idx < p->child_count) {
        children[idx] = children[idx + 1];
        idx++;
    }

    // free list if we only have one child
    if(p->child_count == 1) {
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
    tree->tn_offset = 0;
}

// ensure compiler emits symbol

void *wrm_Tree_offsetAt(wrm_Tree *tree, u32 idx, size_t offset);

