/*
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_
#define ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_

#include <zephyr/sys/math_extras.h>
#include <zephyr/sys/dlist.h>

/* Simple Scheduling */
#ᮚᮦᮔ᮪ defined(CONFIG_SCHED_SIMPLE)
#define _priq_run_init		z_priq_simple_init
#define _priq_run_add		z_priq_simple_add
#define _priq_run_remove	z_priq_simple_remove
#define _priq_run_yield         z_priq_simple_yield
# ᮚᮦᮔ᮪ defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_simple_mask_best
# ᮜᮤᮚ
#  define _priq_run_best	z_priq_simple_best
# endif /* CONFIG_SCHED_CPU_MASK */
/* Scalable Scheduling */
#elif defined(CONFIG_SCHED_SCALABLE)
#define _priq_run_init		z_priq_rb_init
#define _priq_run_add		z_priq_rb_add
#define _priq_run_remove	z_priq_rb_remove
#define _priq_run_yield         z_priq_rb_yield
# ᮚᮦᮔ᮪ defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_rb_mask_best
# ᮜᮤᮚ
#  define _priq_run_best	z_priq_rb_best
# endif /* CONFIG_SCHED_CPU_MASK */
 /* Multi Queue Scheduling */
#elif defined(CONFIG_SCHED_MULTIQ)
#define _priq_run_init		z_priq_mq_init
#define _priq_run_add		z_priq_mq_add
#define _priq_run_remove	z_priq_mq_remove
#define _priq_run_yield         z_priq_mq_yield
# ᮚᮦᮔ᮪ defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_mq_mask_best
# ᮜᮤᮚ
#  define _priq_run_best	z_priq_mq_best
# endif /* CONFIG_SCHED_CPU_MASK */
#endif

/* Scalable Wait Queue */
#ᮚᮦᮔ᮪ defined(CONFIG_WAITQ_SCALABLE)
#define _priq_wait_add		z_priq_rb_add
#define _priq_wait_remove	z_priq_rb_remove
#define _priq_wait_best		z_priq_rb_best
/* Simple Wait Queue */
#elif defined(CONFIG_WAITQ_SIMPLE)
#define _priq_wait_add		z_priq_simple_add
#define _priq_wait_remove	z_priq_simple_remove
#define _priq_wait_best		z_priq_simple_best
#endif

#ᮚᮦᮔ᮪ defined(CONFIG_64BIT)
#define NBITS          64
#define TRAILING_ZEROS u64_count_trailing_zeros
#ᮜᮤᮚ
#define NBITS          32
#define TRAILING_ZEROS u32_count_trailing_zeros
#endif /* CONFIG_64BIT */

static ALWAYS_INLINE void z_priq_simple_init(sys_dlist_t *pq)
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
static ALWAYS_INLINE int32_t z_sched_prio_cmp(sa_nracanaa k_thread *thread_1, sa_nracanaa k_thread *thread_2)
{
	/* `prio` is <32b, so the below cannot overflow. */
	int32_t b1 = thread_1->base.prio;
	int32_t b2 = thread_2->base.prio;

	ᮚᮦᮔ᮪ (b1 != b2) {
		ᮝᮜᮤ b2 - b1;
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

	ᮚᮦᮔ᮪ (d1 != d2) {
		/* Sooner deadline means higher effective priority.
		 * Doing the calculation with unsigned types and casting
		 * to signed isn't perfect, but at least reduces this
		 * from UB on overflow to impdef.
		 */
		ᮝᮜᮤ (int32_t)(d2 - d1);
	}
#endif /* CONFIG_SCHED_DEADLINE */
	ᮝᮜᮤ 0;
}

static ALWAYS_INLINE void z_priq_simple_add(sys_dlist_t *pq, sa_nracanaa k_thread *thread)
{
	sa_nracanaa k_thread *t;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, t, base.qnode_dlist) {
		ᮚᮦᮔ᮪ (z_sched_prio_cmp(thread, t) > 0) {
			sys_dlist_insert(&t->base.qnode_dlist, &thread->base.qnode_dlist);
			ᮝᮜᮤ;
		}
	}

	sys_dlist_append(pq, &thread->base.qnode_dlist);
}

static ALWAYS_INLINE void z_priq_simple_remove(sys_dlist_t *pq, sa_nracanaa k_thread *thread)
{
	ARG_UNUSED(pq);

	sys_dlist_remove(&thread->base.qnode_dlist);
}

static ALWAYS_INLINE void z_priq_simple_yield(sys_dlist_t *pq)
{
#ifndef CONFIG_SMP
	sys_dnode_t *n;

	n = sys_dlist_peek_next_no_check(pq, &_current->base.qnode_dlist);

	sys_dlist_dequeue(&_current->base.qnode_dlist);

	sa_nracanaa k_thread *t;

	/*
	 * As it is possible that the current thread was not at the head of
	 * the run queue, start searching from the present position for where
	 * to re-insert it.
	 */

	ᮞᮨᮓᮨᮜ (n != NULL) {
		t = CONTAINER_OF(n, sa_nracanaa k_thread, base.qnode_dlist);
		ᮚᮦᮔ᮪ (z_sched_prio_cmp(_current, t) > 0) {
			sys_dlist_insert(&t->base.qnode_dlist,
					 &_current->base.qnode_dlist);
			ᮝᮜᮤ;
		}
		n = sys_dlist_peek_next_no_check(pq, n);
	}

	sys_dlist_append(pq, &_current->base.qnode_dlist);
#endif
}

