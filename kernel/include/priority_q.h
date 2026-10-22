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
#如果 defined(CONFIG_SCHED_SIMPLE)
#define _priq_run_init		z_priq_simple_init
#define _priq_run_add		z_priq_simple_add
#define _priq_run_remove	z_priq_simple_remove
#define _priq_run_yield         z_priq_simple_yield
# 如果 defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_simple_mask_best
# 否则
#  define _priq_run_best	z_priq_simple_best
# endif /* CONFIG_SCHED_CPU_MASK */
/* Scalable Scheduling */
#elif defined(CONFIG_SCHED_SCALABLE)
#define _priq_run_init		z_priq_rb_init
#define _priq_run_add		z_priq_rb_add
#define _priq_run_remove	z_priq_rb_remove
#define _priq_run_yield         z_priq_rb_yield
# 如果 defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_rb_mask_best
# 否则
#  define _priq_run_best	z_priq_rb_best
# endif /* CONFIG_SCHED_CPU_MASK */
 /* Multi Queue Scheduling */
#elif defined(CONFIG_SCHED_MULTIQ)
#define _priq_run_init		z_priq_mq_init
#define _priq_run_add		z_priq_mq_add
#define _priq_run_remove	z_priq_mq_remove
#define _priq_run_yield         z_priq_mq_yield
# 如果 defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_mq_mask_best
# 否则
#  define _priq_run_best	z_priq_mq_best
# endif /* CONFIG_SCHED_CPU_MASK */
#endif

/* Scalable Wait Queue */
#如果 defined(CONFIG_WAITQ_SCALABLE)
#define _priq_wait_add		z_priq_rb_add
#define _priq_wait_remove	z_priq_rb_remove
#define _priq_wait_best		z_priq_rb_best
/* Simple Wait Queue */
#elif defined(CONFIG_WAITQ_SIMPLE)
#define _priq_wait_add		z_priq_simple_add
#define _priq_wait_remove	z_priq_simple_remove
#define _priq_wait_best		z_priq_simple_best
#endif

#如果 defined(CONFIG_64BIT)
#define NBITS          64
#define TRAILING_ZEROS u64_count_trailing_zeros
#否则
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
static ALWAYS_INLINE int32_t z_sched_prio_cmp(struct k_thread *thread_1, struct k_thread *thread_2)
{
	/* `prio` is <32b, so the below cannot overflow. */
	int32_t b1 = thread_1->base.prio;
	int32_t b2 = thread_2->base.prio;

	如果 (b1 != b2) {
		返回 b2 - b1;
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

	如果 (d1 != d2) {
		/* Sooner deadline means higher effective priority.
		 * Doing the calculation with unsigned types and casting
		 * to signed isn't perfect, but at least reduces this
		 * from UB on overflow to impdef.
		 */
		返回 (int32_t)(d2 - d1);
	}
#endif /* CONFIG_SCHED_DEADLINE */
	返回 0;
}

static ALWAYS_INLINE void z_priq_simple_add(sys_dlist_t *pq, struct k_thread *thread)
{
	struct k_thread *t;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, t, base.qnode_dlist) {
		如果 (z_sched_prio_cmp(thread, t) > 0) {
			sys_dlist_insert(&t->base.qnode_dlist, &thread->base.qnode_dlist);
			返回;
		}
	}

	sys_dlist_append(pq, &thread->base.qnode_dlist);
}

static ALWAYS_INLINE void z_priq_simple_remove(sys_dlist_t *pq, struct k_thread *thread)
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

	struct k_thread *t;

	/*
	 * As it is possible that the current thread was not at the head of
	 * the run queue, start searching from the present position for where
	 * to re-insert it.
	 */

	当 (n != NULL) {
		t = CONTAINER_OF(n, struct k_thread, base.qnode_dlist);
		如果 (z_sched_prio_cmp(_current, t) > 0) {
			sys_dlist_insert(&t->base.qnode_dlist,
					 &_current->base.qnode_dlist);
			返回;
		}
		n = sys_dlist_peek_next_no_check(pq, n);
	}

	sys_dlist_append(pq, &_current->base.qnode_dlist);
#endif
}

