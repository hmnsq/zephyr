/*
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#که_نه_تعریف_شوی ZEPHYR_KERNEL_INCLUDE_لومړیتوب_Q_H
#تعریف_کړه ZEPHYR_KERNEL_INCLUDE_لومړیتوب_Q_H

#ضمن <zephyr/sys/math_extras.h>
#ضمن <zephyr/sys/dlist.h>

/* Simple Scheduling */
#إذا defined(CONFIG_SCHED_SIMPLE)
#تعریف_کړه priq_پرلپسې_init		z_priq_simple_init
#تعریف_کړه priq_پرلپسې_جمع_کړه		z_priq_simple_جمع_کړه
#تعریف_کړه priq_پرلپسې_remove	z_priq_simple_remove
#تعریف_کړه priq_پرلپسې_yield         z_priq_simple_yield
# إذا defined(CONFIG_SCHED_CPU_ماسک) && !defined(CONFIG_SCHED_CPU_ماسک_PIN_ONLY)
#  define priq_پرلپسې_غوره	z_priq_simple_ماسک_غوره
# وإلا
#  define priq_پرلپسې_غوره	z_priq_simple_غوره
# endif /* CONFIG_SCHED_CPU_MASK */
/* Scalable Scheduling */
#وإلاإذا defined(CONFIG_SCHED_SCALABLE)
#تعریف_کړه priq_پرلپسې_init		z_priq_rb_init
#تعریف_کړه priq_پرلپسې_جمع_کړه		z_priq_rb_جمع_کړه
#تعریف_کړه priq_پرلپسې_remove	z_priq_rb_remove
#تعریف_کړه priq_پرلپسې_yield         z_priq_rb_yield
# إذا defined(CONFIG_SCHED_CPU_ماسک) && !defined(CONFIG_SCHED_CPU_ماسک_PIN_ONLY)
#  define priq_پرلپسې_غوره	z_priq_rb_ماسک_غوره
# وإلا
#  define priq_پرلپسې_غوره	z_priq_rb_غوره
# endif /* CONFIG_SCHED_CPU_MASK */
 /* Multi Queue Scheduling */
#وإلاإذا defined(CONFIG_SCHED_MULTIQ)
#تعریف_کړه priq_پرلپسې_init		z_priq_mq_init
#تعریف_کړه priq_پرلپسې_جمع_کړه		z_priq_mq_جمع_کړه
#تعریف_کړه priq_پرلپسې_remove	z_priq_mq_remove
#تعریف_کړه priq_پرلپسې_yield         z_priq_mq_yield
# إذا defined(CONFIG_SCHED_CPU_ماسک) && !defined(CONFIG_SCHED_CPU_ماسک_PIN_ONLY)
#  define priq_پرلپسې_غوره	z_priq_mq_ماسک_غوره
# وإلا
#  define priq_پرلپسې_غوره	z_priq_mq_غوره
# endif /* CONFIG_SCHED_CPU_MASK */
#د_که_پای

/* Scalable Wait Queue */
#إذا defined(CONFIG_WAITQ_SCALABLE)
#تعریف_کړه priq_wait_جمع_کړه		z_priq_rb_جمع_کړه
#تعریف_کړه _priq_wait_remove	z_priq_rb_remove
#تعریف_کړه priq_wait_غوره		z_priq_rb_غوره
/* Simple Wait Queue */
#وإلاإذا defined(CONFIG_WAITQ_SIMPLE)
#تعریف_کړه priq_wait_جمع_کړه		z_priq_simple_جمع_کړه
#تعریف_کړه _priq_wait_remove	z_priq_simple_remove
#تعریف_کړه priq_wait_غوره		z_priq_simple_غوره
#د_که_پای

#إذا defined(CONFIG_64_بټ)
#تعریف_کړه NBITS          64
#تعریف_کړه TRAILING_ZEROS u64_شمېر_trailing_zeros
#وإلا
#تعریف_کړه NBITS          32
#تعریف_کړه TRAILING_ZEROS u32_شمېر_trailing_zeros
#د_که_پای /* CONFIG_64BIT */

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
	int32_t b1 = thread_1->بنسټ.prio;
	int32_t b2 = thread_2->بنسټ.prio;

	إذا (b1 != b2) {
		أرجع b2 - b1;
	}

