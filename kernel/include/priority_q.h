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
#सचे defined(CONFIG_SCHED_SIMPLE)
#define _priq_run_init		z_priq_simple_init
#define _priq_run_add		z_priq_simple_add
#define _priq_run_remove	z_priq_simple_remove
#define _priq_run_yield         z_priq_simple_yield
# सचे defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_simple_mask_best
# अञ्ञथा
#  define _priq_run_best	z_priq_simple_best
# endif /* CONFIG_SCHED_CPU_MASK */
/* Scalable Scheduling */
#नोचेद्यदि defined(CONFIG_SCHED_SCALABLE)
#define _priq_run_init		z_priq_rb_init
#define _priq_run_add		z_priq_rb_add
#define _priq_run_remove	z_priq_rb_remove
#define _priq_run_yield         z_priq_rb_yield
# सचे defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_rb_mask_best
# अञ्ञथा
#  define _priq_run_best	z_priq_rb_best
# endif /* CONFIG_SCHED_CPU_MASK */
 /* Multi Queue Scheduling */
#नोचेद्यदि defined(CONFIG_SCHED_MULTIQ)
#define _priq_run_init		z_priq_mq_init
#define _priq_run_add		z_priq_mq_add
#define _priq_run_remove	z_priq_mq_remove
#define _priq_run_yield         z_priq_mq_yield
# सचे defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_mq_mask_best
# अञ्ञथा
#  define _priq_run_best	z_priq_mq_best
# endif /* CONFIG_SCHED_CPU_MASK */
#endif

/* Scalable Wait Queue */
#सचे defined(CONFIG_WAITQ_SCALABLE)
#define _priq_wait_add		z_priq_rb_add
#define _priq_wait_remove	z_priq_rb_remove
#define _priq_wait_best		z_priq_rb_best
/* Simple Wait Queue */
#नोचेद्यदि defined(CONFIG_WAITQ_SIMPLE)
#define _priq_wait_add		z_priq_simple_add
#define _priq_wait_remove	z_priq_simple_remove
#define _priq_wait_best		z_priq_simple_best
#endif

#सचे defined(CONFIG_64BIT)
#define NBITS          64
#define TRAILING_ZEROS u64_गणनम्_trailing_zeros
#अञ्ञथा
#define NBITS          32
#define TRAILING_ZEROS u32_गणनम्_trailing_zeros
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

	सचे (b1 != b2) {
		पटिनिवत्त b2 - b1;
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

	सचे (d1 != d2) {
		/* Sooner deadline means higher effective priority.
		 * Doing the calculation with unsigned types and casting
		 * to signed isn't perfect, but at least reduces this
		 * from UB on overflow to impdef.
		 */
		पटिनिवत्त (int32_t)(d2 - d1);
	}
#endif /* CONFIG_SCHED_DEADLINE */
	पटिनिवत्त 0;
}