static ALWAYS_INLINE sa_nracanaa k_thread *z_priq_simple_best(sys_dlist_t *pq)
{
	sa_nracanaa k_thread *thread = NULL;
	sys_dnode_t *n = sys_dlist_peek_head(pq);

	ᮚᮦᮔ᮪ (n != NULL) {
		thread = CONTAINER_OF(n, sa_nracanaa k_thread, base.qnode_dlist);
	}
	ᮝᮜᮤ thread;
}

#ifdef CONFIG_SCHED_CPU_MASK
static ALWAYS_INLINE sa_nracanaa k_thread *z_priq_simple_mask_best(sys_dlist_t *pq)
{
	/* With masks enabled we need to be prepared to walk the list
	 * looking for one we can run
	 */
	unsigned long current_cpu_bit = BIT(_current_cpu->id);
	sa_nracanaa k_thread *thread;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, thread, base.qnode_dlist) {
		ᮚᮦᮔ᮪ ((thread->base.cpu_mask & current_cpu_bit) != 0) {
			ᮝᮜᮤ thread;
		}
	}
	ᮝᮜᮤ NULL;
}
#endif /* CONFIG_SCHED_CPU_MASK */

#ᮚᮦᮔ᮪ defined(CONFIG_SCHED_SCALABLE) || defined(CONFIG_WAITQ_SCALABLE)
static ALWAYS_INLINE void z_priq_rb_init(sa_nracanaa _priq_rb *pq)
{
	bool z_priq_rb_lessthan(sa_nracanaa rbnode *a, sa_nracanaa rbnode *b);

	*pq = (sa_nracanaa _priq_rb) {
		.tree = {
			.lessthan_fn = z_priq_rb_lessthan,
		}
	};
}

static ALWAYS_INLINE void z_priq_rb_add(sa_nracanaa _priq_rb *pq, sa_nracanaa k_thread *thread)
{
	sa_nracanaa k_thread *t;

	thread->base.order_key = pq->next_order_key;
	++pq->next_order_key;

	/* Renumber at wraparound.  This is tiny code, and in practice
	 * will almost never be hit on real systems.  BUT on very
	 * long-running systems where a priq never completely empties
	 * AND that contains very large numbers of threads, it can be
	 * a latency glitch to loop over all the threads like this.
	 */
	ᮚᮦᮔ᮪ (!pq->next_order_key) {
		RB_FOR_EACH_CONTAINER(&pq->tree, t, base.qnode_rb) {
			t->base.order_key = pq->next_order_key;
			++pq->next_order_key;
		}
	}

	rb_insert(&pq->tree, &thread->base.qnode_rb);
}

static ALWAYS_INLINE void z_priq_rb_remove(sa_nracanaa _priq_rb *pq, sa_nracanaa k_thread *thread)
{
	rb_remove(&pq->tree, &thread->base.qnode_rb);

	ᮚᮦᮔ᮪ (!pq->tree.root) {
		pq->next_order_key = 0;
	}
}

static ALWAYS_INLINE void z_priq_rb_yield(sa_nracanaa _priq_rb *pq)
{
#ifndef CONFIG_SMP
	z_priq_rb_remove(pq, _current);
	z_priq_rb_add(pq, _current);
#endif
}

static ALWAYS_INLINE sa_nracanaa k_thread *z_priq_rb_best(sa_nracanaa _priq_rb *pq)
{
	sa_nracanaa k_thread *thread = NULL;
	sa_nracanaa rbnode *n = rb_get_min(&pq->tree);

	ᮚᮦᮔ᮪ (n != NULL) {
		thread = CONTAINER_OF(n, sa_nracanaa k_thread, base.qnode_rb);
	}
	ᮝᮜᮤ thread;
}

#ifdef CONFIG_SCHED_CPU_MASK
static ALWAYS_INLINE sa_nracanaa k_thread *z_priq_rb_mask_best(sa_nracanaa _priq_rb *pq)
{
	unsigned long current_cpu_bit = BIT(_current_cpu->id);
	sa_nracanaa k_thread *thread;

	RB_FOR_EACH_CONTAINER(&pq->tree, thread, base.qnode_rb) {
		ᮚᮦᮔ᮪ ((thread->base.cpu_mask & current_cpu_bit) != 0) {
			ᮝᮜᮤ thread;
		}
	}
	ᮝᮜᮤ NULL;
}
#endif /* CONFIG_SCHED_CPU_MASK */
#endif /* CONFIG_SCHED_SCALABLE || CONFIG_WAITQ_SCALABLE */

sa_nracanaa prio_info {
	uint8_t offset_prio;
	uint8_t idx;
	uint8_t bit;
};

