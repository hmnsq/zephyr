/*
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_
#define ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_

#شامل <zephyr/sys/math_extras.h>
#شامل <zephyr/sys/dlist.h>

/* Simple Scheduling */
#اگر defined(CONFIG_SCHED_SIMPLE)
#define _priq_run_init		z_priq_simple_init
#define _priq_run_add		z_priq_simple_add
#define _priq_run_remove	z_priq_simple_remove
#define _priq_run_yield         z_priq_simple_yield
# اگر defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_simple_mask_best
# وگرنه
#  define _priq_run_best	z_priq_simple_best
# endif /* CONFIG_SCHED_CPU_MASK */
/* Scalable Scheduling */
#وگرنهاگر defined(CONFIG_SCHED_SCALABLE)
#define _priq_run_init		z_priq_rb_init
#define _priq_run_add		z_priq_rb_add
#define _priq_run_remove	z_priq_rb_remove
#define _priq_run_yield         z_priq_rb_yield
# اگر defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_rb_mask_best
# وگرنه
#  define _priq_run_best	z_priq_rb_best
# endif /* CONFIG_SCHED_CPU_MASK */
 /* Multi Queue Scheduling */
#وگرنهاگر defined(CONFIG_SCHED_MULTIQ)
#define _priq_run_init		z_priq_mq_init
#define _priq_run_add		z_priq_mq_add
#define _priq_run_remove	z_priq_mq_remove
#define _priq_run_yield         z_priq_mq_yield
# اگر defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_mq_mask_best
# وگرنه
#  define _priq_run_best	z_priq_mq_best
# endif /* CONFIG_SCHED_CPU_MASK */
#endif

/* Scalable Wait Queue */
#اگر defined(CONFIG_WAITQ_SCALABLE)
#define _priq_wait_add		z_priq_rb_add
#define _priq_wait_remove	z_priq_rb_remove
#define _priq_wait_best		z_priq_rb_best
/* Simple Wait Queue */
#وگرنهاگر defined(CONFIG_WAITQ_SIMPLE)
#define _priq_wait_add		z_priq_simple_add
#define _priq_wait_remove	z_priq_simple_remove
#define _priq_wait_best		z_priq_simple_best
#endif

#اگر defined(CONFIG_64BIT)
#define NBITS          64
#define TRAILING_ZEROS u64_شمار_trailing_zeros
#وگرنه
#define NBITS          32
#define TRAILING_ZEROS u32_شمار_trailing_zeros
#endif /* CONFIG_64BIT */

ایستا ALWAYS_INLINE تهی z_priq_simple_init(sys_dlist_t *pq)
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
ایستا ALWAYS_INLINE int32_t z_sched_prio_cmp(ساختار k_thread *thread_1, ساختار k_thread *thread_2)
{
	/* `prio` is <32b, so the below cannot overflow. */
	int32_t b1 = thread_1->base.prio;
	int32_t b2 = thread_2->base.prio;

	اگر (b1 != b2) {
		بازگشت b2 - b1;
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

	اگر (d1 != d2) {
		/* Sooner deadline means higher effective priority.
		 * Doing the calculation with unsigned types and casting
		 * to signed isn't perfect, but at least reduces this
		 * from UB on overflow to impdef.
		 */
		بازگشت (int32_t)(d2 - d1);
	}
#endif /* CONFIG_SCHED_DEADLINE */
	بازگشت 0;
}

ایستا ALWAYS_INLINE تهی z_priq_simple_add(sys_dlist_t *pq, ساختار k_thread *thread)
{
	ساختار k_thread *t;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, t, base.qnode_dlist) {
		اگر (z_sched_prio_cmp(thread, t) > 0) {
			sys_dlist_درج(&t->base.qnode_dlist, &thread->base.qnode_dlist);
			بازگشت;
		}
	}

	sys_dlist_append(pq, &thread->base.qnode_dlist);
}

ایستا ALWAYS_INLINE تهی z_priq_simple_remove(sys_dlist_t *pq, ساختار k_thread *thread)
{
	ARG_UNUSED(pq);

	sys_dlist_remove(&thread->base.qnode_dlist);
}

ایستا ALWAYS_INLINE تهی z_priq_simple_yield(sys_dlist_t *pq)
{
#ifndef CONFIG_SMP
	sys_dnode_t *n;

	n = sys_dlist_peek_next_no_check(pq, &_current->base.qnode_dlist);

	sys_dlist_dequeue(&_current->base.qnode_dlist);

	ساختار k_thread *t;

	/*
	 * As it is possible that the current thread was not at the head of
	 * the run queue, start searching from the present position for where
	 * to re-insert it.
	 */

	وقتی (n != پوچ) {
		t = CONTAINER_OF(n, ساختار k_thread, base.qnode_dlist);
		اگر (z_sched_prio_cmp(_current, t) > 0) {
			sys_dlist_درج(&t->base.qnode_dlist,
					 &_current->base.qnode_dlist);
			بازگشت;
		}
		n = sys_dlist_peek_next_no_check(pq, n);
	}

	sys_dlist_append(pq, &_current->base.qnode_dlist);
#endif
}

