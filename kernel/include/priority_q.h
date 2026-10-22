/*
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_
#define ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_

#அந்தர்பாவ <zephyr/sys/math_extras.h>
#அந்தர்பாவ <zephyr/sys/dlist.h>

/* Simple Scheduling */
#எனில் defined(CONFIG_SCHED_SIMPLE)
#define _priq_run_init		z_priq_simple_init
#define _priq_run_add		z_priq_simple_add
#define _priq_run_remove	z_priq_simple_remove
#define _priq_run_yield         z_priq_simple_yield
# எனில் defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_simple_mask_best
# இல்லையெனில்
#  define _priq_run_best	z_priq_simple_best
# endif /* CONFIG_SCHED_CPU_MASK */
/* Scalable Scheduling */
#அல்லதெனில் defined(CONFIG_SCHED_SCALABLE)
#define _priq_run_init		z_priq_rb_init
#define _priq_run_add		z_priq_rb_add
#define _priq_run_remove	z_priq_rb_remove
#define _priq_run_yield         z_priq_rb_yield
# எனில் defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_rb_mask_best
# இல்லையெனில்
#  define _priq_run_best	z_priq_rb_best
# endif /* CONFIG_SCHED_CPU_MASK */
 /* Multi Queue Scheduling */
#அல்லதெனில் defined(CONFIG_SCHED_MULTIQ)
#define _priq_run_init		z_priq_mq_init
#define _priq_run_add		z_priq_mq_add
#define _priq_run_remove	z_priq_mq_remove
#define _priq_run_yield         z_priq_mq_yield
# எனில் defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_mq_mask_best
# இல்லையெனில்
#  define _priq_run_best	z_priq_mq_best
# endif /* CONFIG_SCHED_CPU_MASK */
#endif

/* Scalable Wait Queue */
#எனில் defined(CONFIG_WAITQ_SCALABLE)
#define _priq_wait_add		z_priq_rb_add
#define _priq_wait_remove	z_priq_rb_remove
#define _priq_wait_best		z_priq_rb_best
/* Simple Wait Queue */
#அல்லதெனில் defined(CONFIG_WAITQ_SIMPLE)
#define _priq_wait_add		z_priq_simple_add
#define _priq_wait_remove	z_priq_simple_remove
#define _priq_wait_best		z_priq_simple_best
#endif

#எனில் defined(CONFIG_64BIT)
#define NBITS          64
#define TRAILING_ZEROS u64_மொத்தம்_trailing_zeros
#இல்லையெனில்
#define NBITS          32
#define TRAILING_ZEROS u32_மொத்தம்_trailing_zeros
#endif /* CONFIG_64BIT */

நிலையான ALWAYS_INLINE வெறுமை z_priq_simple_init(sys_dlist_t *pq)
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
நிலையான ALWAYS_INLINE int32_t z_sched_prio_cmp(கட்டமைப்பு k_thread *thread_1, கட்டமைப்பு k_thread *thread_2)
{
	/* `prio` is <32b, so the below cannot overflow. */
	int32_t b1 = thread_1->base.prio;
	int32_t b2 = thread_2->base.prio;

	எனில் (b1 != b2) {
		திரும்பு b2 - b1;
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

	எனில் (d1 != d2) {
		/* Sooner deadline means higher effective priority.
		 * Doing the calculation with unsigned types and casting
		 * to signed isn't perfect, but at least reduces this
		 * from UB on overflow to impdef.
		 */
		திரும்பு (int32_t)(d2 - d1);
	}
#endif /* CONFIG_SCHED_DEADLINE */
	திரும்பு 0;
}

நிலையான ALWAYS_INLINE வெறுமை z_priq_simple_add(sys_dlist_t *pq, கட்டமைப்பு k_thread *thread)
{
	கட்டமைப்பு k_thread *t;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, t, base.qnode_dlist) {
		எனில் (z_sched_prio_cmp(thread, t) > 0) {
			sys_dlist_செருகு(&t->base.qnode_dlist, &thread->base.qnode_dlist);
			திரும்பு;
		}
	}

	sys_dlist_append(pq, &thread->base.qnode_dlist);
}

நிலையான ALWAYS_INLINE வெறுமை z_priq_simple_remove(sys_dlist_t *pq, கட்டமைப்பு k_thread *thread)
{
	ARG_UNUSED(pq);

	sys_dlist_remove(&thread->base.qnode_dlist);
}

நிலையான ALWAYS_INLINE வெறுமை z_priq_simple_yield(sys_dlist_t *pq)
{
#ifndef CONFIG_SMP
	sys_dnode_t *n;

	n = sys_dlist_peek_next_no_check(pq, &_current->base.qnode_dlist);

	sys_dlist_dequeue(&_current->base.qnode_dlist);

	கட்டமைப்பு k_thread *t;

	/*
	 * As it is possible that the current thread was not at the head of
	 * the run queue, start searching from the present position for where
	 * to re-insert it.
	 */

	போது (n != வெற்று) {
		t = CONTAINER_OF(n, கட்டமைப்பு k_thread, base.qnode_dlist);
		எனில் (z_sched_prio_cmp(_current, t) > 0) {
			sys_dlist_செருகு(&t->base.qnode_dlist,
					 &_current->base.qnode_dlist);
			திரும்பு;
		}
		n = sys_dlist_peek_next_no_check(pq, n);
	}

	sys_dlist_append(pq, &_current->base.qnode_dlist);
#endif
}

