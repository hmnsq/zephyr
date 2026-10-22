/*
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_
#define ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_

#अन्तर्भाव <zephyr/sys/math_extras.h>
#अन्तर्भाव <zephyr/sys/dlist.h>

/* Simple Scheduling */
#जर defined(CONFIG_SCHED_SIMPLE)
#define _priq_run_init		z_priq_simple_init
#define _priq_run_add		z_priq_simple_add
#define _priq_run_remove	z_priq_simple_remove
#define _priq_run_yield         z_priq_simple_yield
# जर defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_simple_mask_best
# अन्यथा
#  define _priq_run_best	z_priq_simple_best
# endif /* CONFIG_SCHED_CPU_MASK */
/* Scalable Scheduling */
#किंवाजर defined(CONFIG_SCHED_SCALABLE)
#define _priq_run_init		z_priq_rb_init
#define _priq_run_add		z_priq_rb_add
#define _priq_run_remove	z_priq_rb_remove
#define _priq_run_yield         z_priq_rb_yield
# जर defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_rb_mask_best
# अन्यथा
#  define _priq_run_best	z_priq_rb_best
# endif /* CONFIG_SCHED_CPU_MASK */
 /* Multi Queue Scheduling */
#किंवाजर defined(CONFIG_SCHED_MULTIQ)
#define _priq_run_init		z_priq_mq_init
#define _priq_run_add		z_priq_mq_add
#define _priq_run_remove	z_priq_mq_remove
#define _priq_run_yield         z_priq_mq_yield
# जर defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_mq_mask_best
# अन्यथा
#  define _priq_run_best	z_priq_mq_best
# endif /* CONFIG_SCHED_CPU_MASK */
#endif

/* Scalable Wait Queue */
#जर defined(CONFIG_WAITQ_SCALABLE)
#define _priq_wait_add		z_priq_rb_add
#define _priq_wait_remove	z_priq_rb_remove
#define _priq_wait_best		z_priq_rb_best
/* Simple Wait Queue */
#किंवाजर defined(CONFIG_WAITQ_SIMPLE)
#define _priq_wait_add		z_priq_simple_add
#define _priq_wait_remove	z_priq_simple_remove
#define _priq_wait_best		z_priq_simple_best
#endif

#जर defined(CONFIG_64BIT)
#define NBITS          64
#define TRAILING_ZEROS u64_मोजणी_trailing_zeros
#अन्यथा
#define NBITS          32
#define TRAILING_ZEROS u32_मोजणी_trailing_zeros
#endif /* CONFIG_64BIT */

स्थिर ALWAYS_INLINE शून्य z_priq_simple_init(sys_dlist_t *pq)
{
	sys_dlist_init(pq);
}

/*
 * Return value same as e.g. memcmp
 * > 0 -> thread 1 priority  > thread 2 priority
 * = 0 -> thread 1 priority == thread 2 priority
 * < 0 -> thread 1 priority  < thread 2 priority
 * Do not rely on the actual value returned aside from the above.
 * (Again, like memcmp.)
 */
स्थिर ALWAYS_INLINE int32_t z_sched_prio_cmp(संरचना k_thread *thread_1, संरचना k_thread *thread_2)
{
	/* `prio` is <32b, so the below cannot overflow. */
	int32_t b1 = thread_1->base.prio;
	int32_t b2 = thread_2->base.prio;

	जर (b1 != b2) {
		परत b2 - b1;
	}

#ifdef CONFIG_SCHED_DEADLINE
	/* If we assume all deadlines live within the same "half" of
	 * the 32 bit modulus space (this is a documented API rule),
	 * then the latest deadline in the queue minus the earliest is
	 * guaranteed to be (2's complement) non-negative.  We can
	 * leverage that to compare the values without having to check
	 * the current time.
	 */
	uint32_t d1 = thread_1->base.prio_deadline;
	uint32_t d2 = thread_2->base.prio_deadline;

	जर (d1 != d2) {
		/* Sooner deadline means higher effective priority.
		 * Doing the calculation with unsigned types and casting
		 * to signed isn't perfect, but at least reduces this
		 * from UB on overflow to impdef.
		 */
		परत (int32_t)(d2 - d1);
	}
#endif /* CONFIG_SCHED_DEADLINE */
	परत 0;
}

स्थिर ALWAYS_INLINE शून्य z_priq_simple_add(sys_dlist_t *pq, संरचना k_thread *thread)
{
	संरचना k_thread *t;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, t, base.qnode_dlist) {
		जर (z_sched_prio_cmp(thread, t) > 0) {
			sys_dlist_घाला(&t->base.qnode_dlist, &thread->base.qnode_dlist);
			परत;
		}
	}

	sys_dlist_append(pq, &thread->base.qnode_dlist);
}

स्थिर ALWAYS_INLINE शून्य z_priq_simple_remove(sys_dlist_t *pq, संरचना k_thread *thread)
{
	ARG_UNUSED(pq);

	sys_dlist_remove(&thread->base.qnode_dlist);
}

