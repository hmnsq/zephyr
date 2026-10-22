/*
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_
#define ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_

#অন্তর্ভাব <zephyr/sys/math_extras.h>
#অন্তর্ভাব <zephyr/sys/dlist.h>

/* Simple Scheduling */
#যদি defined(CONFIG_SCHED_SIMPLE)
#define _priq_run_init		z_priq_simple_init
#define _priq_run_add		z_priq_simple_add
#define _priq_run_remove	z_priq_simple_remove
#define _priq_run_yield         z_priq_simple_yield
# যদি defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_simple_mask_best
# অন্যথা
#  define _priq_run_best	z_priq_simple_best
# endif /* CONFIG_SCHED_CPU_MASK */
/* Scalable Scheduling */
#নোচেদ্যদি defined(CONFIG_SCHED_SCALABLE)
#define _priq_run_init		z_priq_rb_init
#define _priq_run_add		z_priq_rb_add
#define _priq_run_remove	z_priq_rb_remove
#define _priq_run_yield         z_priq_rb_yield
# যদি defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_rb_mask_best
# অন্যথা
#  define _priq_run_best	z_priq_rb_best
# endif /* CONFIG_SCHED_CPU_MASK */
 /* Multi Queue Scheduling */
#নোচেদ্যদি defined(CONFIG_SCHED_MULTIQ)
#define _priq_run_init		z_priq_mq_init
#define _priq_run_add		z_priq_mq_add
#define _priq_run_remove	z_priq_mq_remove
#define _priq_run_yield         z_priq_mq_yield
# যদি defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_mq_mask_best
# অন্যথা
#  define _priq_run_best	z_priq_mq_best
# endif /* CONFIG_SCHED_CPU_MASK */
#endif

/* Scalable Wait Queue */
#যদি defined(CONFIG_WAITQ_SCALABLE)
#define _priq_wait_add		z_priq_rb_add
#define _priq_wait_remove	z_priq_rb_remove
#define _priq_wait_best		z_priq_rb_best
/* Simple Wait Queue */
#নোচেদ্যদি defined(CONFIG_WAITQ_SIMPLE)
#define _priq_wait_add		z_priq_simple_add
#define _priq_wait_remove	z_priq_simple_remove
#define _priq_wait_best		z_priq_simple_best
#endif

#যদি defined(CONFIG_64BIT)
#define NBITS          64
#define TRAILING_ZEROS u64_গুনতি_trailing_zeros
#অন্যথা
#define NBITS          32
#define TRAILING_ZEROS u32_গুনতি_trailing_zeros
#endif /* CONFIG_64BIT */

স্থির ALWAYS_INLINE শূন্য z_priq_simple_init(sys_dlist_t *pq)
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
স্থির ALWAYS_INLINE int32_t z_sched_prio_cmp(সংরচনা k_thread *thread_1, সংরচনা k_thread *thread_2)
{
	/* `prio` is <32b, so the below cannot overflow. */
	int32_t b1 = thread_1->base.prio;
	int32_t b2 = thread_2->base.prio;

	যদি (b1 != b2) {
		প্রত্যাগম b2 - b1;
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

	যদি (d1 != d2) {
		/* Sooner deadline means higher effective priority.
		 * Doing the calculation with unsigned types and casting
		 * to signed isn't perfect, but at least reduces this
		 * from UB on overflow to impdef.
		 */
		প্রত্যাগম (int32_t)(d2 - d1);
	}
#endif /* CONFIG_SCHED_DEADLINE */
	প্রত্যাগম 0;
}

স্থির ALWAYS_INLINE শূন্য z_priq_simple_add(sys_dlist_t *pq, সংরচনা k_thread *thread)
{
	সংরচনা k_thread *t;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, t, base.qnode_dlist) {
		যদি (z_sched_prio_cmp(thread, t) > 0) {
			sys_dlist_ঢোকাও(&t->base.qnode_dlist, &thread->base.qnode_dlist);
			প্রত্যাগম;
		}
	}

	sys_dlist_append(pq, &thread->base.qnode_dlist);
}

স্থির ALWAYS_INLINE শূন্য z_priq_simple_remove(sys_dlist_t *pq, সংরচনা k_thread *thread)
{
	ARG_UNUSED(pq);

	sys_dlist_remove(&thread->base.qnode_dlist);
}

স্থির ALWAYS_INLINE শূন্য z_priq_simple_yield(sys_dlist_t *pq)
{
#ifndef CONFIG_SMP
	sys_dnode_t *n;

	n = sys_dlist_peek_next_no_check(pq, &_current->base.qnode_dlist);

	sys_dlist_dequeue(&_current->base.qnode_dlist);

	সংরচনা k_thread *t;

	/*
	 * As it is possible that the current thread was not at the head of
	 * the run queue, start searching from the present position for where
	 * to re-insert it.
	 */

	যাবত্ (n != রিক্ত) {
		t = CONTAINER_OF(n, সংরচনা k_thread, base.qnode_dlist);
		যদি (z_sched_prio_cmp(_current, t) > 0) {
			sys_dlist_ঢোকাও(&t->base.qnode_dlist,
					 &_current->base.qnode_dlist);
			প্রত্যাগম;
		}
		n = sys_dlist_peek_next_no_check(pq, n);
	}

	sys_dlist_append(pq, &_current->base.qnode_dlist);
#endif
}

