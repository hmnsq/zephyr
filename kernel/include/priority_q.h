/*
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_
#define ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_

#ضمن <zephyr/sys/math_extras.h>
#ضمن <zephyr/sys/dlist.h>

/* Simple Scheduling */
#إذا defined(CONFIG_SCHED_SIMPLE)
#define _priq_run_init		z_priq_simple_init
#define _priq_run_add		z_priq_simple_add
#define _priq_run_remove	z_priq_simple_remove
#define _priq_run_yield         z_priq_simple_yield
# إذا defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_simple_mask_best
# وإلا
#  define _priq_run_best	z_priq_simple_best
# endif /* CONFIG_SCHED_CPU_MASK */
/* Scalable Scheduling */
#وإلاإذا defined(CONFIG_SCHED_SCALABLE)
#define _priq_run_init		z_priq_rb_init
#define _priq_run_add		z_priq_rb_add
#define _priq_run_remove	z_priq_rb_remove
#define _priq_run_yield         z_priq_rb_yield
# إذا defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_rb_mask_best
# وإلا
#  define _priq_run_best	z_priq_rb_best
# endif /* CONFIG_SCHED_CPU_MASK */
 /* Multi Queue Scheduling */
#وإلاإذا defined(CONFIG_SCHED_MULTIQ)
#define _priq_run_init		z_priq_mq_init
#define _priq_run_add		z_priq_mq_add
#define _priq_run_remove	z_priq_mq_remove
#define _priq_run_yield         z_priq_mq_yield
# إذا defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_mq_mask_best
# وإلا
#  define _priq_run_best	z_priq_mq_best
# endif /* CONFIG_SCHED_CPU_MASK */
#endif

/* Scalable Wait Queue */
#إذا defined(CONFIG_WAITQ_SCALABLE)
#define _priq_wait_add		z_priq_rb_add
#define _priq_wait_remove	z_priq_rb_remove
#define _priq_wait_best		z_priq_rb_best
/* Simple Wait Queue */
#وإلاإذا defined(CONFIG_WAITQ_SIMPLE)
#define _priq_wait_add		z_priq_simple_add
#define _priq_wait_remove	z_priq_simple_remove
#define _priq_wait_best		z_priq_simple_best
#endif

#إذا defined(CONFIG_64BIT)
#define NBITS          64
#define TRAILING_ZEROS u64_گنتی_trailing_zeros
#وإلا
#define NBITS          32
#define TRAILING_ZEROS u32_گنتی_trailing_zeros
#endif /* CONFIG_64BIT */

ساكن ALWAYS_INLINE فراغ z_priq_simple_init(sys_dlist_t *pq)
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
ساكن ALWAYS_INLINE int32_t z_sched_prio_cmp(بنية k_thread *thread_1, بنية k_thread *thread_2)
{
	/* `prio` is <32b, so the below cannot overflow. */
	int32_t b1 = thread_1->base.prio;
	int32_t b2 = thread_2->base.prio;

	إذا (b1 != b2) {
		أرجع b2 - b1;
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

	إذا (d1 != d2) {
		/* Sooner deadline means higher effective priority.
		 * Doing the calculation with unsigned types and casting
		 * to signed isn't perfect, but at least reduces this
		 * from UB on overflow to impdef.
		 */
		أرجع (int32_t)(d2 - d1);
	}
#endif /* CONFIG_SCHED_DEADLINE */
	أرجع 0;
}

ساكن ALWAYS_INLINE فراغ z_priq_simple_add(sys_dlist_t *pq, بنية k_thread *thread)
{
	بنية k_thread *t;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, t, base.qnode_dlist) {
		إذا (z_sched_prio_cmp(thread, t) > 0) {
			sys_dlist_داخل_کرو(&t->base.qnode_dlist, &thread->base.qnode_dlist);
			أرجع;
		}
	}

	sys_dlist_append(pq, &thread->base.qnode_dlist);
}

ساكن ALWAYS_INLINE فراغ z_priq_simple_remove(sys_dlist_t *pq, بنية k_thread *thread)
{
	ARG_UNUSED(pq);

	sys_dlist_remove(&thread->base.qnode_dlist);
}

ساكن ALWAYS_INLINE فراغ z_priq_simple_yield(sys_dlist_t *pq)
{
#ifndef CONFIG_SMP
	sys_dnode_t *n;

	n = sys_dlist_peek_next_no_check(pq, &_current->base.qnode_dlist);

	sys_dlist_dequeue(&_current->base.qnode_dlist);

	بنية k_thread *t;

	/*
	 * As it is possible that the current thread was not at the head of
	 * the run queue, start searching from the present position for where
	 * to re-insert it.
	 */

	طالما (n != عدم) {
		t = CONTAINER_OF(n, بنية k_thread, base.qnode_dlist);
		إذا (z_sched_prio_cmp(_current, t) > 0) {
			sys_dlist_داخل_کرو(&t->base.qnode_dlist,
					 &_current->base.qnode_dlist);
			أرجع;
		}
		n = sys_dlist_peek_next_no_check(pq, n);
	}

	sys_dlist_append(pq, &_current->base.qnode_dlist);
#endif
}