#که_تعریف_شوی CONFIG_SCHED_DEADLINE
	/* If we assume all deadlines live within the same "half" of
	 * the 32 bit modulus space (this is a documented API rule),
	 * then the latest deadline in the queue minus the earliest is
	 * guaranteed to be (2's complement) non-negative.  We can
	 * leverage that to compare the values without having to check
	 * the current time.
	 */
	uint32_t d1 = thread_1->بنسټ.prio_deadline;
	uint32_t d2 = thread_2->بنسټ.prio_deadline;

	إذا (d1 != d2) {
		/* Sooner deadline means higher effective priority.
		 * Doing the calculation with unsigned types and casting
		 * to signed isn't perfect, but at least reduces this
		 * from UB on overflow to impdef.
		 */
		أرجع (int32_t)(d2 - d1);
	}
#د_که_پای /* CONFIG_SCHED_DEADLINE */
	أرجع 0;
}

ساكن ALWAYS_INLINE فراغ z_priq_simple_جمع_کړه(sys_dlist_t *pq, بنية k_thread *thread)
{
	بنية k_thread *t;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, t, بنسټ.qnode_dlist) {
		إذا (z_sched_prio_cmp(thread, t) > 0) {
			sys_dlist_ورننباسه(&t->بنسټ.qnode_dlist, &thread->بنسټ.qnode_dlist);
			أرجع;
		}
	}

	sys_dlist_append(pq, &thread->بنسټ.qnode_dlist);
}

ساكن ALWAYS_INLINE فراغ z_priq_simple_remove(sys_dlist_t *pq, بنية k_thread *thread)
{
	ARG_ناکارول_شوی(pq);

	sys_dlist_remove(&thread->بنسټ.qnode_dlist);
}

ساكن ALWAYS_INLINE فراغ z_priq_simple_yield(sys_dlist_t *pq)
{
#که_نه_تعریف_شوی CONFIG_SMP
	sys_dnode_t *n;

	n = sys_dlist_peek_بل_no_check(pq, &_current->بنسټ.qnode_dlist);

	sys_dlist_له_کتار_ایستل(&_current->بنسټ.qnode_dlist);

	بنية k_thread *t;

	/*
	 * As it is possible that the current thread was not at the head of
	 * the run queue, start searching from the present position for where
	 * to re-insert it.
	 */

	طالما (n != عدم) {
		t = CONTAINER_د(n, بنية k_thread, بنسټ.qnode_dlist);
		إذا (z_sched_prio_cmp(_current, t) > 0) {
			sys_dlist_ورننباسه(&t->بنسټ.qnode_dlist,
					 &_current->بنسټ.qnode_dlist);
			أرجع;
		}
		n = sys_dlist_peek_بل_no_check(pq, n);
	}

	sys_dlist_append(pq, &_current->بنسټ.qnode_dlist);
#د_که_پای
}

ساكن ALWAYS_INLINE بنية k_thread *z_priq_simple_غوره(sys_dlist_t *pq)
{
	بنية k_thread *thread = عدم;
	sys_dnode_t *n = sys_dlist_peek_سر(pq);

	إذا (n != عدم) {
		thread = CONTAINER_د(n, بنية k_thread, بنسټ.qnode_dlist);
	}
	أرجع thread;
}

#که_تعریف_شوی CONFIG_SCHED_CPU_ماسک
ساكن ALWAYS_INLINE بنية k_thread *z_priq_simple_ماسک_غوره(sys_dlist_t *pq)
{
	/* With masks enabled we need to be prepared to walk the list
	 * looking for one we can run
	 */
	بینښه اوږد current_cpu_بټ = بټ(_current_cpu->id);
	بنية k_thread *thread;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, thread, بنسټ.qnode_dlist) {
		إذا ((thread->بنسټ.cpu_ماسک & current_cpu_بټ) != 0) {
			أرجع thread;
		}
	}
	أرجع عدم;
}
#د_که_پای /* CONFIG_SCHED_CPU_MASK */

#إذا defined(CONFIG_SCHED_SCALABLE) || defined(CONFIG_WAITQ_SCALABLE)
ساكن ALWAYS_INLINE فراغ z_priq_rb_init(بنية _priq_rb *pq)
{
	bool z_priq_rb_lessthan(بنية rbnode *a, بنية rbnode *b);

	*pq = (بنية _priq_rb) {
		.ونه = {
			.lessthan_fn = z_priq_rb_lessthan,
		}
	};
}

