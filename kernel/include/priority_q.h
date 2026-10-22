/*
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_
#define ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_

#അന്തര്ഭാവ <zephyr/sys/math_extras.h>
#അന്തര്ഭാവ <zephyr/sys/dlist.h>

/* Simple Scheduling */
#എങ്കിൽ defined(CONFIG_SCHED_SIMPLE)
#define _priq_run_init		z_priq_simple_init
#define _priq_run_add		z_priq_simple_add
#define _priq_run_remove	z_priq_simple_remove
#define _priq_run_yield         z_priq_simple_yield
# എങ്കിൽ defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_simple_mask_best
# അല്ലെങ്കിൽ
#  define _priq_run_best	z_priq_simple_best
# endif /* CONFIG_SCHED_CPU_MASK */
/* Scalable Scheduling */
#അല്ലെങ്കിലെങ്കിൽ defined(CONFIG_SCHED_SCALABLE)
#define _priq_run_init		z_priq_rb_init
#define _priq_run_add		z_priq_rb_add
#define _priq_run_remove	z_priq_rb_remove
#define _priq_run_yield         z_priq_rb_yield
# എങ്കിൽ defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_rb_mask_best
# അല്ലെങ്കിൽ
#  define _priq_run_best	z_priq_rb_best
# endif /* CONFIG_SCHED_CPU_MASK */
 /* Multi Queue Scheduling */
#അല്ലെങ്കിലെങ്കിൽ defined(CONFIG_SCHED_MULTIQ)
#define _priq_run_init		z_priq_mq_init
#define _priq_run_add		z_priq_mq_add
#define _priq_run_remove	z_priq_mq_remove
#define _priq_run_yield         z_priq_mq_yield
# എങ്കിൽ defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_mq_mask_best
# അല്ലെങ്കിൽ
#  define _priq_run_best	z_priq_mq_best
# endif /* CONFIG_SCHED_CPU_MASK */
#endif

/* Scalable Wait Queue */
#എങ്കിൽ defined(CONFIG_WAITQ_SCALABLE)
#define _priq_wait_add		z_priq_rb_add
#define _priq_wait_remove	z_priq_rb_remove
#define _priq_wait_best		z_priq_rb_best
/* Simple Wait Queue */
#അല്ലെങ്കിലെങ്കിൽ defined(CONFIG_WAITQ_SIMPLE)
#define _priq_wait_add		z_priq_simple_add
#define _priq_wait_remove	z_priq_simple_remove
#define _priq_wait_best		z_priq_simple_best
#endif

#എങ്കിൽ defined(CONFIG_64BIT)
#define NBITS          64
#define TRAILING_ZEROS u64_എണ്ണം_trailing_zeros
#അല്ലെങ്കിൽ
#define NBITS          32
#define TRAILING_ZEROS u32_എണ്ണം_trailing_zeros
#endif /* CONFIG_64BIT */

സ്ഥിതം ALWAYS_INLINE ശൂന്യം z_priq_simple_init(sys_dlist_t *pq)
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
സ്ഥിതം ALWAYS_INLINE int32_t z_sched_prio_cmp(ഘടന k_thread *thread_1, ഘടന k_thread *thread_2)
{
	/* `prio` is <32b, so the below cannot overflow. */
	int32_t b1 = thread_1->base.prio;
	int32_t b2 = thread_2->base.prio;

	എങ്കിൽ (b1 != b2) {
		തിരികെ b2 - b1;
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

	എങ്കിൽ (d1 != d2) {
		/* Sooner deadline means higher effective priority.
		 * Doing the calculation with unsigned types and casting
		 * to signed isn't perfect, but at least reduces this
		 * from UB on overflow to impdef.
		 */
		തിരികെ (int32_t)(d2 - d1);
	}
#endif /* CONFIG_SCHED_DEADLINE */
	തിരികെ 0;
}

സ്ഥിതം ALWAYS_INLINE ശൂന്യം z_priq_simple_add(sys_dlist_t *pq, ഘടന k_thread *thread)
{
	ഘടന k_thread *t;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, t, base.qnode_dlist) {
		എങ്കിൽ (z_sched_prio_cmp(thread, t) > 0) {
			sys_dlist_തിരുകുക(&t->base.qnode_dlist, &thread->base.qnode_dlist);
			തിരികെ;
		}
	}

	sys_dlist_append(pq, &thread->base.qnode_dlist);
}