स्थिर ALWAYS_INLINE शून्य z_priq_simple_add(sys_dlist_t *pq, संरचना k_thread *thread)
{
	संरचना k_thread *t;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, t, base.qnode_dlist) {
		सचे (z_sched_prio_cmp(thread, t) > 0) {
			sys_dlist_निवेशय(&t->base.qnode_dlist, &thread->base.qnode_dlist);
			पटिनिवत्त;
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

	याव (n != रिक्त) {
		t = CONTAINER_OF(n, संरचना k_thread, base.qnode_dlist);
		सचे (z_sched_prio_cmp(_current, t) > 0) {
			sys_dlist_निवेशय(&t->base.qnode_dlist,
					 &_current->base.qnode_dlist);
			पटिनिवत्त;
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

	सचे (n != रिक्त) {
		thread = CONTAINER_OF(n, संरचना k_thread, base.qnode_dlist);
	}
	पटिनिवत्त thread;
}

#ifdef CONFIG_SCHED_CPU_MASK
स्थिर ALWAYS_INLINE संरचना k_thread *z_priq_simple_mask_best(sys_dlist_t *pq)
{
	/* With masks enabled we need to be prepared to walk the list
	 * looking for one we can run
	 */
	unsigned long current_cpu_bit = BIT(_current_cpu->id);
	संरचना k_thread *thread;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, thread, base.qnode_dlist) {
		सचे ((thread->base.cpu_mask & current_cpu_bit) != 0) {
			पटिनिवत्त thread;
		}
	}
	पटिनिवत्त रिक्त;
}
#endif /* CONFIG_SCHED_CPU_MASK */

#सचे defined(CONFIG_SCHED_SCALABLE) || defined(CONFIG_WAITQ_SCALABLE)
स्थिर ALWAYS_INLINE शून्य z_priq_rb_init(संरचना _priq_rb *pq)
{
	bool z_priq_rb_lessthan(संरचना rbnode *a, संरचना rbnode *b);

	*pq = (संरचना _priq_rb) {
		.वृक्षः = {
			.lessthan_fn = z_priq_rb_lessthan,
		}
	};
}

स्थिर ALWAYS_INLINE शून्य z_priq_rb_add(संरचना _priq_rb *pq, संरचना k_thread *thread)
{
	संरचना k_thread *t;

	thread->base.order_कुञ्चिका = pq->next_order_कुञ्चिका;
	++pq->next_order_कुञ्चिका;

	/* Renumber at wraparound.  This is tiny code, and in practice
	 * will almost never be hit on real systems.  BUT on very
	 * long-running systems where a priq never completely empties
	 * AND that contains very large numbers of threads, it can be
	 * a latency glitch to loop over all the threads like this.
	 */
	सचे (!pq->next_order_कुञ्चिका) {
		RB_FOR_EACH_CONTAINER(&pq->वृक्षः, t, base.qnode_rb) {
			t->base.order_कुञ्चिका = pq->next_order_कुञ्चिका;
			++pq->next_order_कुञ्चिका;
		}
	}

	rb_निवेशय(&pq->वृक्षः, &thread->base.qnode_rb);
}

स्थिर ALWAYS_INLINE शून्य z_priq_rb_remove(संरचना _priq_rb *pq, संरचना k_thread *thread)
{
	rb_remove(&pq->वृक्षः, &thread->base.qnode_rb);

	सचे (!pq->वृक्षः.मूलम्) {
		pq->next_order_कुञ्चिका = 0;
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
	संरचना rbnode *n = rb_get_min(&pq->वृक्षः);

	सचे (n != रिक्त) {
		thread = CONTAINER_OF(n, संरचना k_thread, base.qnode_rb);
	}
	पटिनिवत्त thread;
}

#ifdef CONFIG_SCHED_CPU_MASK
स्थिर ALWAYS_INLINE संरचना k_thread *z_priq_rb_mask_best(संरचना _priq_rb *pq)
{
	unsigned long current_cpu_bit = BIT(_current_cpu->id);
	संरचना k_thread *thread;

	RB_FOR_EACH_CONTAINER(&pq->वृक्षः, thread, base.qnode_rb) {
		सचे ((thread->base.cpu_mask & current_cpu_bit) != 0) {
			पटिनिवत्त thread;
		}
	}
	पटिनिवत्त रिक्त;
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

	पटिनिवत्त ret;
}

स्थिर ALWAYS_INLINE unsigned पूर्णाङ्क z_priq_mq_best_queue_index(संरचना _priq_mq *pq)
{
	unsigned पूर्णाङ्क i = 0;

	करोहि {
		सचे (likely(pq->bitmask[i])) {
			पटिनिवत्त i * NBITS + TRAILING_ZEROS(pq->bitmask[i]);
		}
		i++;
	} याव (i < PRIQ_BITMAP_SIZE);

	पटिनिवत्त K_अङ्कः_THREAD_PRIO - 1;
}

स्थिर ALWAYS_INLINE शून्य z_priq_mq_init(संरचना _priq_mq *q)
{
	अत्थाय (size_t i = 0; i < ARRAY_SIZE(q->queues); i++) {
		sys_dlist_init(&q->queues[i]);
	}

#ifndef CONFIG_SMP
	q->cached_queue_index = K_अङ्कः_THREAD_PRIO - 1;
#endif
}

स्थिर ALWAYS_INLINE शून्य z_priq_mq_add(संरचना _priq_mq *pq,
					संरचना k_thread *thread)
{
	संरचना prio_info pos = get_prio_info(thread->base.prio);

	sys_dlist_append(&pq->queues[pos.offset_prio], &thread->base.qnode_dlist);
	pq->bitmask[pos.idx] |= BIT(pos.bit);

#ifndef CONFIG_SMP
	सचे (pos.offset_prio < pq->cached_queue_index) {
		pq->cached_queue_index = pos.offset_prio;
	}
#endif
}

स्थिर ALWAYS_INLINE शून्य z_priq_mq_remove(संरचना _priq_mq *pq,
					   संरचना k_thread *thread)
{
	संरचना prio_info pos = get_prio_info(thread->base.prio);

	sys_dlist_dequeue(&thread->base.qnode_dlist);
	सचे (unlikely(sys_dlist_is_empty(&pq->queues[pos.offset_prio]))) {
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
	unsigned पूर्णाङ्क index = z_priq_mq_best_queue_index(pq);
#अञ्ञथा
	unsigned पूर्णाङ्क index = pq->cached_queue_index;
#endif

	sys_dnode_t *n = sys_dlist_peek_head(&pq->queues[index]);

	सचे (likely(n != रिक्त)) {
		पटिनिवत्त CONTAINER_OF(n, संरचना k_thread, base.qnode_dlist);
	}

	पटिनिवत्त रिक्त;
}

#ifdef CONFIG_SCHED_CPU_MASK
स्थिर ALWAYS_INLINE संरचना k_thread *z_priq_mq_mask_best(संरचना _priq_mq *pq)
{
	unsigned long current_cpu_bit = BIT(_current_cpu->id);

	/* Walk priority levels from highest to lowest, and within each
	 * level walk the thread list to find the first thread runnable
	 * on the current CPU.
	 */
	अत्थाय (unsigned पूर्णाङ्क i = 0U; i < PRIQ_BITMAP_SIZE; i++) {
		unsigned long bitmask = pq->bitmask[i];

		याव (bitmask != 0UL) {
			unsigned पूर्णाङ्क bit = TRAILING_ZEROS(bitmask);
			unsigned पूर्णाङ्क index = i * NBITS + bit;
			संरचना k_thread *thread;

			SYS_DLIST_FOR_EACH_CONTAINER(&pq->queues[index],
						     thread, base.qnode_dlist) {
				सचे ((thread->base.cpu_mask &
				     current_cpu_bit) != 0) {
					पटिनिवत्त thread;
				}
			}
			bitmask &= (bitmask - 1UL);
		}
	}
	पटिनिवत्त रिक्त;
}
#endif /* CONFIG_SCHED_CPU_MASK */

#endif /* ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_ */
