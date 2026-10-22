/*
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_
#define ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_

#ଅନ୍ତର୍ଭାଵ <zephyr/sys/math_extras.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/dlist.h>

/* Simple Scheduling */
#ଯଦି defined(CONFIG_SCHED_SIMPLE)
#define _priq_run_init		z_priq_simple_init
#define _priq_run_add		z_priq_simple_add
#define _priq_run_remove	z_priq_simple_remove
#define _priq_run_yield         z_priq_simple_yield
# ଯଦି defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_simple_mask_best
# ଅନ୍ୟଥା
#  define _priq_run_best	z_priq_simple_best
# endif /* CONFIG_SCHED_CPU_MASK */
/* Scalable Scheduling */
#ନଚେତଯଦି defined(CONFIG_SCHED_SCALABLE)
#define _priq_run_init		z_priq_rb_init
#define _priq_run_add		z_priq_rb_add
#define _priq_run_remove	z_priq_rb_remove
#define _priq_run_yield         z_priq_rb_yield
# ଯଦି defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_rb_mask_best
# ଅନ୍ୟଥା
#  define _priq_run_best	z_priq_rb_best
# endif /* CONFIG_SCHED_CPU_MASK */
 /* Multi Queue Scheduling */
#ନଚେତଯଦି defined(CONFIG_SCHED_MULTIQ)
#define _priq_run_init		z_priq_mq_init
#define _priq_run_add		z_priq_mq_add
#define _priq_run_remove	z_priq_mq_remove
#define _priq_run_yield         z_priq_mq_yield
# ଯଦି defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_mq_mask_best
# ଅନ୍ୟଥା
#  define _priq_run_best	z_priq_mq_best
# endif /* CONFIG_SCHED_CPU_MASK */
#endif

/* Scalable Wait Queue */
#ଯଦି defined(CONFIG_WAITQ_SCALABLE)
#define _priq_wait_add		z_priq_rb_add
#define _priq_wait_remove	z_priq_rb_remove
#define _priq_wait_best		z_priq_rb_best
/* Simple Wait Queue */
#ନଚେତଯଦି defined(CONFIG_WAITQ_SIMPLE)
#define _priq_wait_add		z_priq_simple_add
#define _priq_wait_remove	z_priq_simple_remove
#define _priq_wait_best		z_priq_simple_best
#endif

#ଯଦି defined(CONFIG_64BIT)
#define NBITS          64
#define TRAILING_ZEROS u64_ଗଣନା_trailing_zeros
#ଅନ୍ୟଥା
#define NBITS          32
#define TRAILING_ZEROS u32_ଗଣନା_trailing_zeros
#endif /* CONFIG_64BIT */