സ്ഥിതം ALWAYS_INLINE ശൂന്യം z_priq_simple_remove(sys_dlist_t *pq, ഘടന k_thread *thread)
{
	ARG_UNUSED(pq);

	sys_dlist_remove(&thread->base.qnode_dlist);
}

സ്ഥിതം ALWAYS_INLINE ശൂന്യം z_priq_simple_yield(sys_dlist_t *pq)
{
#ifndef CONFIG_SMP
	sys_dnode_t *n;

	n = sys_dlist_peek_next_no_check(pq, &_current->base.qnode_dlist);

	sys_dlist_dequeue(&_current->base.qnode_dlist);

	ഘടന k_thread *t;

	/*
	 * As it is possible that the current thread was not at the head of
	 * the run queue, start searching from the present position for where
	 * to re-insert it.
	 */

	ആകുമ്പോൾ (n != ശൂന്യ) {
		t = CONTAINER_OF(n, ഘടന k_thread, base.qnode_dlist);
		എങ്കിൽ (z_sched_prio_cmp(_current, t) > 0) {
			sys_dlist_തിരുകുക(&t->base.qnode_dlist,
					 &_current->base.qnode_dlist);
			തിരികെ;
		}
		n = sys_dlist_peek_next_no_check(pq, n);
	}

	sys_dlist_append(pq, &_current->base.qnode_dlist);
#endif
}

സ്ഥിതം ALWAYS_INLINE ഘടന k_thread *z_priq_simple_best(sys_dlist_t *pq)
{
	ഘടന k_thread *thread = ശൂന്യ;
	sys_dnode_t *n = sys_dlist_peek_head(pq);

	എങ്കിൽ (n != ശൂന്യ) {
		thread = CONTAINER_OF(n, ഘടന k_thread, base.qnode_dlist);
	}
	തിരികെ thread;
}

#ifdef CONFIG_SCHED_CPU_MASK
സ്ഥിതം ALWAYS_INLINE ഘടന k_thread *z_priq_simple_mask_best(sys_dlist_t *pq)
{
	/* With masks enabled we need to be prepared to walk the list
	 * looking for one we can run
	 */
	unsigned long current_cpu_bit = BIT(_current_cpu->id);
	ഘടന k_thread *thread;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, thread, base.qnode_dlist) {
		എങ്കിൽ ((thread->base.cpu_mask & current_cpu_bit) != 0) {
			തിരികെ thread;
		}
	}
	തിരികെ ശൂന്യ;
}
#endif /* CONFIG_SCHED_CPU_MASK */

#എങ്കിൽ defined(CONFIG_SCHED_SCALABLE) || defined(CONFIG_WAITQ_SCALABLE)
സ്ഥിതം ALWAYS_INLINE ശൂന്യം z_priq_rb_init(ഘടന _priq_rb *pq)
{
	bool z_priq_rb_lessthan(ഘടന rbnode *a, ഘടന rbnode *b);

	*pq = (ഘടന _priq_rb) {
		.വൃക്ഷം = {
			.lessthan_fn = z_priq_rb_lessthan,
		}
	};
}