স্থির ALWAYS_INLINE সংরচনা k_thread *z_priq_simple_best(sys_dlist_t *pq)
{
	সংরচনা k_thread *thread = রিক্ত;
	sys_dnode_t *n = sys_dlist_peek_head(pq);

	যদি (n != রিক্ত) {
		thread = CONTAINER_OF(n, সংরচনা k_thread, base.qnode_dlist);
	}
	প্রত্যাগম thread;
}

#ifdef CONFIG_SCHED_CPU_MASK
স্থির ALWAYS_INLINE সংরচনা k_thread *z_priq_simple_mask_best(sys_dlist_t *pq)
{
	/* With masks enabled we need to be prepared to walk the list
	 * looking for one we can run
	 */
	unsigned long current_cpu_bit = BIT(_current_cpu->id);
	সংরচনা k_thread *thread;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, thread, base.qnode_dlist) {
		যদি ((thread->base.cpu_mask & current_cpu_bit) != 0) {
			প্রত্যাগম thread;
		}
	}
	প্রত্যাগম রিক্ত;
}
#endif /* CONFIG_SCHED_CPU_MASK */

#যদি defined(CONFIG_SCHED_SCALABLE) || defined(CONFIG_WAITQ_SCALABLE)
স্থির ALWAYS_INLINE শূন্য z_priq_rb_init(সংরচনা _priq_rb *pq)
{
	bool z_priq_rb_lessthan(সংরচনা rbnode *a, সংরচনা rbnode *b);

	*pq = (সংরচনা _priq_rb) {
		.বৃক্ষ = {
			.lessthan_fn = z_priq_rb_lessthan,
		}
	};
}

স্থির ALWAYS_INLINE শূন্য z_priq_rb_add(সংরচনা _priq_rb *pq, সংরচনা k_thread *thread)
{
	সংরচনা k_thread *t;

	thread->base.order_চাবি = pq->next_order_চাবি;
	++pq->next_order_চাবি;

	/* Renumber at wraparound.  This is tiny code, and in practice
	 * will almost never be hit on real systems.  BUT on very
	 * long-running systems where a priq never completely empties
	 * AND that contains very large numbers of threads, it can be
	 * a latency glitch to loop over all the threads like this.
	 */
	যদি (!pq->next_order_চাবি) {
		RB_FOR_EACH_CONTAINER(&pq->বৃক্ষ, t, base.qnode_rb) {
			t->base.order_চাবি = pq->next_order_চাবি;
			++pq->next_order_চাবি;
		}
	}

	rb_ঢোকাও(&pq->বৃক্ষ, &thread->base.qnode_rb);
}

স্থির ALWAYS_INLINE শূন্য z_priq_rb_remove(সংরচনা _priq_rb *pq, সংরচনা k_thread *thread)
{
	rb_remove(&pq->বৃক্ষ, &thread->base.qnode_rb);

	যদি (!pq->বৃক্ষ.মূল) {
		pq->next_order_চাবি = 0;
	}
}

স্থির ALWAYS_INLINE শূন্য z_priq_rb_yield(সংরচনা _priq_rb *pq)
{
#ifndef CONFIG_SMP
	z_priq_rb_remove(pq, _current);
	z_priq_rb_add(pq, _current);
#endif
}

স্থির ALWAYS_INLINE সংরচনা k_thread *z_priq_rb_best(সংরচনা _priq_rb *pq)
{
	সংরচনা k_thread *thread = রিক্ত;
	সংরচনা rbnode *n = rb_get_min(&pq->বৃক্ষ);

	যদি (n != রিক্ত) {
		thread = CONTAINER_OF(n, সংরচনা k_thread, base.qnode_rb);
	}
	প্রত্যাগম thread;
}

#ifdef CONFIG_SCHED_CPU_MASK
স্থির ALWAYS_INLINE সংরচনা k_thread *z_priq_rb_mask_best(সংরচনা _priq_rb *pq)
{
	unsigned long current_cpu_bit = BIT(_current_cpu->id);
	সংরচনা k_thread *thread;

	RB_FOR_EACH_CONTAINER(&pq->বৃক্ষ, thread, base.qnode_rb) {
		যদি ((thread->base.cpu_mask & current_cpu_bit) != 0) {
			প্রত্যাগম thread;
		}
	}
	প্রত্যাগম রিক্ত;
}
#endif /* CONFIG_SCHED_CPU_MASK */
#endif /* CONFIG_SCHED_SCALABLE || CONFIG_WAITQ_SCALABLE */

সংরচনা prio_info {
	uint8_t offset_prio;
	uint8_t idx;
	uint8_t bit;
};