நிலையான ALWAYS_INLINE கட்டமைப்பு k_thread *z_priq_simple_best(sys_dlist_t *pq)
{
	கட்டமைப்பு k_thread *thread = வெற்று;
	sys_dnode_t *n = sys_dlist_peek_head(pq);

	எனில் (n != வெற்று) {
		thread = CONTAINER_OF(n, கட்டமைப்பு k_thread, base.qnode_dlist);
	}
	திரும்பு thread;
}

#ifdef CONFIG_SCHED_CPU_MASK
நிலையான ALWAYS_INLINE கட்டமைப்பு k_thread *z_priq_simple_mask_best(sys_dlist_t *pq)
{
	/* With masks enabled we need to be prepared to walk the list
	 * looking for one we can run
	 */
	unsigned long current_cpu_bit = BIT(_current_cpu->id);
	கட்டமைப்பு k_thread *thread;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, thread, base.qnode_dlist) {
		எனில் ((thread->base.cpu_mask & current_cpu_bit) != 0) {
			திரும்பு thread;
		}
	}
	திரும்பு வெற்று;
}
#endif /* CONFIG_SCHED_CPU_MASK */

#எனில் defined(CONFIG_SCHED_SCALABLE) || defined(CONFIG_WAITQ_SCALABLE)
நிலையான ALWAYS_INLINE வெறுமை z_priq_rb_init(கட்டமைப்பு _priq_rb *pq)
{
	bool z_priq_rb_lessthan(கட்டமைப்பு rbnode *a, கட்டமைப்பு rbnode *b);

	*pq = (கட்டமைப்பு _priq_rb) {
		.மரம் = {
			.lessthan_fn = z_priq_rb_lessthan,
		}
	};
}

நிலையான ALWAYS_INLINE வெறுமை z_priq_rb_add(கட்டமைப்பு _priq_rb *pq, கட்டமைப்பு k_thread *thread)
{
	கட்டமைப்பு k_thread *t;

	thread->base.order_சாவி = pq->next_order_சாவி;
	++pq->next_order_சாவி;

	/* Renumber at wraparound.  This is tiny code, and in practice
	 * will almost never be hit on real systems.  BUT on very
	 * long-running systems where a priq never completely empties
	 * AND that contains very large numbers of threads, it can be
	 * a latency glitch to loop over all the threads like this.
	 */
	எனில் (!pq->next_order_சாவி) {
		RB_FOR_EACH_CONTAINER(&pq->மரம், t, base.qnode_rb) {
			t->base.order_சாவி = pq->next_order_சாவி;
			++pq->next_order_சாவி;
		}
	}

	rb_செருகு(&pq->மரம், &thread->base.qnode_rb);
}

நிலையான ALWAYS_INLINE வெறுமை z_priq_rb_remove(கட்டமைப்பு _priq_rb *pq, கட்டமைப்பு k_thread *thread)
{
	rb_remove(&pq->மரம், &thread->base.qnode_rb);

	எனில் (!pq->மரம்.வேர்) {
		pq->next_order_சாவி = 0;
	}
}

நிலையான ALWAYS_INLINE வெறுமை z_priq_rb_yield(கட்டமைப்பு _priq_rb *pq)
{
#ifndef CONFIG_SMP
	z_priq_rb_remove(pq, _current);
	z_priq_rb_add(pq, _current);
#endif
}

நிலையான ALWAYS_INLINE கட்டமைப்பு k_thread *z_priq_rb_best(கட்டமைப்பு _priq_rb *pq)
{
	கட்டமைப்பு k_thread *thread = வெற்று;
	கட்டமைப்பு rbnode *n = rb_get_min(&pq->மரம்);

	எனில் (n != வெற்று) {
		thread = CONTAINER_OF(n, கட்டமைப்பு k_thread, base.qnode_rb);
	}
	திரும்பு thread;
}

#ifdef CONFIG_SCHED_CPU_MASK
நிலையான ALWAYS_INLINE கட்டமைப்பு k_thread *z_priq_rb_mask_best(கட்டமைப்பு _priq_rb *pq)
{
	unsigned long current_cpu_bit = BIT(_current_cpu->id);
	கட்டமைப்பு k_thread *thread;

	RB_FOR_EACH_CONTAINER(&pq->மரம், thread, base.qnode_rb) {
		எனில் ((thread->base.cpu_mask & current_cpu_bit) != 0) {
			திரும்பு thread;
		}
	}
	திரும்பு வெற்று;
}
#endif /* CONFIG_SCHED_CPU_MASK */
#endif /* CONFIG_SCHED_SCALABLE || CONFIG_WAITQ_SCALABLE */

கட்டமைப்பு prio_info {
	uint8_t offset_prio;
	uint8_t idx;
	uint8_t bit;
};

