/*
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_
#define ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_

#ಅನ್ತರ್ಭಾವ <zephyr/sys/math_extras.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/dlist.h>

/* Simple Scheduling */
#ಯದಿ defined(CONFIG_SCHED_SIMPLE)
#define _priq_run_init		z_priq_simple_init
#define _priq_run_add		z_priq_simple_add
#define _priq_run_remove	z_priq_simple_remove
#define _priq_run_yield         z_priq_simple_yield
# ಯದಿ defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_simple_mask_best
# ಅನ್ಯಥಾ
#  define _priq_run_best	z_priq_simple_best
# endif /* CONFIG_SCHED_CPU_MASK */
/* Scalable Scheduling */
#ನೋಚೇದ್ಯದಿ defined(CONFIG_SCHED_SCALABLE)
#define _priq_run_init		z_priq_rb_init
#define _priq_run_add		z_priq_rb_add
#define _priq_run_remove	z_priq_rb_remove
#define _priq_run_yield         z_priq_rb_yield
# ಯದಿ defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_rb_mask_best
# ಅನ್ಯಥಾ
#  define _priq_run_best	z_priq_rb_best
# endif /* CONFIG_SCHED_CPU_MASK */
 /* Multi Queue Scheduling */
#ನೋಚೇದ್ಯದಿ defined(CONFIG_SCHED_MULTIQ)
#define _priq_run_init		z_priq_mq_init
#define _priq_run_add		z_priq_mq_add
#define _priq_run_remove	z_priq_mq_remove
#define _priq_run_yield         z_priq_mq_yield
# ಯದಿ defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_mq_mask_best
# ಅನ್ಯಥಾ
#  define _priq_run_best	z_priq_mq_best
# endif /* CONFIG_SCHED_CPU_MASK */
#endif

/* Scalable Wait Queue */
#ಯದಿ defined(CONFIG_WAITQ_SCALABLE)
#define _priq_wait_add		z_priq_rb_add
#define _priq_wait_remove	z_priq_rb_remove
#define _priq_wait_best		z_priq_rb_best
/* Simple Wait Queue */
#ನೋಚೇದ್ಯದಿ defined(CONFIG_WAITQ_SIMPLE)
#define _priq_wait_add		z_priq_simple_add
#define _priq_wait_remove	z_priq_simple_remove
#define _priq_wait_best		z_priq_simple_best
#endif