ସ୍ଥିର ALWAYS_INLINE ଶୂନ୍ୟ z_priq_simple_init(sys_dlist_t *pq)
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
ସ୍ଥିର ALWAYS_INLINE int32_t z_sched_prio_cmp(ସଂରଚନା k_thread *thread_1, ସଂରଚନା k_thread *thread_2)
{
	/* `prio` is <32b, so the below cannot overflow. */
	int32_t b1 = thread_1->base.prio;
	int32_t b2 = thread_2->base.prio;

	ଯଦି (b1 != b2) {
		ଫେରନ୍ତୁ b2 - b1;
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

	ଯଦି (d1 != d2) {
		/* Sooner deadline means higher effective priority.
		 * Doing the calculation with unsigned types and casting
		 * to signed isn't perfect, but at least reduces this
		 * from UB on overflow to impdef.
		 */
		ଫେରନ୍ତୁ (int32_t)(d2 - d1);
	}
#endif /* CONFIG_SCHED_DEADLINE */
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ALWAYS_INLINE ଶୂନ୍ୟ z_priq_simple_add(sys_dlist_t *pq, ସଂରଚନା k_thread *thread)
{
	ସଂରଚନା k_thread *t;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, t, base.qnode_dlist) {
		ଯଦି (z_sched_prio_cmp(thread, t) > 0) {
			sys_dlist_ଭର୍ତ୍ତି(&t->base.qnode_dlist, &thread->base.qnode_dlist);
			ଫେରନ୍ତୁ;
		}
	}

	sys_dlist_append(pq, &thread->base.qnode_dlist);
}

ସ୍ଥିର ALWAYS_INLINE ଶୂନ୍ୟ z_priq_simple_remove(sys_dlist_t *pq, ସଂରଚନା k_thread *thread)
{
	ARG_UNUSED(pq);

	sys_dlist_remove(&thread->base.qnode_dlist);
}

ସ୍ଥିର ALWAYS_INLINE ଶୂନ୍ୟ z_priq_simple_yield(sys_dlist_t *pq)
{
#ifndef CONFIG_SMP
	sys_dnode_t *n;

	n = sys_dlist_peek_next_no_check(pq, &_current->base.qnode_dlist);

	sys_dlist_dequeue(&_current->base.qnode_dlist);

	ସଂରଚନା k_thread *t;

	/*
	 * As it is possible that the current thread was not at the head of
	 * the run queue, start searching from the present position for where
	 * to re-insert it.
	 */

	ଯେତେବେଳେ (n != ଖାଲି) {
		t = CONTAINER_OF(n, ସଂରଚନା k_thread, base.qnode_dlist);
		ଯଦି (z_sched_prio_cmp(_current, t) > 0) {
			sys_dlist_ଭର୍ତ୍ତି(&t->base.qnode_dlist,
					 &_current->base.qnode_dlist);
			ଫେରନ୍ତୁ;
		}
		n = sys_dlist_peek_next_no_check(pq, n);
	}

	sys_dlist_append(pq, &_current->base.qnode_dlist);
#endif
}

ସ୍ଥିର ALWAYS_INLINE ସଂରଚନା k_thread *z_priq_simple_best(sys_dlist_t *pq)
{
	ସଂରଚନା k_thread *thread = ଖାଲି;
	sys_dnode_t *n = sys_dlist_peek_head(pq);

	ଯଦି (n != ଖାଲି) {
		thread = CONTAINER_OF(n, ସଂରଚନା k_thread, base.qnode_dlist);
	}
	ଫେରନ୍ତୁ thread;
}

#ifdef CONFIG_SCHED_CPU_MASK
ସ୍ଥିର ALWAYS_INLINE ସଂରଚନା k_thread *z_priq_simple_mask_best(sys_dlist_t *pq)
{
	/* With masks enabled we need to be prepared to walk the list
	 * looking for one we can run
	 */
	unsigned long current_cpu_bit = BIT(_current_cpu->id);
	ସଂରଚନା k_thread *thread;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, thread, base.qnode_dlist) {
		ଯଦି ((thread->base.cpu_mask & current_cpu_bit) != 0) {
			ଫେରନ୍ତୁ thread;
		}
	}
	ଫେରନ୍ତୁ ଖାଲି;
}
#endif /* CONFIG_SCHED_CPU_MASK */

#ଯଦି defined(CONFIG_SCHED_SCALABLE) || defined(CONFIG_WAITQ_SCALABLE)
ସ୍ଥିର ALWAYS_INLINE ଶୂନ୍ୟ z_priq_rb_init(ସଂରଚନା _priq_rb *pq)
{
	bool z_priq_rb_lessthan(ସଂରଚନା rbnode *a, ସଂରଚନା rbnode *b);

	*pq = (ସଂରଚନା _priq_rb) {
		.ବୃକ୍ଷ = {
			.lessthan_fn = z_priq_rb_lessthan,
		}
	};
}

ସ୍ଥିର ALWAYS_INLINE ଶୂନ୍ୟ z_priq_rb_add(ସଂରଚନା _priq_rb *pq, ସଂରଚନା k_thread *thread)
{
	ସଂରଚନା k_thread *t;

	thread->base.order_ଚାବି = pq->next_order_ଚାବି;
	++pq->next_order_ଚାବି;

	/* Renumber at wraparound.  This is tiny code, and in practice
	 * will almost never be hit on real systems.  BUT on very
	 * long-running systems where a priq never completely empties
	 * AND that contains very large numbers of threads, it can be
	 * a latency glitch to loop over all the threads like this.
	 */
	ଯଦି (!pq->next_order_ଚାବି) {
		RB_FOR_EACH_CONTAINER(&pq->ବୃକ୍ଷ, t, base.qnode_rb) {
			t->base.order_ଚାବି = pq->next_order_ଚାବି;
			++pq->next_order_ଚାବି;
		}
	}

	rb_ଭର୍ତ୍ତି(&pq->ବୃକ୍ଷ, &thread->base.qnode_rb);
}