स्थिर ALWAYS_INLINE शून्य z_priq_simple_yield(sys_dlist_t *pq)
{
#ifndef CONFIG_SMP
	sys_dnode_t *n;

	n = sys_dlist_peek_next_no_check(pq, &_current->base.qnode_dlist);

	sys_dlist_dequeue(&_current->base.qnode_dlist);

	संरचना k_thread *t;

	/*
	 * As it is possible that the current thread was not at the head of
	 * the run queue, start searching from the present position for where
	 * to re-insert it.
	 */

	पर्यंत (n != रिक्त) {
		t = CONTAINER_OF(n, संरचना k_thread, base.qnode_dlist);
		जर (z_sched_prio_cmp(_current, t) > 0) {
			sys_dlist_घाला(&t->base.qnode_dlist,
					 &_current->base.qnode_dlist);
			परत;
		}
		n = sys_dlist_peek_next_no_check(pq, n);
	}

	sys_dlist_append(pq, &_current->base.qnode_dlist);
#endif
}

स्थिर ALWAYS_INLINE संरचना k_thread *z_priq_simple_best(sys_dlist_t *pq)
{
	संरचना k_thread *thread = रिक्त;
	sys_dnode_t *n = sys_dlist_peek_head(pq);

	जर (n != रिक्त) {
		thread = CONTAINER_OF(n, संरचना k_thread, base.qnode_dlist);
	}
	परत thread;
}

#ifdef CONFIG_SCHED_CPU_MASK
स्थिर ALWAYS_INLINE संरचना k_thread *z_priq_simple_mask_best(sys_dlist_t *pq)
{
	/* With masks enabled we need to be prepared to walk the list
	 * looking for one we can run
	 */
	अचिन्हित दीर्घ current_cpu_bit = BIT(_current_cpu->id);
	संरचना k_thread *thread;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, thread, base.qnode_dlist) {
		जर ((thread->base.cpu_mask & current_cpu_bit) != 0) {
			परत thread;
		}
	}
	परत रिक्त;
}
#endif /* CONFIG_SCHED_CPU_MASK */

#जर defined(CONFIG_SCHED_SCALABLE) || defined(CONFIG_WAITQ_SCALABLE)
स्थिर ALWAYS_INLINE शून्य z_priq_rb_init(संरचना _priq_rb *pq)
{
	bool z_priq_rb_lessthan(संरचना rbnode *a, संरचना rbnode *b);

	*pq = (संरचना _priq_rb) {
		.वृक्ष = {
			.lessthan_fn = z_priq_rb_lessthan,
		}
	};
}

स्थिर ALWAYS_INLINE शून्य z_priq_rb_add(संरचना _priq_rb *pq, संरचना k_thread *thread)
{
	संरचना k_thread *t;

	thread->base.order_किल्ली = pq->next_order_किल्ली;
	++pq->next_order_किल्ली;

	/* Renumber at wraparound.  This is tiny code, and in practice
	 * will almost never be hit on real systems.  BUT on very
	 * long-running systems where a priq never completely empties
	 * AND that contains very large numbers of threads, it can be
	 * a latency glitch to loop over all the threads like this.
	 */
	जर (!pq->next_order_किल्ली) {
		RB_FOR_EACH_CONTAINER(&pq->वृक्ष, t, base.qnode_rb) {
			t->base.order_किल्ली = pq->next_order_किल्ली;
			++pq->next_order_किल्ली;
		}
	}

	rb_घाला(&pq->वृक्ष, &thread->base.qnode_rb);
}

स्थिर ALWAYS_INLINE शून्य z_priq_rb_remove(संरचना _priq_rb *pq, संरचना k_thread *thread)
{
	rb_remove(&pq->वृक्ष, &thread->base.qnode_rb);

	जर (!pq->वृक्ष.मूळ) {
		pq->next_order_किल्ली = 0;
	}
}

स्थिर ALWAYS_INLINE शून्य z_priq_rb_yield(संरचना _priq_rb *pq)
{
#ifndef CONFIG_SMP
	z_priq_rb_remove(pq, _current);
	z_priq_rb_add(pq, _current);
#endif
}

स्थिर ALWAYS_INLINE संरचना k_thread *z_priq_rb_best(संरचना _priq_rb *pq)
{
	संरचना k_thread *thread = रिक्त;
	संरचना rbnode *n = rb_get_min(&pq->वृक्ष);

	जर (n != रिक्त) {
		thread = CONTAINER_OF(n, संरचना k_thread, base.qnode_rb);
	}
	परत thread;
}

#ifdef CONFIG_SCHED_CPU_MASK
स्थिर ALWAYS_INLINE संरचना k_thread *z_priq_rb_mask_best(संरचना _priq_rb *pq)
{
	अचिन्हित दीर्घ current_cpu_bit = BIT(_current_cpu->id);
	संरचना k_thread *thread;

	RB_FOR_EACH_CONTAINER(&pq->वृक्ष, thread, base.qnode_rb) {
		जर ((thread->base.cpu_mask & current_cpu_bit) != 0) {
			परत thread;
		}
	}
	परत रिक्त;
}
#endif /* CONFIG_SCHED_CPU_MASK */
#endif /* CONFIG_SCHED_SCALABLE || CONFIG_WAITQ_SCALABLE */