സ്ഥിതം ALWAYS_INLINE ശൂന്യം z_priq_rb_add(ഘടന _priq_rb *pq, ഘടന k_thread *thread)
{
	ഘടന k_thread *t;

	thread->base.order_താക്കോൽ = pq->next_order_താക്കോൽ;
	++pq->next_order_താക്കോൽ;

	/* Renumber at wraparound.  This is tiny code, and in practice
	 * will almost never be hit on real systems.  BUT on very
	 * long-running systems where a priq never completely empties
	 * AND that contains very large numbers of threads, it can be
	 * a latency glitch to loop over all the threads like this.
	 */
	എങ്കിൽ (!pq->next_order_താക്കോൽ) {
		RB_FOR_EACH_CONTAINER(&pq->വൃക്ഷം, t, base.qnode_rb) {
			t->base.order_താക്കോൽ = pq->next_order_താക്കോൽ;
			++pq->next_order_താക്കോൽ;
		}
	}

	rb_തിരുകുക(&pq->വൃക്ഷം, &thread->base.qnode_rb);
}

സ്ഥിതം ALWAYS_INLINE ശൂന്യം z_priq_rb_remove(ഘടന _priq_rb *pq, ഘടന k_thread *thread)
{
	rb_remove(&pq->വൃക്ഷം, &thread->base.qnode_rb);

	എങ്കിൽ (!pq->വൃക്ഷം.വേര്) {
		pq->next_order_താക്കോൽ = 0;
	}
}

സ്ഥിതം ALWAYS_INLINE ശൂന്യം z_priq_rb_yield(ഘടന _priq_rb *pq)
{
#ifndef CONFIG_SMP
	z_priq_rb_remove(pq, _current);
	z_priq_rb_add(pq, _current);
#endif
}

സ്ഥിതം ALWAYS_INLINE ഘടന k_thread *z_priq_rb_best(ഘടന _priq_rb *pq)
{
	ഘടന k_thread *thread = ശൂന്യ;
	ഘടന rbnode *n = rb_get_min(&pq->വൃക്ഷം);

	എങ്കിൽ (n != ശൂന്യ) {
		thread = CONTAINER_OF(n, ഘടന k_thread, base.qnode_rb);
	}
	തിരികെ thread;
}

#ifdef CONFIG_SCHED_CPU_MASK
സ്ഥിതം ALWAYS_INLINE ഘടന k_thread *z_priq_rb_mask_best(ഘടന _priq_rb *pq)
{
	unsigned long current_cpu_bit = BIT(_current_cpu->id);
	ഘടന k_thread *thread;

	RB_FOR_EACH_CONTAINER(&pq->വൃക്ഷം, thread, base.qnode_rb) {
		എങ്കിൽ ((thread->base.cpu_mask & current_cpu_bit) != 0) {
			തിരികെ thread;
		}
	}
	തിരികെ ശൂന്യ;
}
#endif /* CONFIG_SCHED_CPU_MASK */
#endif /* CONFIG_SCHED_SCALABLE || CONFIG_WAITQ_SCALABLE */

ഘടന prio_info {
	uint8_t offset_prio;
	uint8_t idx;
	uint8_t bit;
};

സ്ഥിതം ALWAYS_INLINE ഘടന prio_info get_prio_info(int8_t old_prio)
{
	ഘടന prio_info ret;

	ret.offset_prio = old_prio - K_HIGHEST_THREAD_PRIO;
	ret.idx = ret.offset_prio / NBITS;
	ret.bit = ret.offset_prio % NBITS;

	തിരികെ ret;
}

സ്ഥിതം ALWAYS_INLINE unsigned പൂർണ്ണസംഖ്യ z_priq_mq_best_queue_index(ഘടന _priq_mq *pq)
{
	unsigned പൂർണ്ണസംഖ്യ i = 0;

	ചെയ്യുക {
		എങ്കിൽ (likely(pq->bitmask[i])) {
			തിരികെ i * NBITS + TRAILING_ZEROS(pq->bitmask[i]);
		}
		i++;
	} ആകുമ്പോൾ (i < PRIQ_BITMAP_SIZE);

	തിരികെ K_അക്കം_THREAD_PRIO - 1;
}

സ്ഥിതം ALWAYS_INLINE ശൂന്യം z_priq_mq_init(ഘടന _priq_mq *q)
{
	വേണ്ടി (size_t i = 0; i < ARRAY_SIZE(q->queues); i++) {
		sys_dlist_init(&q->queues[i]);
	}

#ifndef CONFIG_SMP
	q->cached_queue_index = K_അക്കം_THREAD_PRIO - 1;
#endif
}

