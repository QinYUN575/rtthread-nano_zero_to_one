/**
 * @file rtthread.c
 * @author TsMax (QinYUN575@Foxmail.com)
 * @brief
 * @version 0.1
 * @date 2021-07-02
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <rthw.h>
#include <rtthread.h>

extern struct rt_thread *rt_current_thread;
extern rt_uint32_t rt_thread_ready_priority_group;
extern rt_list_t rt_thread_priority_table[RT_THREAD_PRIORITY_MAX];
/**
 * @brief
 *
 * @param thread
 * @param entry
 * @param parameter
 * @param stack_start
 * @param stack_size
 * @return rt_err_t
 */
rt_err_t rt_thread_init(struct rt_thread *thread,
                        const char *name,
                        void (*entry)(void *parameter),
                        void *parameter,
                        void *stack_start,
                        rt_uint32_t stack_size,
                        rt_uint8_t priority,
                        rt_uint32_t tick)
{
    /* initialize thread object */
    rt_object_init((rt_object_t)thread, RT_Object_Class_Thread, name);
    rt_list_init(&(thread->tlist));

    thread->entry = (void *)entry;
    thread->parameter = parameter;

    thread->stack_addr = stack_start;
    thread->stack_size = stack_size;

    thread->sp = (void *)rt_hw_stack_init(thread->entry,
                                          thread->parameter,
                                          (void *)((char *)thread->stack_addr + thread->stack_size - 4));

    thread->init_priority = priority;
    thread->current_priority = priority;
    thread->number_mask = 0;
    thread->error = RT_EOK;
    thread->stat = RT_THREAD_INIT;

    thread->init_tick = tick;
    thread->remaining_tick = tick;

    rt_timer_init(&(thread->thread_timer),
                  thread->name,
                  rt_thread_timeout,
                  thread,
                  0,
                  RT_TIMER_FLAG_ONE_SHOT);

    return RT_EOK;
}

rt_err_t rt_thread_suspend(rt_thread_t thread)
{
    register rt_base_t level;

    if ((thread->stat & RT_THREAD_STAT_MASK) != RT_THREAD_READY)
    {
        return -RT_ERROR;
    }
    level = rt_hw_interrupt_disable();

    thread->stat = RT_THREAD_SUSPEND;
    rt_schedule_remove_thread(thread);
    rt_timer_stop(&(thread->thread_timer));

    rt_hw_interrupt_enable(level);

    return RT_EOK;
}

rt_err_t rt_thread_sleep(rt_tick_t tick)
{
    register rt_base_t level;
    struct rt_thread *thread;
    level = rt_hw_interrupt_disable();
    thread = rt_current_thread;
    rt_thread_suspend(thread);
    rt_timer_control(&(thread->thread_timer), RT_TIMER_CTRL_SET_TIME, &tick);
    rt_timer_start(&(thread->thread_timer));
    rt_hw_interrupt_enable(level);

    rt_schedule();
    return RT_EOK;
}

void rt_thread_delay(rt_tick_t tick)
{
    rt_thread_sleep(tick);
}

rt_err_t rt_thread_resume(rt_thread_t thread)
{
    register rt_base_t level;

    if ((thread->stat & RT_THREAD_STAT_MASK) != RT_THREAD_SUSPEND)
    {
        return -RT_ERROR;
    }

    level = rt_hw_interrupt_disable();
    rt_list_remove(&(thread->tlist));
    rt_hw_interrupt_enable(level);

    rt_schedule_insert_thread(thread);

    return RT_EOK;
}

rt_thread_t rt_thread_self(void)
{
    return rt_current_thread;
}

rt_err_t rt_thread_startup(rt_thread_t thread)
{
    thread->current_priority = thread->init_priority;
    thread->number_mask = 1L << thread->current_priority;

    thread->stat = RT_THREAD_SUSPEND;

    rt_thread_resume(thread);

    if (rt_thread_self() != RT_NULL)
    {
        rt_schedule();
    }

    return RT_EOK;
}

void rt_thread_timeout(void *parameter)
{
    struct rt_thread *thread;

    thread = (struct rt_thread *)parameter;

    thread->error = -RT_ETIMEOUT;
    rt_list_remove(&(thread->tlist));

    rt_schedule_insert_thread(thread);
    rt_schedule();
}



rt_err_t rt_thread_yield(void)
{
    register rt_base_t level;
    struct rt_thread *thread;

    level = rt_hw_interrupt_disable();
    thread = rt_current_thread;

    if ((thread->stat & RT_THREAD_STAT_MASK) == RT_THREAD_READY 
        && thread->tlist.next != thread->tlist.prev)
    {
        rt_list_remove(&(thread->tlist));
        rt_list_insert_before(&(rt_thread_priority_table[thread->current_priority]),
                            &(thread->tlist));

        rt_hw_interrupt_enable(level);
        rt_schedule();
        return RT_EOK;
    }

    rt_hw_interrupt_enable(level);

    return RT_EOK;
}