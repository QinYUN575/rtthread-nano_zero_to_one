/**
 * @file rtservice.h
 * @author TsMax (QinYUN575@Foxmail.com)
 * @brief
 * @version 0.1
 * @date 2021-07-02
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef __RT_SERVICE_H__
#define __RT_SERVICE_H__

#include <rtdef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup KernelService
 */

/**
 * rt_container _of - return the member address of ptr, if the type of ptr is the
 * struct type.
 */
#define rt_container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

/**
 * @brief initialize a list.
 *
 * @param l list to be initialized.
 */
rt_inline void rt_list_init(rt_list_t *l)
{
    l->next = l->prev = l;
}

/**
 * @brief insert a node after a list.
 *
 * @param l list to insert it.
 * @param n new node to be inserted.
 */
rt_inline void rt_list_insert_after(rt_list_t *l, rt_list_t *n)
{
    l->next->prev = n;
    n->next = l->next;

    l->next = n;
    n->prev = l;
}

/**
 * @brief insert a node before a list.
 *
 * @param l list to insert it.
 * @param n new node to be inserted.
 */
rt_inline void rt_list_insert_before(rt_list_t *l, rt_list_t *n)
{
    l->prev->next = n;
    n->prev = l->prev;

    l->prev = n;
    n->next = l;
}

/**
 * @brief remove node from list.
 *
 * @param n the node to remove from the list.
 */
rt_inline void rt_list_remove(rt_list_t *n)
{
    n->next->prev = n->prev;
    n->prev->next = n->next;

    n->next = n->prev = n;
}

/**
 * @brief tests whether a list is empty
 * 
 * @param l the list to tests
 * @return result of list is empty
 */
rt_inline rt_bool_t rt_list_isempty(rt_list_t *l)
{
    return (l->next == l);
}

/**
 * @brief get the struct for this entry
 * @param node the entry point
 * @param type the type of structure
 * @param member the name of list in structure
 */
#define rt_list_entry(node, type, member) \
    rt_container_of(node, type, member)


#ifdef __cplusplus
}
#endif

#endif
