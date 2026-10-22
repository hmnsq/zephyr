/*
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_
#define ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_

#અન્તર્ભાવ <zephyr/sys/math_extras.h>
#અન્તર્ભાવ <zephyr/sys/dlist.h>

/* Simple Scheduling */
#જો defined(CONFIG_SCHED_SIMPLE)
#define _priq_run_init		z_priq_simple_init
#define _priq_run_add		z_priq_simple_add
#define _priq_run_remove	z_priq_simple_remove
#define _priq_run_yield         z_priq_simple_yield
# જો defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_simple_mask_best
# નહીંતર
#  define _priq_run_best	z_priq_simple_best
# endif /* CONFIG_SCHED_CPU_MASK */
/* Scalable Scheduling */
#અથવાજો defined(CONFIG_SCHED_SCALABLE)
#define _priq_run_init		z_priq_rb_init
#define _priq_run_add		z_priq_rb_add
#define _priq_run_remove	z_priq_rb_remove
#define _priq_run_yield         z_priq_rb_yield
# જો defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_rb_mask_best
# નહીંતર
#  define _priq_run_best	z_priq_rb_best
# endif /* CONFIG_SCHED_CPU_MASK */
 /* Multi Queue Scheduling */
#અથવાજો defined(CONFIG_SCHED_MULTIQ)
#define _priq_run_init		z_priq_mq_init
#define _priq_run_add		z_priq_mq_add
#define _priq_run_remove	z_priq_mq_remove
#define _priq_run_yield         z_priq_mq_yield
# જો defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_mq_mask_best
# નહીંતર
#  define _priq_run_best	z_priq_mq_best
# endif /* CONFIG_SCHED_CPU_MASK */
#endif

/* Scalable Wait Queue */
#જો defined(CONFIG_WAITQ_SCALABLE)
#define _priq_wait_add		z_priq_rb_add
#define _priq_wait_remove	z_priq_rb_remove
#define _priq_wait_best		z_priq_rb_best
/* Simple Wait Queue */
#અથવાજો defined(CONFIG_WAITQ_SIMPLE)
#define _priq_wait_add		z_priq_simple_add
#define _priq_wait_remove	z_priq_simple_remove
#define _priq_wait_best		z_priq_simple_best
#endif

#જો defined(CONFIG_64BIT)
#define NBITS          64
#define TRAILING_ZEROS u64_ગણતરી_trailing_zeros
#નહીંતર
#define NBITS          32
#define TRAILING_ZEROS u32_ગણતરી_trailing_zeros
#endif /* CONFIG_64BIT */

સ્થિર ALWAYS_INLINE શૂન્ય z_priq_simple_init(sys_dlist_t *pq)
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
સ્થિર ALWAYS_INLINE int32_t z_sched_prio_cmp(સંરચના k_thread *thread_1, સંરચના k_thread *thread_2)
{
	/* `prio` is <32b, so the below cannot overflow. */
	int32_t b1 = thread_1->base.prio;
	int32_t b2 = thread_2->base.prio;

	જો (b1 != b2) {
		પાછા b2 - b1;
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

	જો (d1 != d2) {
		/* Sooner deadline means higher effective priority.
		 * Doing the calculation with unsigned types and casting
		 * to signed isn't perfect, but at least reduces this
		 * from UB on overflow to impdef.
		 */
		પાછા (int32_t)(d2 - d1);
	}
#endif /* CONFIG_SCHED_DEADLINE */
	પાછા 0;
}

સ્થિર ALWAYS_INLINE શૂન્ય z_priq_simple_add(sys_dlist_t *pq, સંરચના k_thread *thread)
{
	સંરચના k_thread *t;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, t, base.qnode_dlist) {
		જો (z_sched_prio_cmp(thread, t) > 0) {
			sys_dlist_ઉમેરો(&t->base.qnode_dlist, &thread->base.qnode_dlist);
			પાછા;
		}
	}

	sys_dlist_append(pq, &thread->base.qnode_dlist);
}

સ્થિર ALWAYS_INLINE શૂન્ય z_priq_simple_remove(sys_dlist_t *pq, સંરચના k_thread *thread)
{
	ARG_UNUSED(pq);

	sys_dlist_remove(&thread->base.qnode_dlist);
}

સ્થિર ALWAYS_INLINE શૂન્ય z_priq_simple_yield(sys_dlist_t *pq)
{
#ifndef CONFIG_SMP
	sys_dnode_t *n;

	n = sys_dlist_peek_next_no_check(pq, &_current->base.qnode_dlist);

	sys_dlist_dequeue(&_current->base.qnode_dlist);

	સંરચના k_thread *t;

	/*
	 * As it is possible that the current thread was not at the head of
	 * the run queue, start searching from the present position for where
	 * to re-insert it.
	 */

	જ્યારે (n != ખાલી) {
		t = CONTAINER_OF(n, સંરચના k_thread, base.qnode_dlist);
		જો (z_sched_prio_cmp(_current, t) > 0) {
			sys_dlist_ઉમેરો(&t->base.qnode_dlist,
					 &_current->base.qnode_dlist);
			પાછા;
		}
		n = sys_dlist_peek_next_no_check(pq, n);
	}

	sys_dlist_append(pq, &_current->base.qnode_dlist);
#endif
}