স্থির ALWAYS_INLINE সংরচনা prio_info get_prio_info(int8_t old_prio)
{
	সংরচনা prio_info ret;

	ret.offset_prio = old_prio - K_HIGHEST_THREAD_PRIO;
	ret.idx = ret.offset_prio / NBITS;
	ret.bit = ret.offset_prio % NBITS;

	প্রত্যাগম ret;
}

স্থির ALWAYS_INLINE unsigned পূর্ণাঙ্ক z_priq_mq_best_queue_index(সংরচনা _priq_mq *pq)
{
	unsigned পূর্ণাঙ্ক i = 0;

	কুরু {
		যদি (likely(pq->bitmask[i])) {
			প্রত্যাগম i * NBITS + TRAILING_ZEROS(pq->bitmask[i]);
		}
		i++;
	} যাবত্ (i < PRIQ_BITMAP_SIZE);

	প্রত্যাগম K_অঙ্ক_THREAD_PRIO - 1;
}

স্থির ALWAYS_INLINE শূন্য z_priq_mq_init(সংরচনা _priq_mq *q)
{
	প্রতি (size_t i = 0; i < ARRAY_SIZE(q->queues); i++) {
		sys_dlist_init(&q->queues[i]);
	}

#ifndef CONFIG_SMP
	q->cached_queue_index = K_অঙ্ক_THREAD_PRIO - 1;
#endif
}

স্থির ALWAYS_INLINE শূন্য z_priq_mq_add(সংরচনা _priq_mq *pq,
					সংরচনা k_thread *thread)
{
	সংরচনা prio_info pos = get_prio_info(thread->base.prio);

	sys_dlist_append(&pq->queues[pos.offset_prio], &thread->base.qnode_dlist);
	pq->bitmask[pos.idx] |= BIT(pos.bit);

#ifndef CONFIG_SMP
	যদি (pos.offset_prio < pq->cached_queue_index) {
		pq->cached_queue_index = pos.offset_prio;
	}
#endif
}

স্থির ALWAYS_INLINE শূন্য z_priq_mq_remove(সংরচনা _priq_mq *pq,
					   সংরচনা k_thread *thread)
{
	সংরচনা prio_info pos = get_prio_info(thread->base.prio);

	sys_dlist_dequeue(&thread->base.qnode_dlist);
	যদি (unlikely(sys_dlist_is_empty(&pq->queues[pos.offset_prio]))) {
		pq->bitmask[pos.idx] &= ~BIT(pos.bit);
#ifndef CONFIG_SMP
		pq->cached_queue_index = z_priq_mq_best_queue_index(pq);
#endif
	}
}

স্থির ALWAYS_INLINE শূন্য z_priq_mq_yield(সংরচনা _priq_mq *pq)
{
#ifndef CONFIG_SMP
	সংরচনা prio_info pos = get_prio_info(_current->base.prio);

	sys_dlist_dequeue(&_current->base.qnode_dlist);
	sys_dlist_append(&pq->queues[pos.offset_prio],
			 &_current->base.qnode_dlist);
#endif
}

স্থির ALWAYS_INLINE সংরচনা k_thread *z_priq_mq_best(সংরচনা _priq_mq *pq)
{
#ifdef CONFIG_SMP
	unsigned পূর্ণাঙ্ক index = z_priq_mq_best_queue_index(pq);
#অন্যথা
	unsigned পূর্ণাঙ্ক index = pq->cached_queue_index;
#endif

	sys_dnode_t *n = sys_dlist_peek_head(&pq->queues[index]);

	যদি (likely(n != রিক্ত)) {
		প্রত্যাগম CONTAINER_OF(n, সংরচনা k_thread, base.qnode_dlist);
	}

	প্রত্যাগম রিক্ত;
}

#ifdef CONFIG_SCHED_CPU_MASK
স্থির ALWAYS_INLINE সংরচনা k_thread *z_priq_mq_mask_best(সংরচনা _priq_mq *pq)
{
	unsigned long current_cpu_bit = BIT(_current_cpu->id);

	/* Walk priority levels from highest to lowest, and within each
	 * level walk the thread list to find the first thread runnable
	 * on the current CPU.
	 */
	প্রতি (unsigned পূর্ণাঙ্ক i = 0U; i < PRIQ_BITMAP_SIZE; i++) {
		unsigned long bitmask = pq->bitmask[i];

		যাবত্ (bitmask != 0UL) {
			unsigned পূর্ণাঙ্ক bit = TRAILING_ZEROS(bitmask);
			unsigned পূর্ণাঙ্ক index = i * NBITS + bit;
			সংরচনা k_thread *thread;

			SYS_DLIST_FOR_EACH_CONTAINER(&pq->queues[index],
						     thread, base.qnode_dlist) {
				যদি ((thread->base.cpu_mask &
				     current_cpu_bit) != 0) {
					প্রত্যাগম thread;
				}
			}
			bitmask &= (bitmask - 1UL);
		}
	}
	প্রত্যাগম রিক্ত;
}
#endif /* CONFIG_SCHED_CPU_MASK */

#endif /* ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_ */