ایستا ALWAYS_INLINE ساختار k_thread *z_priq_simple_best(sys_dlist_t *pq)
{
	ساختار k_thread *thread = پوچ;
	sys_dnode_t *n = sys_dlist_peek_head(pq);

	اگر (n != پوچ) {
		thread = CONTAINER_OF(n, ساختار k_thread, base.qnode_dlist);
	}
	بازگشت thread;
}

#ifdef CONFIG_SCHED_CPU_MASK
ایستا ALWAYS_INLINE ساختار k_thread *z_priq_simple_mask_best(sys_dlist_t *pq)
{
	/* With masks enabled we need to be prepared to walk the list
	 * looking for one we can run
	 */
	unsigned long current_cpu_bit = BIT(_current_cpu->id);
	ساختار k_thread *thread;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, thread, base.qnode_dlist) {
		اگر ((thread->base.cpu_mask & current_cpu_bit) != 0) {
			بازگشت thread;
		}
	}
	بازگشت پوچ;
}
#endif /* CONFIG_SCHED_CPU_MASK */

#اگر defined(CONFIG_SCHED_SCALABLE) || defined(CONFIG_WAITQ_SCALABLE)
ایستا ALWAYS_INLINE تهی z_priq_rb_init(ساختار _priq_rb *pq)
{
	bool z_priq_rb_lessthan(ساختار rbnode *a, ساختار rbnode *b);

	*pq = (ساختار _priq_rb) {
		.درخت = {
			.lessthan_fn = z_priq_rb_lessthan,
		}
	};
}

ایستا ALWAYS_INLINE تهی z_priq_rb_add(ساختار _priq_rb *pq, ساختار k_thread *thread)
{
	ساختار k_thread *t;

	thread->base.order_کلید = pq->next_order_کلید;
	++pq->next_order_کلید;

	/* Renumber at wraparound.  This is tiny code, and in practice
	 * will almost never be hit on real systems.  BUT on very
	 * long-running systems where a priq never completely empties
	 * AND that contains very large numbers of threads, it can be
	 * a latency glitch to loop over all the threads like this.
	 */
	اگر (!pq->next_order_کلید) {
		RB_FOR_EACH_CONTAINER(&pq->درخت, t, base.qnode_rb) {
			t->base.order_کلید = pq->next_order_کلید;
			++pq->next_order_کلید;
		}
	}

	rb_درج(&pq->درخت, &thread->base.qnode_rb);
}

ایستا ALWAYS_INLINE تهی z_priq_rb_remove(ساختار _priq_rb *pq, ساختار k_thread *thread)
{
	rb_remove(&pq->درخت, &thread->base.qnode_rb);

	اگر (!pq->درخت.ریشه) {
		pq->next_order_کلید = 0;
	}
}

ایستا ALWAYS_INLINE تهی z_priq_rb_yield(ساختار _priq_rb *pq)
{
#ifndef CONFIG_SMP
	z_priq_rb_remove(pq, _current);
	z_priq_rb_add(pq, _current);
#endif
}

ایستا ALWAYS_INLINE ساختار k_thread *z_priq_rb_best(ساختار _priq_rb *pq)
{
	ساختار k_thread *thread = پوچ;
	ساختار rbnode *n = rb_get_min(&pq->درخت);

	اگر (n != پوچ) {
		thread = CONTAINER_OF(n, ساختار k_thread, base.qnode_rb);
	}
	بازگشت thread;
}

#ifdef CONFIG_SCHED_CPU_MASK
ایستا ALWAYS_INLINE ساختار k_thread *z_priq_rb_mask_best(ساختار _priq_rb *pq)
{
	unsigned long current_cpu_bit = BIT(_current_cpu->id);
	ساختار k_thread *thread;

	RB_FOR_EACH_CONTAINER(&pq->درخت, thread, base.qnode_rb) {
		اگر ((thread->base.cpu_mask & current_cpu_bit) != 0) {
			بازگشت thread;
		}
	}
	بازگشت پوچ;
}
#endif /* CONFIG_SCHED_CPU_MASK */
#endif /* CONFIG_SCHED_SCALABLE || CONFIG_WAITQ_SCALABLE */

ساختار prio_info {
	uint8_t offset_prio;
	uint8_t idx;
	uint8_t bit;
};