ساكن ALWAYS_INLINE فراغ z_priq_rb_جمع_کړه(بنية _priq_rb *pq, بنية k_thread *thread)
{
	بنية k_thread *t;

	thread->بنسټ.ترتیب_کلی = pq->بل_ترتیب_کلی;
	++pq->بل_ترتیب_کلی;

	/* Renumber at wraparound.  This is tiny code, and in practice
	 * will almost never be hit on real systems.  BUT on very
	 * long-running systems where a priq never completely empties
	 * AND that contains very large numbers of threads, it can be
	 * a latency glitch to loop over all the threads like this.
	 */
	إذا (!pq->بل_ترتیب_کلی) {
		RB_FOR_EACH_CONTAINER(&pq->ونه, t, بنسټ.qnode_rb) {
			t->بنسټ.ترتیب_کلی = pq->بل_ترتیب_کلی;
			++pq->بل_ترتیب_کلی;
		}
	}

	rb_ورننباسه(&pq->ونه, &thread->بنسټ.qnode_rb);
}

ساكن ALWAYS_INLINE فراغ z_priq_rb_remove(بنية _priq_rb *pq, بنية k_thread *thread)
{
	rb_remove(&pq->ونه, &thread->بنسټ.qnode_rb);

	إذا (!pq->ونه.ریښه) {
		pq->بل_ترتیب_کلی = 0;
	}
}

ساكن ALWAYS_INLINE فراغ z_priq_rb_yield(بنية _priq_rb *pq)
{
#که_نه_تعریف_شوی CONFIG_SMP
	z_priq_rb_remove(pq, _current);
	z_priq_rb_جمع_کړه(pq, _current);
#د_که_پای
}

ساكن ALWAYS_INLINE بنية k_thread *z_priq_rb_غوره(بنية _priq_rb *pq)
{
	بنية k_thread *thread = عدم;
	بنية rbnode *n = rb_get_min(&pq->ونه);

	إذا (n != عدم) {
		thread = CONTAINER_د(n, بنية k_thread, بنسټ.qnode_rb);
	}
	أرجع thread;
}

#که_تعریف_شوی CONFIG_SCHED_CPU_ماسک
ساكن ALWAYS_INLINE بنية k_thread *z_priq_rb_ماسک_غوره(بنية _priq_rb *pq)
{
	بینښه اوږد current_cpu_بټ = بټ(_current_cpu->id);
	بنية k_thread *thread;

	RB_FOR_EACH_CONTAINER(&pq->ونه, thread, بنسټ.qnode_rb) {
		إذا ((thread->بنسټ.cpu_ماسک & current_cpu_بټ) != 0) {
			أرجع thread;
		}
	}
	أرجع عدم;
}
#د_که_پای /* CONFIG_SCHED_CPU_MASK */
#د_که_پای /* CONFIG_SCHED_SCALABLE || CONFIG_WAITQ_SCALABLE */

بنية prio_info {
	uint8_t دننه_واټن_prio;
	uint8_t idx;
	uint8_t بټ_2;
};

ساكن ALWAYS_INLINE بنية prio_info get_prio_info(int8_t old_prio)
{
	بنية prio_info ret;

	ret.دننه_واټن_prio = old_prio - K_HIGHEST_THREAD_PRIO;
	ret.idx = ret.دننه_واټن_prio / NBITS;
	ret.بټ_2 = ret.دننه_واټن_prio % NBITS;

	أرجع ret;
}

ساكن ALWAYS_INLINE بینښه صحيح z_priq_mq_غوره_کتار_شاخص(بنية _priq_mq *pq)
{
	بینښه صحيح i = 0;

	افعل {
		إذا (likely(pq->bitmask[i])) {
			أرجع i * NBITS + TRAILING_ZEROS(pq->bitmask[i]);
		}
		i++;
	} طالما (i < PRIQ_BITMAP_اندازه);

	أرجع K_عدد_THREAD_PRIO - 1;
}