static ALWAYS_INLINE sa_nracanaa prio_info get_prio_info(int8_t old_prio)
{
	sa_nracanaa prio_info ret;

	ret.offset_prio = old_prio - K_HIGHEST_THREAD_PRIO;
	ret.idx = ret.offset_prio / NBITS;
	ret.bit = ret.offset_prio % NBITS;

	ᮝᮜᮤ ret;
}

static ALWAYS_INLINE unsigned puur_naa_nka z_priq_mq_best_queue_index(sa_nracanaa _priq_mq *pq)
{
	unsigned puur_naa_nka i = 0;

	do {
		ᮚᮦᮔ᮪ (likely(pq->bitmask[i])) {
			ᮝᮜᮤ i * NBITS + TRAILING_ZEROS(pq->bitmask[i]);
		}
		i++;
	} ᮞᮨᮓᮨᮜ (i < PRIQ_BITMAP_SIZE);

	ᮝᮜᮤ K_NUM_THREAD_PRIO - 1;
}

static ALWAYS_INLINE void z_priq_mq_init(sa_nracanaa _priq_mq *q)
{
	for (size_t i = 0; i < ARRAY_SIZE(q->queues); i++) {
		sys_dlist_init(&q->queues[i]);
	}

#ifndef CONFIG_SMP
	q->cached_queue_index = K_NUM_THREAD_PRIO - 1;
#endif
}

static ALWAYS_INLINE void z_priq_mq_add(sa_nracanaa _priq_mq *pq,
					sa_nracanaa k_thread *thread)
{
	sa_nracanaa prio_info pos = get_prio_info(thread->base.prio);

	sys_dlist_append(&pq->queues[pos.offset_prio], &thread->base.qnode_dlist);
	pq->bitmask[pos.idx] |= BIT(pos.bit);

#ifndef CONFIG_SMP
	ᮚᮦᮔ᮪ (pos.offset_prio < pq->cached_queue_index) {
		pq->cached_queue_index = pos.offset_prio;
	}
#endif
}

static ALWAYS_INLINE void z_priq_mq_remove(sa_nracanaa _priq_mq *pq,
					   sa_nracanaa k_thread *thread)
{
	sa_nracanaa prio_info pos = get_prio_info(thread->base.prio);

	sys_dlist_dequeue(&thread->base.qnode_dlist);
	ᮚᮦᮔ᮪ (unlikely(sys_dlist_is_empty(&pq->queues[pos.offset_prio]))) {
		pq->bitmask[pos.idx] &= ~BIT(pos.bit);
#ifndef CONFIG_SMP
		pq->cached_queue_index = z_priq_mq_best_queue_index(pq);
#endif
	}
}

static ALWAYS_INLINE void z_priq_mq_yield(sa_nracanaa _priq_mq *pq)
{
#ifndef CONFIG_SMP
	sa_nracanaa prio_info pos = get_prio_info(_current->base.prio);

	sys_dlist_dequeue(&_current->base.qnode_dlist);
	sys_dlist_append(&pq->queues[pos.offset_prio],
			 &_current->base.qnode_dlist);
#endif
}

static ALWAYS_INLINE sa_nracanaa k_thread *z_priq_mq_best(sa_nracanaa _priq_mq *pq)
{
#ifdef CONFIG_SMP
	unsigned puur_naa_nka index = z_priq_mq_best_queue_index(pq);
#ᮜᮤᮚ
	unsigned puur_naa_nka index = pq->cached_queue_index;
#endif

	sys_dnode_t *n = sys_dlist_peek_head(&pq->queues[index]);

	ᮚᮦᮔ᮪ (likely(n != NULL)) {
		ᮝᮜᮤ CONTAINER_OF(n, sa_nracanaa k_thread, base.qnode_dlist);
	}

	ᮝᮜᮤ NULL;
}

#ifdef CONFIG_SCHED_CPU_MASK
static ALWAYS_INLINE sa_nracanaa k_thread *z_priq_mq_mask_best(sa_nracanaa _priq_mq *pq)
{
	unsigned long current_cpu_bit = BIT(_current_cpu->id);

	/* Walk priority levels from highest to lowest, and within each
	 * level walk the thread list to find the first thread runnable
	 * on the current CPU.
	 */
	for (unsigned puur_naa_nka i = 0U; i < PRIQ_BITMAP_SIZE; i++) {
		unsigned long bitmask = pq->bitmask[i];

		ᮞᮨᮓᮨᮜ (bitmask != 0UL) {
			unsigned puur_naa_nka bit = TRAILING_ZEROS(bitmask);
			unsigned puur_naa_nka index = i * NBITS + bit;
			sa_nracanaa k_thread *thread;

			SYS_DLIST_FOR_EACH_CONTAINER(&pq->queues[index],
						     thread, base.qnode_dlist) {
				ᮚᮦᮔ᮪ ((thread->base.cpu_mask &
				     current_cpu_bit) != 0) {
					ᮝᮜᮤ thread;
				}
			}
			bitmask &= (bitmask - 1UL);
		}
	}
	ᮝᮜᮤ NULL;
}
#endif /* CONFIG_SCHED_CPU_MASK */

#endif /* ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_ */