ساكن ALWAYS_INLINE بنية k_thread *z_priq_simple_best(sys_dlist_t *pq)
{
	بنية k_thread *thread = عدم;
	sys_dnode_t *n = sys_dlist_peek_head(pq);

	إذا (n != عدم) {
		thread = CONTAINER_OF(n, بنية k_thread, base.qnode_dlist);
	}
	أرجع thread;
}

#ifdef CONFIG_SCHED_CPU_MASK
ساكن ALWAYS_INLINE بنية k_thread *z_priq_simple_mask_best(sys_dlist_t *pq)
{
	/* With masks enabled we need to be prepared to walk the list
	 * looking for one we can run
	 */
	unsigned long current_cpu_bit = BIT(_current_cpu->id);
	بنية k_thread *thread;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, thread, base.qnode_dlist) {
		إذا ((thread->base.cpu_mask & current_cpu_bit) != 0) {
			أرجع thread;
		}
	}
	أرجع عدم;
}
#endif /* CONFIG_SCHED_CPU_MASK */

#إذا defined(CONFIG_SCHED_SCALABLE) || defined(CONFIG_WAITQ_SCALABLE)
ساكن ALWAYS_INLINE فراغ z_priq_rb_init(بنية _priq_rb *pq)
{
	bool z_priq_rb_lessthan(بنية rbnode *a, بنية rbnode *b);

	*pq = (بنية _priq_rb) {
		.درخت = {
			.lessthan_fn = z_priq_rb_lessthan,
		}
	};
}

ساكن ALWAYS_INLINE فراغ z_priq_rb_add(بنية _priq_rb *pq, بنية k_thread *thread)
{
	بنية k_thread *t;

	thread->base.order_کلید = pq->next_order_کلید;
	++pq->next_order_کلید;

	/* Renumber at wraparound.  This is tiny code, and in practice
	 * will almost never be hit on real systems.  BUT on very
	 * long-running systems where a priq never completely empties
	 * AND that contains very large numbers of threads, it can be
	 * a latency glitch to loop over all the threads like this.
	 */
	إذا (!pq->next_order_کلید) {
		RB_FOR_EACH_CONTAINER(&pq->درخت, t, base.qnode_rb) {
			t->base.order_کلید = pq->next_order_کلید;
			++pq->next_order_کلید;
		}
	}

	rb_داخل_کرو(&pq->درخت, &thread->base.qnode_rb);
}

ساكن ALWAYS_INLINE فراغ z_priq_rb_remove(بنية _priq_rb *pq, بنية k_thread *thread)
{
	rb_remove(&pq->درخت, &thread->base.qnode_rb);

	إذا (!pq->درخت.جڑ) {
		pq->next_order_کلید = 0;
	}
}

ساكن ALWAYS_INLINE فراغ z_priq_rb_yield(بنية _priq_rb *pq)
{
#ifndef CONFIG_SMP
	z_priq_rb_remove(pq, _current);
	z_priq_rb_add(pq, _current);
#endif
}

ساكن ALWAYS_INLINE بنية k_thread *z_priq_rb_best(بنية _priq_rb *pq)
{
	بنية k_thread *thread = عدم;
	بنية rbnode *n = rb_get_min(&pq->درخت);

	إذا (n != عدم) {
		thread = CONTAINER_OF(n, بنية k_thread, base.qnode_rb);
	}
	أرجع thread;
}

#ifdef CONFIG_SCHED_CPU_MASK
ساكن ALWAYS_INLINE بنية k_thread *z_priq_rb_mask_best(بنية _priq_rb *pq)
{
	unsigned long current_cpu_bit = BIT(_current_cpu->id);
	بنية k_thread *thread;

	RB_FOR_EACH_CONTAINER(&pq->درخت, thread, base.qnode_rb) {
		إذا ((thread->base.cpu_mask & current_cpu_bit) != 0) {
			أرجع thread;
		}
	}
	أرجع عدم;
}
#endif /* CONFIG_SCHED_CPU_MASK */
#endif /* CONFIG_SCHED_SCALABLE || CONFIG_WAITQ_SCALABLE */

بنية prio_info {
	uint8_t offset_prio;
	uint8_t idx;
	uint8_t bit;
};