static ALWAYS_INLINE struct k_thread *z_priq_simple_best(sys_dlist_t *pq)
{
	struct k_thread *thread = NULL;
	sys_dnode_t *n = sys_dlist_peek_head(pq);

	如果 (n != NULL) {
		thread = CONTAINER_OF(n, struct k_thread, base.qnode_dlist);
	}
	返回 thread;
}

#ifdef CONFIG_SCHED_CPU_MASK
static ALWAYS_INLINE struct k_thread *z_priq_simple_mask_best(sys_dlist_t *pq)
{
	/* With masks enabled we need to be prepared to walk the list
	 * looking for one we can run
	 */
	unsigned long current_cpu_bit = BIT(_current_cpu->id);
	struct k_thread *thread;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, thread, base.qnode_dlist) {
		如果 ((thread->base.cpu_mask & current_cpu_bit) != 0) {
			返回 thread;
		}
	}
	返回 NULL;
}
#endif /* CONFIG_SCHED_CPU_MASK */

#如果 defined(CONFIG_SCHED_SCALABLE) || defined(CONFIG_WAITQ_SCALABLE)
static ALWAYS_INLINE void z_priq_rb_init(struct _priq_rb *pq)
{
	bool z_priq_rb_lessthan(struct rbnode *a, struct rbnode *b);

	*pq = (struct _priq_rb) {
		.tree = {
			.lessthan_fn = z_priq_rb_lessthan,
		}
	};
}

static ALWAYS_INLINE void z_priq_rb_add(struct _priq_rb *pq, struct k_thread *thread)
{
	struct k_thread *t;

	thread->base.order_key = pq->next_order_key;
	++pq->next_order_key;

	/* Renumber at wraparound.  This is tiny code, and in practice
	 * will almost never be hit on real systems.  BUT on very
	 * long-running systems where a priq never completely empties
	 * AND that contains very large numbers of threads, it can be
	 * a latency glitch to loop over all the threads like this.
	 */
	如果 (!pq->next_order_key) {
		RB_FOR_EACH_CONTAINER(&pq->tree, t, base.qnode_rb) {
			t->base.order_key = pq->next_order_key;
			++pq->next_order_key;
		}
	}

	rb_insert(&pq->tree, &thread->base.qnode_rb);
}

static ALWAYS_INLINE void z_priq_rb_remove(struct _priq_rb *pq, struct k_thread *thread)
{
	rb_remove(&pq->tree, &thread->base.qnode_rb);

	如果 (!pq->tree.root) {
		pq->next_order_key = 0;
	}
}

static ALWAYS_INLINE void z_priq_rb_yield(struct _priq_rb *pq)
{
#ifndef CONFIG_SMP
	z_priq_rb_remove(pq, _current);
	z_priq_rb_add(pq, _current);
#endif
}

static ALWAYS_INLINE struct k_thread *z_priq_rb_best(struct _priq_rb *pq)
{
	struct k_thread *thread = NULL;
	struct rbnode *n = rb_get_min(&pq->tree);

	如果 (n != NULL) {
		thread = CONTAINER_OF(n, struct k_thread, base.qnode_rb);
	}
	返回 thread;
}

#ifdef CONFIG_SCHED_CPU_MASK
static ALWAYS_INLINE struct k_thread *z_priq_rb_mask_best(struct _priq_rb *pq)
{
	unsigned long current_cpu_bit = BIT(_current_cpu->id);
	struct k_thread *thread;

	RB_FOR_EACH_CONTAINER(&pq->tree, thread, base.qnode_rb) {
		如果 ((thread->base.cpu_mask & current_cpu_bit) != 0) {
			返回 thread;
		}
	}
	返回 NULL;
}
#endif /* CONFIG_SCHED_CPU_MASK */
#endif /* CONFIG_SCHED_SCALABLE || CONFIG_WAITQ_SCALABLE */

struct prio_info {
	uint8_t offset_prio;
	uint8_t idx;
	uint8_t bit;
};

static ALWAYS_INLINE struct prio_info get_prio_info(int8_t old_prio)
{
	struct prio_info ret;

	ret.offset_prio = old_prio - K_HIGHEST_THREAD_PRIO;
	ret.idx = ret.offset_prio / NBITS;
	ret.bit = ret.offset_prio % NBITS;

	返回 ret;
}