संरचना prio_info {
	uint8_t offset_prio;
	uint8_t idx;
	uint8_t bit;
};

स्थिर ALWAYS_INLINE संरचना prio_info get_prio_info(int8_t old_prio)
{
	संरचना prio_info ret;

	ret.offset_prio = old_prio - K_HIGHEST_THREAD_PRIO;
	ret.idx = ret.offset_prio / NBITS;
	ret.bit = ret.offset_prio % NBITS;

	परत ret;
}

स्थिर ALWAYS_INLINE अचिन्हित पूर्णांक z_priq_mq_best_queue_index(संरचना _priq_mq *pq)
{
	अचिन्हित पूर्णांक i = 0;

	करा {
		जर (likely(pq->bitmask[i])) {
			परत i * NBITS + TRAILING_ZEROS(pq->bitmask[i]);
		}
		i++;
	} पर्यंत (i < PRIQ_BITMAP_SIZE);

	परत K_अंक_THREAD_PRIO - 1;
}

स्थिर ALWAYS_INLINE शून्य z_priq_mq_init(संरचना _priq_mq *q)
{
	साठी (माप_प्रकार i = 0; i < ARRAY_SIZE(q->queues); i++) {
		sys_dlist_init(&q->queues[i]);
	}

#ifndef CONFIG_SMP
	q->cached_queue_index = K_अंक_THREAD_PRIO - 1;
#endif
}

स्थिर ALWAYS_INLINE शून्य z_priq_mq_add(संरचना _priq_mq *pq,
					संरचना k_thread *thread)
{
	संरचना prio_info pos = get_prio_info(thread->base.prio);

	sys_dlist_append(&pq->queues[pos.offset_prio], &thread->base.qnode_dlist);
	pq->bitmask[pos.idx] |= BIT(pos.bit);

#ifndef CONFIG_SMP
	जर (pos.offset_prio < pq->cached_queue_index) {
		pq->cached_queue_index = pos.offset_prio;
	}
#endif
}

स्थिर ALWAYS_INLINE शून्य z_priq_mq_remove(संरचना _priq_mq *pq,
					   संरचना k_thread *thread)
{
	संरचना prio_info pos = get_prio_info(thread->base.prio);

	sys_dlist_dequeue(&thread->base.qnode_dlist);
	जर (unlikely(sys_dlist_is_empty(&pq->queues[pos.offset_prio]))) {
		pq->bitmask[pos.idx] &= ~BIT(pos.bit);
#ifndef CONFIG_SMP
		pq->cached_queue_index = z_priq_mq_best_queue_index(pq);
#endif
	}
}

स्थिर ALWAYS_INLINE शून्य z_priq_mq_yield(संरचना _priq_mq *pq)
{
#ifndef CONFIG_SMP
	संरचना prio_info pos = get_prio_info(_current->base.prio);

	sys_dlist_dequeue(&_current->base.qnode_dlist);
	sys_dlist_append(&pq->queues[pos.offset_prio],
			 &_current->base.qnode_dlist);
#endif
}

स्थिर ALWAYS_INLINE संरचना k_thread *z_priq_mq_best(संरचना _priq_mq *pq)
{
#ifdef CONFIG_SMP
	अचिन्हित पूर्णांक index = z_priq_mq_best_queue_index(pq);
#अन्यथा
	अचिन्हित पूर्णांक index = pq->cached_queue_index;
#endif

	sys_dnode_t *n = sys_dlist_peek_head(&pq->queues[index]);

	जर (likely(n != रिक्त)) {
		परत CONTAINER_OF(n, संरचना k_thread, base.qnode_dlist);
	}

	परत रिक्त;
}

#ifdef CONFIG_SCHED_CPU_MASK
स्थिर ALWAYS_INLINE संरचना k_thread *z_priq_mq_mask_best(संरचना _priq_mq *pq)
{
	अचिन्हित दीर्घ current_cpu_bit = BIT(_current_cpu->id);

	/* Walk priority levels from highest to lowest, and within each
	 * level walk the thread list to find the first thread runnable
	 * on the current CPU.
	 */
	साठी (अचिन्हित पूर्णांक i = 0U; i < PRIQ_BITMAP_SIZE; i++) {
		अचिन्हित दीर्घ bitmask = pq->bitmask[i];

		पर्यंत (bitmask != 0UL) {
			अचिन्हित पूर्णांक bit = TRAILING_ZEROS(bitmask);
			अचिन्हित पूर्णांक index = i * NBITS + bit;
			संरचना k_thread *thread;

			SYS_DLIST_FOR_EACH_CONTAINER(&pq->queues[index],
						     thread, base.qnode_dlist) {
				जर ((thread->base.cpu_mask &
				     current_cpu_bit) != 0) {
					परत thread;
				}
			}
			bitmask &= (bitmask - 1UL);
		}
	}
	परत रिक्त;
}
#endif /* CONFIG_SCHED_CPU_MASK */

#endif /* ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_ */