ساكن ALWAYS_INLINE فراغ z_priq_mq_init(بنية _priq_mq *q)
{
	لكل (size_t i = 0; i < ARRAY_اندازه(q->queues); i++) {
		sys_dlist_init(&q->queues[i]);
	}

#که_نه_تعریف_شوی CONFIG_SMP
	q->cached_کتار_شاخص = K_عدد_THREAD_PRIO - 1;
#د_که_پای
}

ساكن ALWAYS_INLINE فراغ z_priq_mq_جمع_کړه(بنية _priq_mq *pq,
					بنية k_thread *thread)
{
	بنية prio_info pos = get_prio_info(thread->بنسټ.prio);

	sys_dlist_append(&pq->queues[pos.دننه_واټن_prio], &thread->بنسټ.qnode_dlist);
	pq->bitmask[pos.idx] |= بټ(pos.بټ_2);

#که_نه_تعریف_شوی CONFIG_SMP
	إذا (pos.دننه_واټن_prio < pq->cached_کتار_شاخص) {
		pq->cached_کتار_شاخص = pos.دننه_واټن_prio;
	}
#د_که_پای
}

ساكن ALWAYS_INLINE فراغ z_priq_mq_remove(بنية _priq_mq *pq,
					   بنية k_thread *thread)
{
	بنية prio_info pos = get_prio_info(thread->بنسټ.prio);

	sys_dlist_له_کتار_ایستل(&thread->بنسټ.qnode_dlist);
	إذا (unlikely(sys_dlist_is_تشه(&pq->queues[pos.دننه_واټن_prio]))) {
		pq->bitmask[pos.idx] &= ~بټ(pos.بټ_2);
#که_نه_تعریف_شوی CONFIG_SMP
		pq->cached_کتار_شاخص = z_priq_mq_غوره_کتار_شاخص(pq);
#د_که_پای
	}
}

ساكن ALWAYS_INLINE فراغ z_priq_mq_yield(بنية _priq_mq *pq)
{
#که_نه_تعریف_شوی CONFIG_SMP
	بنية prio_info pos = get_prio_info(_current->بنسټ.prio);

	sys_dlist_له_کتار_ایستل(&_current->بنسټ.qnode_dlist);
	sys_dlist_append(&pq->queues[pos.دننه_واټن_prio],
			 &_current->بنسټ.qnode_dlist);
#د_که_پای
}

ساكن ALWAYS_INLINE بنية k_thread *z_priq_mq_غوره(بنية _priq_mq *pq)
{
#که_تعریف_شوی CONFIG_SMP
	بینښه صحيح شاخص = z_priq_mq_غوره_کتار_شاخص(pq);
#وإلا
	بینښه صحيح شاخص = pq->cached_کتار_شاخص;
#د_که_پای

	sys_dnode_t *n = sys_dlist_peek_سر(&pq->queues[شاخص]);

	إذا (likely(n != عدم)) {
		أرجع CONTAINER_د(n, بنية k_thread, بنسټ.qnode_dlist);
	}

	أرجع عدم;
}

#که_تعریف_شوی CONFIG_SCHED_CPU_ماسک
ساكن ALWAYS_INLINE بنية k_thread *z_priq_mq_ماسک_غوره(بنية _priq_mq *pq)
{
	بینښه اوږد current_cpu_بټ = بټ(_current_cpu->id);

	/* Walk priority levels from highest to lowest, and within each
	 * level walk the thread list to find the first thread runnable
	 * on the current CPU.
	 */
	لكل (بینښه صحيح i = 0U; i < PRIQ_BITMAP_اندازه; i++) {
		بینښه اوږد bitmask = pq->bitmask[i];

		طالما (bitmask != 0UL) {
			بینښه صحيح بټ_2 = TRAILING_ZEROS(bitmask);
			بینښه صحيح شاخص = i * NBITS + بټ_2;
			بنية k_thread *thread;

			SYS_DLIST_FOR_EACH_CONTAINER(&pq->queues[شاخص],
						     thread, بنسټ.qnode_dlist) {
				إذا ((thread->بنسټ.cpu_ماسک &
				     current_cpu_بټ) != 0) {
					أرجع thread;
				}
			}
			bitmask &= (bitmask - 1UL);
		}
	}
	أرجع عدم;
}
#د_که_پای /* CONFIG_SCHED_CPU_MASK */

#د_که_پای /* ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_ */