#ಯದಿ defined(CONFIG_64BIT)
#define NBITS          64
#define TRAILING_ZEROS u64_ಎಣಿಕೆ_trailing_zeros
#ಅನ್ಯಥಾ
#define NBITS          32
#define TRAILING_ZEROS u32_ಎಣಿಕೆ_trailing_zeros
#endif /* CONFIG_64BIT */

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ z_priq_simple_init(sys_dlist_t *pq)
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
ಸ್ಥಿರ ALWAYS_INLINE int32_t z_sched_prio_cmp(ಸಂರಚನಾ k_thread *thread_1, ಸಂರಚನಾ k_thread *thread_2)
{
	/* `prio` is <32b, so the below cannot overflow. */
	int32_t b1 = thread_1->base.prio;
	int32_t b2 = thread_2->base.prio;

	ಯದಿ (b1 != b2) {
		ಪ್ರತ್ಯಾಗಮ b2 - b1;
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

	ಯದಿ (d1 != d2) {
		/* Sooner deadline means higher effective priority.
		 * Doing the calculation with unsigned types and casting
		 * to signed isn't perfect, but at least reduces this
		 * from UB on overflow to impdef.
		 */
		ಪ್ರತ್ಯಾಗಮ (int32_t)(d2 - d1);
	}
#endif /* CONFIG_SCHED_DEADLINE */
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ z_priq_simple_add(sys_dlist_t *pq, ಸಂರಚನಾ k_thread *thread)
{
	ಸಂರಚನಾ k_thread *t;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, t, base.qnode_dlist) {
		ಯದಿ (z_sched_prio_cmp(thread, t) > 0) {
			sys_dlist_ಸೇರಿಸು(&t->base.qnode_dlist, &thread->base.qnode_dlist);
			ಪ್ರತ್ಯಾಗಮ;
		}
	}

	sys_dlist_append(pq, &thread->base.qnode_dlist);
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ z_priq_simple_remove(sys_dlist_t *pq, ಸಂರಚನಾ k_thread *thread)
{
	ARG_UNUSED(pq);

	sys_dlist_remove(&thread->base.qnode_dlist);
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ z_priq_simple_yield(sys_dlist_t *pq)
{
#ifndef CONFIG_SMP
	sys_dnode_t *n;

	n = sys_dlist_peek_next_no_check(pq, &_current->base.qnode_dlist);

	sys_dlist_dequeue(&_current->base.qnode_dlist);

	ಸಂರಚನಾ k_thread *t;

	/*
	 * As it is possible that the current thread was not at the head of
	 * the run queue, start searching from the present position for where
	 * to re-insert it.
	 */

	ಯಾವತ್ (n != ರಿಕ್ತ) {
		t = CONTAINER_OF(n, ಸಂರಚನಾ k_thread, base.qnode_dlist);
		ಯದಿ (z_sched_prio_cmp(_current, t) > 0) {
			sys_dlist_ಸೇರಿಸು(&t->base.qnode_dlist,
					 &_current->base.qnode_dlist);
			ಪ್ರತ್ಯಾಗಮ;
		}
		n = sys_dlist_peek_next_no_check(pq, n);
	}

	sys_dlist_append(pq, &_current->base.qnode_dlist);
#endif
}

ಸ್ಥಿರ ALWAYS_INLINE ಸಂರಚನಾ k_thread *z_priq_simple_best(sys_dlist_t *pq)
{
	ಸಂರಚನಾ k_thread *thread = ರಿಕ್ತ;
	sys_dnode_t *n = sys_dlist_peek_head(pq);

	ಯದಿ (n != ರಿಕ್ತ) {
		thread = CONTAINER_OF(n, ಸಂರಚನಾ k_thread, base.qnode_dlist);
	}
	ಪ್ರತ್ಯಾಗಮ thread;
}

#ifdef CONFIG_SCHED_CPU_MASK
ಸ್ಥಿರ ALWAYS_INLINE ಸಂರಚನಾ k_thread *z_priq_simple_mask_best(sys_dlist_t *pq)
{
	/* With masks enabled we need to be prepared to walk the list
	 * looking for one we can run
	 */
	unsigned long current_cpu_bit = BIT(_current_cpu->id);
	ಸಂರಚನಾ k_thread *thread;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, thread, base.qnode_dlist) {
		ಯದಿ ((thread->base.cpu_mask & current_cpu_bit) != 0) {
			ಪ್ರತ್ಯಾಗಮ thread;
		}
	}
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}
#endif /* CONFIG_SCHED_CPU_MASK */

#ಯದಿ defined(CONFIG_SCHED_SCALABLE) || defined(CONFIG_WAITQ_SCALABLE)
ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ z_priq_rb_init(ಸಂರಚನಾ _priq_rb *pq)
{
	bool z_priq_rb_lessthan(ಸಂರಚನಾ rbnode *a, ಸಂರಚನಾ rbnode *b);

	*pq = (ಸಂರಚನಾ _priq_rb) {
		.ವೃಕ್ಷ = {
			.lessthan_fn = z_priq_rb_lessthan,
		}
	};
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ z_priq_rb_add(ಸಂರಚನಾ _priq_rb *pq, ಸಂರಚನಾ k_thread *thread)
{
	ಸಂರಚನಾ k_thread *t;

	thread->base.order_ಕೀಲಿ = pq->next_order_ಕೀಲಿ;
	++pq->next_order_ಕೀಲಿ;

	/* Renumber at wraparound.  This is tiny code, and in practice
	 * will almost never be hit on real systems.  BUT on very
	 * long-running systems where a priq never completely empties
	 * AND that contains very large numbers of threads, it can be
	 * a latency glitch to loop over all the threads like this.
	 */
	ಯದಿ (!pq->next_order_ಕೀಲಿ) {
		RB_FOR_EACH_CONTAINER(&pq->ವೃಕ್ಷ, t, base.qnode_rb) {
			t->base.order_ಕೀಲಿ = pq->next_order_ಕೀಲಿ;
			++pq->next_order_ಕೀಲಿ;
		}
	}

	rb_ಸೇರಿಸು(&pq->ವೃಕ್ಷ, &thread->base.qnode_rb);
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ z_priq_rb_remove(ಸಂರಚನಾ _priq_rb *pq, ಸಂರಚನಾ k_thread *thread)
{
	rb_remove(&pq->ವೃಕ್ಷ, &thread->base.qnode_rb);

	ಯದಿ (!pq->ವೃಕ್ಷ.ಬೇರು) {
		pq->next_order_ಕೀಲಿ = 0;
	}
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ z_priq_rb_yield(ಸಂರಚನಾ _priq_rb *pq)
{
#ifndef CONFIG_SMP
	z_priq_rb_remove(pq, _current);
	z_priq_rb_add(pq, _current);
#endif
}

ಸ್ಥಿರ ALWAYS_INLINE ಸಂರಚನಾ k_thread *z_priq_rb_best(ಸಂರಚನಾ _priq_rb *pq)
{
	ಸಂರಚನಾ k_thread *thread = ರಿಕ್ತ;
	ಸಂರಚನಾ rbnode *n = rb_get_min(&pq->ವೃಕ್ಷ);

	ಯದಿ (n != ರಿಕ್ತ) {
		thread = CONTAINER_OF(n, ಸಂರಚನಾ k_thread, base.qnode_rb);
	}
	ಪ್ರತ್ಯಾಗಮ thread;
}

#ifdef CONFIG_SCHED_CPU_MASK
ಸ್ಥಿರ ALWAYS_INLINE ಸಂರಚನಾ k_thread *z_priq_rb_mask_best(ಸಂರಚನಾ _priq_rb *pq)
{
	unsigned long current_cpu_bit = BIT(_current_cpu->id);
	ಸಂರಚನಾ k_thread *thread;

	RB_FOR_EACH_CONTAINER(&pq->ವೃಕ್ಷ, thread, base.qnode_rb) {
		ಯದಿ ((thread->base.cpu_mask & current_cpu_bit) != 0) {
			ಪ್ರತ್ಯಾಗಮ thread;
		}
	}
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}
#endif /* CONFIG_SCHED_CPU_MASK */
#endif /* CONFIG_SCHED_SCALABLE || CONFIG_WAITQ_SCALABLE */

ಸಂರಚನಾ prio_info {
	uint8_t offset_prio;
	uint8_t idx;
	uint8_t bit;
};

ಸ್ಥಿರ ALWAYS_INLINE ಸಂರಚನಾ prio_info get_prio_info(int8_t old_prio)
{
	ಸಂರಚನಾ prio_info ret;

	ret.offset_prio = old_prio - K_HIGHEST_THREAD_PRIO;
	ret.idx = ret.offset_prio / NBITS;
	ret.bit = ret.offset_prio % NBITS;

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ALWAYS_INLINE unsigned ಪೂರ್ಣಾಙ್ಕ z_priq_mq_best_queue_index(ಸಂರಚನಾ _priq_mq *pq)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ i = 0;

	ಕುರು {
		ಯದಿ (likely(pq->bitmask[i])) {
			ಪ್ರತ್ಯಾಗಮ i * NBITS + TRAILING_ZEROS(pq->bitmask[i]);
		}
		i++;
	} ಯಾವತ್ (i < PRIQ_BITMAP_SIZE);

	ಪ್ರತ್ಯಾಗಮ K_ಅಂಕೆ_THREAD_PRIO - 1;
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ z_priq_mq_init(ಸಂರಚನಾ _priq_mq *q)
{
	ಪ್ರತಿ (size_t i = 0; i < ARRAY_SIZE(q->queues); i++) {
		sys_dlist_init(&q->queues[i]);
	}

#ifndef CONFIG_SMP
	q->cached_queue_index = K_ಅಂಕೆ_THREAD_PRIO - 1;
#endif
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ z_priq_mq_add(ಸಂರಚನಾ _priq_mq *pq,
					ಸಂರಚನಾ k_thread *thread)
{
	ಸಂರಚನಾ prio_info pos = get_prio_info(thread->base.prio);

	sys_dlist_append(&pq->queues[pos.offset_prio], &thread->base.qnode_dlist);
	pq->bitmask[pos.idx] |= BIT(pos.bit);

#ifndef CONFIG_SMP
	ಯದಿ (pos.offset_prio < pq->cached_queue_index) {
		pq->cached_queue_index = pos.offset_prio;
	}
#endif
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ z_priq_mq_remove(ಸಂರಚನಾ _priq_mq *pq,
					   ಸಂರಚನಾ k_thread *thread)
{
	ಸಂರಚನಾ prio_info pos = get_prio_info(thread->base.prio);

	sys_dlist_dequeue(&thread->base.qnode_dlist);
	ಯದಿ (unlikely(sys_dlist_is_empty(&pq->queues[pos.offset_prio]))) {
		pq->bitmask[pos.idx] &= ~BIT(pos.bit);
#ifndef CONFIG_SMP
		pq->cached_queue_index = z_priq_mq_best_queue_index(pq);
#endif
	}
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ z_priq_mq_yield(ಸಂರಚನಾ _priq_mq *pq)
{
#ifndef CONFIG_SMP
	ಸಂರಚನಾ prio_info pos = get_prio_info(_current->base.prio);

	sys_dlist_dequeue(&_current->base.qnode_dlist);
	sys_dlist_append(&pq->queues[pos.offset_prio],
			 &_current->base.qnode_dlist);
#endif
}

ಸ್ಥಿರ ALWAYS_INLINE ಸಂರಚನಾ k_thread *z_priq_mq_best(ಸಂರಚನಾ _priq_mq *pq)
{
#ifdef CONFIG_SMP
	unsigned ಪೂರ್ಣಾಙ್ಕ index = z_priq_mq_best_queue_index(pq);
#ಅನ್ಯಥಾ
	unsigned ಪೂರ್ಣಾಙ್ಕ index = pq->cached_queue_index;
#endif

	sys_dnode_t *n = sys_dlist_peek_head(&pq->queues[index]);

	ಯದಿ (likely(n != ರಿಕ್ತ)) {
		ಪ್ರತ್ಯಾಗಮ CONTAINER_OF(n, ಸಂರಚನಾ k_thread, base.qnode_dlist);
	}

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

#ifdef CONFIG_SCHED_CPU_MASK
ಸ್ಥಿರ ALWAYS_INLINE ಸಂರಚನಾ k_thread *z_priq_mq_mask_best(ಸಂರಚನಾ _priq_mq *pq)
{
	unsigned long current_cpu_bit = BIT(_current_cpu->id);

	/* Walk priority levels from highest to lowest, and within each
	 * level walk the thread list to find the first thread runnable
	 * on the current CPU.
	 */
	ಪ್ರತಿ (unsigned ಪೂರ್ಣಾಙ್ಕ i = 0U; i < PRIQ_BITMAP_SIZE; i++) {
		unsigned long bitmask = pq->bitmask[i];

		ಯಾವತ್ (bitmask != 0UL) {
			unsigned ಪೂರ್ಣಾಙ್ಕ bit = TRAILING_ZEROS(bitmask);
			unsigned ಪೂರ್ಣಾಙ್ಕ index = i * NBITS + bit;
			ಸಂರಚನಾ k_thread *thread;

			SYS_DLIST_FOR_EACH_CONTAINER(&pq->queues[index],
						     thread, base.qnode_dlist) {
				ಯದಿ ((thread->base.cpu_mask &
				     current_cpu_bit) != 0) {
					ಪ್ರತ್ಯಾಗಮ thread;
				}
			}
			bitmask &= (bitmask - 1UL);
		}
	}
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}
#endif /* CONFIG_SCHED_CPU_MASK */

#endif /* ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_ */