static ALWAYS_INLINE unsigned 整数 z_priq_mq_best_queue_index(struct _priq_mq *pq)
{
	unsigned 整数 i = 0;

	do {
		如果 (likely(pq->bitmask[i])) {
			返回 i * NBITS + TRAILING_ZEROS(pq->bitmask[i]);
		}
		i++;
	} 当 (i < PRIQ_BITMAP_SIZE);

	返回 K_NUM_THREAD_PRIO - 1;
}

static ALWAYS_INLINE void z_priq_mq_init(struct _priq_mq *q)
{
	为 (size_t i = 0; i < ARRAY_SIZE(q->queues); i++) {
		sys_dlist_init(&q->queues[i]);
	}

#ifndef CONFIG_SMP
	q->cached_queue_index = K_NUM_THREAD_PRIO - 1;
#endif
}

static ALWAYS_INLINE void z_priq_mq_add(struct _priq_mq *pq,
					struct k_thread *thread)
{
	struct prio_info pos = get_prio_info(thread->base.prio);

	sys_dlist_append(&pq->queues[pos.offset_prio], &thread->base.qnode_dlist);
	pq->bitmask[pos.idx] |= BIT(pos.bit);

#ifndef CONFIG_SMP
	如果 (pos.offset_prio < pq->cached_queue_index) {
		pq->cached_queue_index = pos.offset_prio;
	}
#endif
}

static ALWAYS_INLINE void z_priq_mq_remove(struct _priq_mq *pq,
					   struct k_thread *thread)
{
	struct prio_info pos = get_prio_info(thread->base.prio);

	sys_dlist_dequeue(&thread->base.qnode_dlist);
	如果 (unlikely(sys_dlist_is_empty(&pq->queues[pos.offset_prio]))) {
		pq->bitmask[pos.idx] &= ~BIT(pos.bit);
#ifndef CONFIG_SMP
		pq->cached_queue_index = z_priq_mq_best_queue_index(pq);
#endif
	}
}

static ALWAYS_INLINE void z_priq_mq_yield(struct _priq_mq *pq)
{
#ifndef CONFIG_SMP
	struct prio_info pos = get_prio_info(_current->base.prio);

	sys_dlist_dequeue(&_current->base.qnode_dlist);
	sys_dlist_append(&pq->queues[pos.offset_prio],
			 &_current->base.qnode_dlist);
#endif
}

static ALWAYS_INLINE struct k_thread *z_priq_mq_best(struct _priq_mq *pq)
{
#ifdef CONFIG_SMP
	unsigned 整数 index = z_priq_mq_best_queue_index(pq);
#否则
	unsigned 整数 index = pq->cached_queue_index;
#endif

	sys_dnode_t *n = sys_dlist_peek_head(&pq->queues[index]);

	如果 (likely(n != NULL)) {
		返回 CONTAINER_OF(n, struct k_thread, base.qnode_dlist);
	}

	返回 NULL;
}

#ifdef CONFIG_SCHED_CPU_MASK
static ALWAYS_INLINE struct k_thread *z_priq_mq_mask_best(struct _priq_mq *pq)
{
	unsigned long current_cpu_bit = BIT(_current_cpu->id);

	/* Walk priority levels from highest to lowest, and within each
	 * level walk the thread list to find the first thread runnable
	 * on the current CPU.
	 */
	为 (unsigned 整数 i = 0U; i < PRIQ_BITMAP_SIZE; i++) {
		unsigned long bitmask = pq->bitmask[i];

		当 (bitmask != 0UL) {
			unsigned 整数 bit = TRAILING_ZEROS(bitmask);
			unsigned 整数 index = i * NBITS + bit;
			struct k_thread *thread;

			SYS_DLIST_FOR_EACH_CONTAINER(&pq->queues[index],
						     thread, base.qnode_dlist) {
				如果 ((thread->base.cpu_mask &
				     current_cpu_bit) != 0) {
					返回 thread;
				}
			}
			bitmask &= (bitmask - 1UL);
		}
	}
	返回 NULL;
}
#endif /* CONFIG_SCHED_CPU_MASK */

#endif /* ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_ */