സ്ഥിതം ALWAYS_INLINE ശൂന്യം z_priq_mq_add(ഘടന _priq_mq *pq,
					ഘടന k_thread *thread)
{
	ഘടന prio_info pos = get_prio_info(thread->base.prio);

	sys_dlist_append(&pq->queues[pos.offset_prio], &thread->base.qnode_dlist);
	pq->bitmask[pos.idx] |= BIT(pos.bit);

#ifndef CONFIG_SMP
	എങ്കിൽ (pos.offset_prio < pq->cached_queue_index) {
		pq->cached_queue_index = pos.offset_prio;
	}
#endif
}

സ്ഥിതം ALWAYS_INLINE ശൂന്യം z_priq_mq_remove(ഘടന _priq_mq *pq,
					   ഘടന k_thread *thread)
{
	ഘടന prio_info pos = get_prio_info(thread->base.prio);

	sys_dlist_dequeue(&thread->base.qnode_dlist);
	എങ്കിൽ (unlikely(sys_dlist_is_empty(&pq->queues[pos.offset_prio]))) {
		pq->bitmask[pos.idx] &= ~BIT(pos.bit);
#ifndef CONFIG_SMP
		pq->cached_queue_index = z_priq_mq_best_queue_index(pq);
#endif
	}
}

സ്ഥിതം ALWAYS_INLINE ശൂന്യം z_priq_mq_yield(ഘടന _priq_mq *pq)
{
#ifndef CONFIG_SMP
	ഘടന prio_info pos = get_prio_info(_current->base.prio);

	sys_dlist_dequeue(&_current->base.qnode_dlist);
	sys_dlist_append(&pq->queues[pos.offset_prio],
			 &_current->base.qnode_dlist);
#endif
}

സ്ഥിതം ALWAYS_INLINE ഘടന k_thread *z_priq_mq_best(ഘടന _priq_mq *pq)
{
#ifdef CONFIG_SMP
	unsigned പൂർണ്ണസംഖ്യ index = z_priq_mq_best_queue_index(pq);
#അല്ലെങ്കിൽ
	unsigned പൂർണ്ണസംഖ്യ index = pq->cached_queue_index;
#endif

	sys_dnode_t *n = sys_dlist_peek_head(&pq->queues[index]);

	എങ്കിൽ (likely(n != ശൂന്യ)) {
		തിരികെ CONTAINER_OF(n, ഘടന k_thread, base.qnode_dlist);
	}

	തിരികെ ശൂന്യ;
}

#ifdef CONFIG_SCHED_CPU_MASK
സ്ഥിതം ALWAYS_INLINE ഘടന k_thread *z_priq_mq_mask_best(ഘടന _priq_mq *pq)
{
	unsigned long current_cpu_bit = BIT(_current_cpu->id);

	/* Walk priority levels from highest to lowest, and within each
	 * level walk the thread list to find the first thread runnable
	 * on the current CPU.
	 */
	വേണ്ടി (unsigned പൂർണ്ണസംഖ്യ i = 0U; i < PRIQ_BITMAP_SIZE; i++) {
		unsigned long bitmask = pq->bitmask[i];

		ആകുമ്പോൾ (bitmask != 0UL) {
			unsigned പൂർണ്ണസംഖ്യ bit = TRAILING_ZEROS(bitmask);
			unsigned പൂർണ്ണസംഖ്യ index = i * NBITS + bit;
			ഘടന k_thread *thread;

			SYS_DLIST_FOR_EACH_CONTAINER(&pq->queues[index],
						     thread, base.qnode_dlist) {
				എങ്കിൽ ((thread->base.cpu_mask &
				     current_cpu_bit) != 0) {
					തിരികെ thread;
				}
			}
			bitmask &= (bitmask - 1UL);
		}
	}
	തിരികെ ശൂന്യ;
}
#endif /* CONFIG_SCHED_CPU_MASK */

#endif /* ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_ */