ସ୍ଥିର ALWAYS_INLINE ଶୂନ୍ୟ z_priq_rb_remove(ସଂରଚନା _priq_rb *pq, ସଂରଚନା k_thread *thread)
{
	rb_remove(&pq->ବୃକ୍ଷ, &thread->base.qnode_rb);

	ଯଦି (!pq->ବୃକ୍ଷ.ମୂଳ) {
		pq->next_order_ଚାବି = 0;
	}
}

ସ୍ଥିର ALWAYS_INLINE ଶୂନ୍ୟ z_priq_rb_yield(ସଂରଚନା _priq_rb *pq)
{
#ifndef CONFIG_SMP
	z_priq_rb_remove(pq, _current);
	z_priq_rb_add(pq, _current);
#endif
}

ସ୍ଥିର ALWAYS_INLINE ସଂରଚନା k_thread *z_priq_rb_best(ସଂରଚନା _priq_rb *pq)
{
	ସଂରଚନା k_thread *thread = ଖାଲି;
	ସଂରଚନା rbnode *n = rb_get_min(&pq->ବୃକ୍ଷ);

	ଯଦି (n != ଖାଲି) {
		thread = CONTAINER_OF(n, ସଂରଚନା k_thread, base.qnode_rb);
	}
	ଫେରନ୍ତୁ thread;
}

#ifdef CONFIG_SCHED_CPU_MASK
ସ୍ଥିର ALWAYS_INLINE ସଂରଚନା k_thread *z_priq_rb_mask_best(ସଂରଚନା _priq_rb *pq)
{
	unsigned long current_cpu_bit = BIT(_current_cpu->id);
	ସଂରଚନା k_thread *thread;

	RB_FOR_EACH_CONTAINER(&pq->ବୃକ୍ଷ, thread, base.qnode_rb) {
		ଯଦି ((thread->base.cpu_mask & current_cpu_bit) != 0) {
			ଫେରନ୍ତୁ thread;
		}
	}
	ଫେରନ୍ତୁ ଖାଲି;
}
#endif /* CONFIG_SCHED_CPU_MASK */
#endif /* CONFIG_SCHED_SCALABLE || CONFIG_WAITQ_SCALABLE */

ସଂରଚନା prio_info {
	uint8_t offset_prio;
	uint8_t idx;
	uint8_t bit;
};

ସ୍ଥିର ALWAYS_INLINE ସଂରଚନା prio_info get_prio_info(int8_t old_prio)
{
	ସଂରଚନା prio_info ret;

	ret.offset_prio = old_prio - K_HIGHEST_THREAD_PRIO;
	ret.idx = ret.offset_prio / NBITS;
	ret.bit = ret.offset_prio % NBITS;

	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ALWAYS_INLINE unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_priq_mq_best_queue_index(ସଂରଚନା _priq_mq *pq)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0;

	କର {
		ଯଦି (likely(pq->bitmask[i])) {
			ଫେରନ୍ତୁ i * NBITS + TRAILING_ZEROS(pq->bitmask[i]);
		}
		i++;
	} ଯେତେବେଳେ (i < PRIQ_BITMAP_SIZE);

	ଫେରନ୍ତୁ K_ଅଙ୍କ_THREAD_PRIO - 1;
}

ସ୍ଥିର ALWAYS_INLINE ଶୂନ୍ୟ z_priq_mq_init(ସଂରଚନା _priq_mq *q)
{
	ପାଇଁ (size_t i = 0; i < ARRAY_SIZE(q->queues); i++) {
		sys_dlist_init(&q->queues[i]);
	}

#ifndef CONFIG_SMP
	q->cached_queue_index = K_ଅଙ୍କ_THREAD_PRIO - 1;
#endif
}