ایستا ALWAYS_INLINE ساختار prio_info get_prio_info(int8_t old_prio)
{
	ساختار prio_info ret;

	ret.offset_prio = old_prio - K_HIGHEST_THREAD_PRIO;
	ret.idx = ret.offset_prio / NBITS;
	ret.bit = ret.offset_prio % NBITS;

	بازگشت ret;
}

ایستا ALWAYS_INLINE unsigned صحیح z_priq_mq_best_queue_index(ساختار _priq_mq *pq)
{
	unsigned صحیح i = 0;

	انجامبده {
		اگر (likely(pq->bitmask[i])) {
			بازگشت i * NBITS + TRAILING_ZEROS(pq->bitmask[i]);
		}
		i++;
	} وقتی (i < PRIQ_BITMAP_SIZE);

	بازگشت K_رقم_THREAD_PRIO - 1;
}

ایستا ALWAYS_INLINE تهی z_priq_mq_init(ساختار _priq_mq *q)
{
	برای (size_t i = 0; i < ARRAY_SIZE(q->queues); i++) {
		sys_dlist_init(&q->queues[i]);
	}

#ifndef CONFIG_SMP
	q->cached_queue_index = K_رقم_THREAD_PRIO - 1;
#endif
}

ایستا ALWAYS_INLINE تهی z_priq_mq_add(ساختار _priq_mq *pq,
					ساختار k_thread *thread)
{
	ساختار prio_info pos = get_prio_info(thread->base.prio);

	sys_dlist_append(&pq->queues[pos.offset_prio], &thread->base.qnode_dlist);
	pq->bitmask[pos.idx] |= BIT(pos.bit);

#ifndef CONFIG_SMP
	اگر (pos.offset_prio < pq->cached_queue_index) {
		pq->cached_queue_index = pos.offset_prio;
	}
#endif
}

ایستا ALWAYS_INLINE تهی z_priq_mq_remove(ساختار _priq_mq *pq,
					   ساختار k_thread *thread)
{
	ساختار prio_info pos = get_prio_info(thread->base.prio);

	sys_dlist_dequeue(&thread->base.qnode_dlist);
	اگر (unlikely(sys_dlist_is_empty(&pq->queues[pos.offset_prio]))) {
		pq->bitmask[pos.idx] &= ~BIT(pos.bit);
#ifndef CONFIG_SMP
		pq->cached_queue_index = z_priq_mq_best_queue_index(pq);
#endif
	}
}

ایستا ALWAYS_INLINE تهی z_priq_mq_yield(ساختار _priq_mq *pq)
{
#ifndef CONFIG_SMP
	ساختار prio_info pos = get_prio_info(_current->base.prio);

	sys_dlist_dequeue(&_current->base.qnode_dlist);
	sys_dlist_append(&pq->queues[pos.offset_prio],
			 &_current->base.qnode_dlist);
#endif
}

ایستا ALWAYS_INLINE ساختار k_thread *z_priq_mq_best(ساختار _priq_mq *pq)
{
#ifdef CONFIG_SMP
	unsigned صحیح index = z_priq_mq_best_queue_index(pq);
#وگرنه
	unsigned صحیح index = pq->cached_queue_index;
#endif

	sys_dnode_t *n = sys_dlist_peek_head(&pq->queues[index]);

	اگر (likely(n != پوچ)) {
		بازگشت CONTAINER_OF(n, ساختار k_thread, base.qnode_dlist);
	}

	بازگشت پوچ;
}

#ifdef CONFIG_SCHED_CPU_MASK
ایستا ALWAYS_INLINE ساختار k_thread *z_priq_mq_mask_best(ساختار _priq_mq *pq)
{
	unsigned long current_cpu_bit = BIT(_current_cpu->id);

	/* Walk priority levels from highest to lowest, and within each
	 * level walk the thread list to find the first thread runnable
	 * on the current CPU.
	 */
	برای (unsigned صحیح i = 0U; i < PRIQ_BITMAP_SIZE; i++) {
		unsigned long bitmask = pq->bitmask[i];

		وقتی (bitmask != 0UL) {
			unsigned صحیح bit = TRAILING_ZEROS(bitmask);
			unsigned صحیح index = i * NBITS + bit;
			ساختار k_thread *thread;

			SYS_DLIST_FOR_EACH_CONTAINER(&pq->queues[index],
						     thread, base.qnode_dlist) {
				اگر ((thread->base.cpu_mask &
				     current_cpu_bit) != 0) {
					بازگشت thread;
				}
			}
			bitmask &= (bitmask - 1UL);
		}
	}
	بازگشت پوچ;
}
#endif /* CONFIG_SCHED_CPU_MASK */

#endif /* ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_ */