நிலையான ALWAYS_INLINE கட்டமைப்பு prio_info get_prio_info(int8_t old_prio)
{
	கட்டமைப்பு prio_info ret;

	ret.offset_prio = old_prio - K_HIGHEST_THREAD_PRIO;
	ret.idx = ret.offset_prio / NBITS;
	ret.bit = ret.offset_prio % NBITS;

	திரும்பு ret;
}

நிலையான ALWAYS_INLINE unsigned முழு z_priq_mq_best_queue_index(கட்டமைப்பு _priq_mq *pq)
{
	unsigned முழு i = 0;

	செய் {
		எனில் (likely(pq->bitmask[i])) {
			திரும்பு i * NBITS + TRAILING_ZEROS(pq->bitmask[i]);
		}
		i++;
	} போது (i < PRIQ_BITMAP_SIZE);

	திரும்பு K_இலக்கம்_THREAD_PRIO - 1;
}

நிலையான ALWAYS_INLINE வெறுமை z_priq_mq_init(கட்டமைப்பு _priq_mq *q)
{
	க்கு (size_t i = 0; i < ARRAY_SIZE(q->queues); i++) {
		sys_dlist_init(&q->queues[i]);
	}

#ifndef CONFIG_SMP
	q->cached_queue_index = K_இலக்கம்_THREAD_PRIO - 1;
#endif
}

நிலையான ALWAYS_INLINE வெறுமை z_priq_mq_add(கட்டமைப்பு _priq_mq *pq,
					கட்டமைப்பு k_thread *thread)
{
	கட்டமைப்பு prio_info pos = get_prio_info(thread->base.prio);

	sys_dlist_append(&pq->queues[pos.offset_prio], &thread->base.qnode_dlist);
	pq->bitmask[pos.idx] |= BIT(pos.bit);

#ifndef CONFIG_SMP
	எனில் (pos.offset_prio < pq->cached_queue_index) {
		pq->cached_queue_index = pos.offset_prio;
	}
#endif
}

நிலையான ALWAYS_INLINE வெறுமை z_priq_mq_remove(கட்டமைப்பு _priq_mq *pq,
					   கட்டமைப்பு k_thread *thread)
{
	கட்டமைப்பு prio_info pos = get_prio_info(thread->base.prio);

	sys_dlist_dequeue(&thread->base.qnode_dlist);
	எனில் (unlikely(sys_dlist_is_empty(&pq->queues[pos.offset_prio]))) {
		pq->bitmask[pos.idx] &= ~BIT(pos.bit);
#ifndef CONFIG_SMP
		pq->cached_queue_index = z_priq_mq_best_queue_index(pq);
#endif
	}
}

நிலையான ALWAYS_INLINE வெறுமை z_priq_mq_yield(கட்டமைப்பு _priq_mq *pq)
{
#ifndef CONFIG_SMP
	கட்டமைப்பு prio_info pos = get_prio_info(_current->base.prio);

	sys_dlist_dequeue(&_current->base.qnode_dlist);
	sys_dlist_append(&pq->queues[pos.offset_prio],
			 &_current->base.qnode_dlist);
#endif
}

நிலையான ALWAYS_INLINE கட்டமைப்பு k_thread *z_priq_mq_best(கட்டமைப்பு _priq_mq *pq)
{
#ifdef CONFIG_SMP
	unsigned முழு index = z_priq_mq_best_queue_index(pq);
#இல்லையெனில்
	unsigned முழு index = pq->cached_queue_index;
#endif

	sys_dnode_t *n = sys_dlist_peek_head(&pq->queues[index]);

	எனில் (likely(n != வெற்று)) {
		திரும்பு CONTAINER_OF(n, கட்டமைப்பு k_thread, base.qnode_dlist);
	}

	திரும்பு வெற்று;
}

#ifdef CONFIG_SCHED_CPU_MASK
நிலையான ALWAYS_INLINE கட்டமைப்பு k_thread *z_priq_mq_mask_best(கட்டமைப்பு _priq_mq *pq)
{
	unsigned long current_cpu_bit = BIT(_current_cpu->id);

	/* Walk priority levels from highest to lowest, and within each
	 * level walk the thread list to find the first thread runnable
	 * on the current CPU.
	 */
	க்கு (unsigned முழு i = 0U; i < PRIQ_BITMAP_SIZE; i++) {
		unsigned long bitmask = pq->bitmask[i];

		போது (bitmask != 0UL) {
			unsigned முழு bit = TRAILING_ZEROS(bitmask);
			unsigned முழு index = i * NBITS + bit;
			கட்டமைப்பு k_thread *thread;

			SYS_DLIST_FOR_EACH_CONTAINER(&pq->queues[index],
						     thread, base.qnode_dlist) {
				எனில் ((thread->base.cpu_mask &
				     current_cpu_bit) != 0) {
					திரும்பு thread;
				}
			}
			bitmask &= (bitmask - 1UL);
		}
	}
	திரும்பு வெற்று;
}
#endif /* CONFIG_SCHED_CPU_MASK */

#endif /* ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_ */