ساكن ALWAYS_INLINE بنية prio_info get_prio_info(int8_t old_prio)
{
	بنية prio_info ret;

	ret.offset_prio = old_prio - K_HIGHEST_THREAD_PRIO;
	ret.idx = ret.offset_prio / NBITS;
	ret.bit = ret.offset_prio % NBITS;

	أرجع ret;
}

ساكن ALWAYS_INLINE unsigned صحيح z_priq_mq_best_queue_index(بنية _priq_mq *pq)
{
	unsigned صحيح i = 0;

	افعل {
		إذا (likely(pq->bitmask[i])) {
			أرجع i * NBITS + TRAILING_ZEROS(pq->bitmask[i]);
		}
		i++;
	} طالما (i < PRIQ_BITMAP_SIZE);

	أرجع K_عدد_THREAD_PRIO - 1;
}

ساكن ALWAYS_INLINE فراغ z_priq_mq_init(بنية _priq_mq *q)
{
	لكل (size_t i = 0; i < ARRAY_SIZE(q->queues); i++) {
		sys_dlist_init(&q->queues[i]);
	}

#ifndef CONFIG_SMP
	q->cached_queue_index = K_عدد_THREAD_PRIO - 1;
#endif
}

ساكن ALWAYS_INLINE فراغ z_priq_mq_add(بنية _priq_mq *pq,
					بنية k_thread *thread)
{
	بنية prio_info pos = get_prio_info(thread->base.prio);

	sys_dlist_append(&pq->queues[pos.offset_prio], &thread->base.qnode_dlist);
	pq->bitmask[pos.idx] |= BIT(pos.bit);

#ifndef CONFIG_SMP
	إذا (pos.offset_prio < pq->cached_queue_index) {
		pq->cached_queue_index = pos.offset_prio;
	}
#endif
}

ساكن ALWAYS_INLINE فراغ z_priq_mq_remove(بنية _priq_mq *pq,
					   بنية k_thread *thread)
{
	بنية prio_info pos = get_prio_info(thread->base.prio);

	sys_dlist_dequeue(&thread->base.qnode_dlist);
	إذا (unlikely(sys_dlist_is_empty(&pq->queues[pos.offset_prio]))) {
		pq->bitmask[pos.idx] &= ~BIT(pos.bit);
#ifndef CONFIG_SMP
		pq->cached_queue_index = z_priq_mq_best_queue_index(pq);
#endif
	}
}

ساكن ALWAYS_INLINE فراغ z_priq_mq_yield(بنية _priq_mq *pq)
{
#ifndef CONFIG_SMP
	بنية prio_info pos = get_prio_info(_current->base.prio);

	sys_dlist_dequeue(&_current->base.qnode_dlist);
	sys_dlist_append(&pq->queues[pos.offset_prio],
			 &_current->base.qnode_dlist);
#endif
}

ساكن ALWAYS_INLINE بنية k_thread *z_priq_mq_best(بنية _priq_mq *pq)
{
#ifdef CONFIG_SMP
	unsigned صحيح index = z_priq_mq_best_queue_index(pq);
#وإلا
	unsigned صحيح index = pq->cached_queue_index;
#endif

	sys_dnode_t *n = sys_dlist_peek_head(&pq->queues[index]);

	إذا (likely(n != عدم)) {
		أرجع CONTAINER_OF(n, بنية k_thread, base.qnode_dlist);
	}

	أرجع عدم;
}

#ifdef CONFIG_SCHED_CPU_MASK
ساكن ALWAYS_INLINE بنية k_thread *z_priq_mq_mask_best(بنية _priq_mq *pq)
{
	unsigned long current_cpu_bit = BIT(_current_cpu->id);

	/* Walk priority levels from highest to lowest, and within each
	 * level walk the thread list to find the first thread runnable
	 * on the current CPU.
	 */
	لكل (unsigned صحيح i = 0U; i < PRIQ_BITMAP_SIZE; i++) {
		unsigned long bitmask = pq->bitmask[i];

		طالما (bitmask != 0UL) {
			unsigned صحيح bit = TRAILING_ZEROS(bitmask);
			unsigned صحيح index = i * NBITS + bit;
			بنية k_thread *thread;

			SYS_DLIST_FOR_EACH_CONTAINER(&pq->queues[index],
						     thread, base.qnode_dlist) {
				إذا ((thread->base.cpu_mask &
				     current_cpu_bit) != 0) {
					أرجع thread;
				}
			}
			bitmask &= (bitmask - 1UL);
		}
	}
	أرجع عدم;
}
#endif /* CONFIG_SCHED_CPU_MASK */

#endif /* ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_ */