સ્થિર ALWAYS_INLINE સંરચના k_thread *z_priq_simple_best(sys_dlist_t *pq)
{
	સંરચના k_thread *thread = ખાલી;
	sys_dnode_t *n = sys_dlist_peek_head(pq);

	જો (n != ખાલી) {
		thread = CONTAINER_OF(n, સંરચના k_thread, base.qnode_dlist);
	}
	પાછા thread;
}

#ifdef CONFIG_SCHED_CPU_MASK
સ્થિર ALWAYS_INLINE સંરચના k_thread *z_priq_simple_mask_best(sys_dlist_t *pq)
{
	/* With masks enabled we need to be prepared to walk the list
	 * looking for one we can run
	 */
	unsigned long current_cpu_bit = BIT(_current_cpu->id);
	સંરચના k_thread *thread;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, thread, base.qnode_dlist) {
		જો ((thread->base.cpu_mask & current_cpu_bit) != 0) {
			પાછા thread;
		}
	}
	પાછા ખાલી;
}
#endif /* CONFIG_SCHED_CPU_MASK */

#જો defined(CONFIG_SCHED_SCALABLE) || defined(CONFIG_WAITQ_SCALABLE)
સ્થિર ALWAYS_INLINE શૂન્ય z_priq_rb_init(સંરચના _priq_rb *pq)
{
	bool z_priq_rb_lessthan(સંરચના rbnode *a, સંરચના rbnode *b);

	*pq = (સંરચના _priq_rb) {
		.વૃક્ષ = {
			.lessthan_fn = z_priq_rb_lessthan,
		}
	};
}

સ્થિર ALWAYS_INLINE શૂન્ય z_priq_rb_add(સંરચના _priq_rb *pq, સંરચના k_thread *thread)
{
	સંરચના k_thread *t;

	thread->base.order_ચાવી = pq->next_order_ચાવી;
	++pq->next_order_ચાવી;

	/* Renumber at wraparound.  This is tiny code, and in practice
	 * will almost never be hit on real systems.  BUT on very
	 * long-running systems where a priq never completely empties
	 * AND that contains very large numbers of threads, it can be
	 * a latency glitch to loop over all the threads like this.
	 */
	જો (!pq->next_order_ચાવી) {
		RB_FOR_EACH_CONTAINER(&pq->વૃક્ષ, t, base.qnode_rb) {
			t->base.order_ચાવી = pq->next_order_ચાવી;
			++pq->next_order_ચાવી;
		}
	}

	rb_ઉમેરો(&pq->વૃક્ષ, &thread->base.qnode_rb);
}

સ્થિર ALWAYS_INLINE શૂન્ય z_priq_rb_remove(સંરચના _priq_rb *pq, સંરચના k_thread *thread)
{
	rb_remove(&pq->વૃક્ષ, &thread->base.qnode_rb);

	જો (!pq->વૃક્ષ.મૂળ) {
		pq->next_order_ચાવી = 0;
	}
}

સ્થિર ALWAYS_INLINE શૂન્ય z_priq_rb_yield(સંરચના _priq_rb *pq)
{
#ifndef CONFIG_SMP
	z_priq_rb_remove(pq, _current);
	z_priq_rb_add(pq, _current);
#endif
}

સ્થિર ALWAYS_INLINE સંરચના k_thread *z_priq_rb_best(સંરચના _priq_rb *pq)
{
	સંરચના k_thread *thread = ખાલી;
	સંરચના rbnode *n = rb_get_min(&pq->વૃક્ષ);

	જો (n != ખાલી) {
		thread = CONTAINER_OF(n, સંરચના k_thread, base.qnode_rb);
	}
	પાછા thread;
}

#ifdef CONFIG_SCHED_CPU_MASK
સ્થિર ALWAYS_INLINE સંરચના k_thread *z_priq_rb_mask_best(સંરચના _priq_rb *pq)
{
	unsigned long current_cpu_bit = BIT(_current_cpu->id);
	સંરચના k_thread *thread;

	RB_FOR_EACH_CONTAINER(&pq->વૃક્ષ, thread, base.qnode_rb) {
		જો ((thread->base.cpu_mask & current_cpu_bit) != 0) {
			પાછા thread;
		}
	}
	પાછા ખાલી;
}
#endif /* CONFIG_SCHED_CPU_MASK */
#endif /* CONFIG_SCHED_SCALABLE || CONFIG_WAITQ_SCALABLE */

સંરચના prio_info {
	uint8_t offset_prio;
	uint8_t idx;
	uint8_t bit;
};