ସ୍ଥିର ALWAYS_INLINE ଶୂନ୍ୟ z_priq_mq_add(ସଂରଚନା _priq_mq *pq,
					ସଂରଚନା k_thread *thread)
{
	ସଂରଚନା prio_info pos = get_prio_info(thread->base.prio);

	sys_dlist_append(&pq->queues[pos.offset_prio], &thread->base.qnode_dlist);
	pq->bitmask[pos.idx] |= BIT(pos.bit);

#ifndef CONFIG_SMP
	ଯଦି (pos.offset_prio < pq->cached_queue_index) {
		pq->cached_queue_index = pos.offset_prio;
	}
#endif
}

ସ୍ଥିର ALWAYS_INLINE ଶୂନ୍ୟ z_priq_mq_remove(ସଂରଚନା _priq_mq *pq,
					   ସଂରଚନା k_thread *thread)
{
	ସଂରଚନା prio_info pos = get_prio_info(thread->base.prio);

	sys_dlist_dequeue(&thread->base.qnode_dlist);
	ଯଦି (unlikely(sys_dlist_is_empty(&pq->queues[pos.offset_prio]))) {
		pq->bitmask[pos.idx] &= ~BIT(pos.bit);
#ifndef CONFIG_SMP
		pq->cached_queue_index = z_priq_mq_best_queue_index(pq);
#endif
	}
}

ସ୍ଥିର ALWAYS_INLINE ଶୂନ୍ୟ z_priq_mq_yield(ସଂରଚନା _priq_mq *pq)
{
#ifndef CONFIG_SMP
	ସଂରଚନା prio_info pos = get_prio_info(_current->base.prio);

	sys_dlist_dequeue(&_current->base.qnode_dlist);
	sys_dlist_append(&pq->queues[pos.offset_prio],
			 &_current->base.qnode_dlist);
#endif
}

ସ୍ଥିର ALWAYS_INLINE ସଂରଚନା k_thread *z_priq_mq_best(ସଂରଚନା _priq_mq *pq)
{
#ifdef CONFIG_SMP
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା index = z_priq_mq_best_queue_index(pq);
#ଅନ୍ୟଥା
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା index = pq->cached_queue_index;
#endif

	sys_dnode_t *n = sys_dlist_peek_head(&pq->queues[index]);

	ଯଦି (likely(n != ଖାଲି)) {
		ଫେରନ୍ତୁ CONTAINER_OF(n, ସଂରଚନା k_thread, base.qnode_dlist);
	}

	ଫେରନ୍ତୁ ଖାଲି;
}

#ifdef CONFIG_SCHED_CPU_MASK
ସ୍ଥିର ALWAYS_INLINE ସଂରଚନା k_thread *z_priq_mq_mask_best(ସଂରଚନା _priq_mq *pq)
{
	unsigned long current_cpu_bit = BIT(_current_cpu->id);

	/* Walk priority levels from highest to lowest, and within each
	 * level walk the thread list to find the first thread runnable
	 * on the current CPU.
	 */
	ପାଇଁ (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0U; i < PRIQ_BITMAP_SIZE; i++) {
		unsigned long bitmask = pq->bitmask[i];

		ଯେତେବେଳେ (bitmask != 0UL) {
			unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା bit = TRAILING_ZEROS(bitmask);
			unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା index = i * NBITS + bit;
			ସଂରଚନା k_thread *thread;

			SYS_DLIST_FOR_EACH_CONTAINER(&pq->queues[index],
						     thread, base.qnode_dlist) {
				ଯଦି ((thread->base.cpu_mask &
				     current_cpu_bit) != 0) {
					ଫେରନ୍ତୁ thread;
				}
			}
			bitmask &= (bitmask - 1UL);
		}
	}
	ଫେରନ୍ତୁ ଖାଲି;
}
#endif /* CONFIG_SCHED_CPU_MASK */

#endif /* ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_ */