સ્થિર ALWAYS_INLINE સંરચના prio_info get_prio_info(int8_t old_prio)
{
	સંરચના prio_info ret;

	ret.offset_prio = old_prio - K_HIGHEST_THREAD_PRIO;
	ret.idx = ret.offset_prio / NBITS;
	ret.bit = ret.offset_prio % NBITS;

	પાછા ret;
}

સ્થિર ALWAYS_INLINE unsigned પૂર્ણાંક z_priq_mq_best_queue_index(સંરચના _priq_mq *pq)
{
	unsigned પૂર્ણાંક i = 0;

	કરો {
		જો (likely(pq->bitmask[i])) {
			પાછા i * NBITS + TRAILING_ZEROS(pq->bitmask[i]);
		}
		i++;
	} જ્યારે (i < PRIQ_BITMAP_SIZE);

	પાછા K_અંક_THREAD_PRIO - 1;
}

સ્થિર ALWAYS_INLINE શૂન્ય z_priq_mq_init(સંરચના _priq_mq *q)
{
	માટે (size_t i = 0; i < ARRAY_SIZE(q->queues); i++) {
		sys_dlist_init(&q->queues[i]);
	}

#ifndef CONFIG_SMP
	q->cached_queue_index = K_અંક_THREAD_PRIO - 1;
#endif
}

સ્થિર ALWAYS_INLINE શૂન્ય z_priq_mq_add(સંરચના _priq_mq *pq,
					સંરચના k_thread *thread)
{
	સંરચના prio_info pos = get_prio_info(thread->base.prio);

	sys_dlist_append(&pq->queues[pos.offset_prio], &thread->base.qnode_dlist);
	pq->bitmask[pos.idx] |= BIT(pos.bit);

#ifndef CONFIG_SMP
	જો (pos.offset_prio < pq->cached_queue_index) {
		pq->cached_queue_index = pos.offset_prio;
	}
#endif
}

સ્થિર ALWAYS_INLINE શૂન્ય z_priq_mq_remove(સંરચના _priq_mq *pq,
					   સંરચના k_thread *thread)
{
	સંરચના prio_info pos = get_prio_info(thread->base.prio);

	sys_dlist_dequeue(&thread->base.qnode_dlist);
	જો (unlikely(sys_dlist_is_empty(&pq->queues[pos.offset_prio]))) {
		pq->bitmask[pos.idx] &= ~BIT(pos.bit);
#ifndef CONFIG_SMP
		pq->cached_queue_index = z_priq_mq_best_queue_index(pq);
#endif
	}
}

સ્થિર ALWAYS_INLINE શૂન્ય z_priq_mq_yield(સંરચના _priq_mq *pq)
{
#ifndef CONFIG_SMP
	સંરચના prio_info pos = get_prio_info(_current->base.prio);

	sys_dlist_dequeue(&_current->base.qnode_dlist);
	sys_dlist_append(&pq->queues[pos.offset_prio],
			 &_current->base.qnode_dlist);
#endif
}

સ્થિર ALWAYS_INLINE સંરચના k_thread *z_priq_mq_best(સંરચના _priq_mq *pq)
{
#ifdef CONFIG_SMP
	unsigned પૂર્ણાંક index = z_priq_mq_best_queue_index(pq);
#નહીંતર
	unsigned પૂર્ણાંક index = pq->cached_queue_index;
#endif

	sys_dnode_t *n = sys_dlist_peek_head(&pq->queues[index]);

	જો (likely(n != ખાલી)) {
		પાછા CONTAINER_OF(n, સંરચના k_thread, base.qnode_dlist);
	}

	પાછા ખાલી;
}

#ifdef CONFIG_SCHED_CPU_MASK
સ્થિર ALWAYS_INLINE સંરચના k_thread *z_priq_mq_mask_best(સંરચના _priq_mq *pq)
{
	unsigned long current_cpu_bit = BIT(_current_cpu->id);

	/* Walk priority levels from highest to lowest, and within each
	 * level walk the thread list to find the first thread runnable
	 * on the current CPU.
	 */
	માટે (unsigned પૂર્ણાંક i = 0U; i < PRIQ_BITMAP_SIZE; i++) {
		unsigned long bitmask = pq->bitmask[i];

		જ્યારે (bitmask != 0UL) {
			unsigned પૂર્ણાંક bit = TRAILING_ZEROS(bitmask);
			unsigned પૂર્ણાંક index = i * NBITS + bit;
			સંરચના k_thread *thread;

			SYS_DLIST_FOR_EACH_CONTAINER(&pq->queues[index],
						     thread, base.qnode_dlist) {
				જો ((thread->base.cpu_mask &
				     current_cpu_bit) != 0) {
					પાછા thread;
				}
			}
			bitmask &= (bitmask - 1UL);
		}
	}
	પાછા ખાલી;
}
#endif /* CONFIG_SCHED_CPU_MASK */

#endif /* ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_ */
