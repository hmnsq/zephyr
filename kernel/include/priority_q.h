/*
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZEPHYR_KERNEL_INCLUDE_ਤਰਜੀਹ_Q_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZEPHYR_KERNEL_INCLUDE_ਤਰਜੀਹ_Q_H

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/math_extras.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/dlist.h>

/* Simple Scheduling */
#ਜੇ defined(CONFIG_SCHED_SIMPLE)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ priq_ਲੜੀਵਾਰ_init		z_priq_simple_init
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ priq_ਲੜੀਵਾਰ_ਜੋੜੋ		z_priq_simple_ਜੋੜੋ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ priq_ਲੜੀਵਾਰ_remove	z_priq_simple_remove
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ priq_ਲੜੀਵਾਰ_yield         z_priq_simple_yield
# ਜੇ defined(CONFIG_SCHED_CPU_ਨਕਾਬ) && !defined(CONFIG_SCHED_CPU_ਨਕਾਬ_PIN_ONLY)
#  define priq_ਲੜੀਵਾਰ_ਉੱਤਮ	z_priq_simple_ਨਕਾਬ_ਉੱਤਮ
# ਨਹੀਂਤਾਂ
#  define priq_ਲੜੀਵਾਰ_ਉੱਤਮ	z_priq_simple_ਉੱਤਮ
# endif /* CONFIG_SCHED_CPU_MASK */
/* Scalable Scheduling */
#ਜਾਂਜੇ defined(CONFIG_SCHED_SCALABLE)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ priq_ਲੜੀਵਾਰ_init		z_priq_rb_init
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ priq_ਲੜੀਵਾਰ_ਜੋੜੋ		z_priq_rb_ਜੋੜੋ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ priq_ਲੜੀਵਾਰ_remove	z_priq_rb_remove
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ priq_ਲੜੀਵਾਰ_yield         z_priq_rb_yield
# ਜੇ defined(CONFIG_SCHED_CPU_ਨਕਾਬ) && !defined(CONFIG_SCHED_CPU_ਨਕਾਬ_PIN_ONLY)
#  define priq_ਲੜੀਵਾਰ_ਉੱਤਮ	z_priq_rb_ਨਕਾਬ_ਉੱਤਮ
# ਨਹੀਂਤਾਂ
#  define priq_ਲੜੀਵਾਰ_ਉੱਤਮ	z_priq_rb_ਉੱਤਮ
# endif /* CONFIG_SCHED_CPU_MASK */
 /* Multi Queue Scheduling */
#ਜਾਂਜੇ defined(CONFIG_SCHED_MULTIQ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ priq_ਲੜੀਵਾਰ_init		z_priq_mq_init
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ priq_ਲੜੀਵਾਰ_ਜੋੜੋ		z_priq_mq_ਜੋੜੋ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ priq_ਲੜੀਵਾਰ_remove	z_priq_mq_remove
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ priq_ਲੜੀਵਾਰ_yield         z_priq_mq_yield
# ਜੇ defined(CONFIG_SCHED_CPU_ਨਕਾਬ) && !defined(CONFIG_SCHED_CPU_ਨਕਾਬ_PIN_ONLY)
#  define priq_ਲੜੀਵਾਰ_ਉੱਤਮ	z_priq_mq_ਨਕਾਬ_ਉੱਤਮ
# ਨਹੀਂਤਾਂ
#  define priq_ਲੜੀਵਾਰ_ਉੱਤਮ	z_priq_mq_ਉੱਤਮ
# endif /* CONFIG_SCHED_CPU_MASK */
#ਜੇ_ਅੰਤ

/* Scalable Wait Queue */
#ਜੇ defined(CONFIG_WAITQ_SCALABLE)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ priq_wait_ਜੋੜੋ		z_priq_rb_ਜੋੜੋ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _priq_wait_remove	z_priq_rb_remove
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ priq_wait_ਉੱਤਮ		z_priq_rb_ਉੱਤਮ
/* Simple Wait Queue */
#ਜਾਂਜੇ defined(CONFIG_WAITQ_SIMPLE)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ priq_wait_ਜੋੜੋ		z_priq_simple_ਜੋੜੋ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _priq_wait_remove	z_priq_simple_remove
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ priq_wait_ਉੱਤਮ		z_priq_simple_ਉੱਤਮ
#ਜੇ_ਅੰਤ

#ਜੇ defined(CONFIG_64_ਦੋਆਧਾਰੀ_ਅੰਕ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NBITS          64
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRAILING_ZEROS u64_ਗਿਣਤੀ_trailing_zeros
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NBITS          32
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRAILING_ZEROS u32_ਗਿਣਤੀ_trailing_zeros
#ਜੇ_ਅੰਤ /* CONFIG_64BIT */

ਸਥਿਰ ALWAYS_INLINE ਖਾਲੀ z_priq_simple_init(sys_dlist_t *pq)
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
ਸਥਿਰ ALWAYS_INLINE int32_t z_sched_prio_cmp(ਬਣਤਰ k_thread *thread_1, ਬਣਤਰ k_thread *thread_2)
{
	/* `prio` is <32b, so the below cannot overflow. */
	int32_t b1 = thread_1->ਅਧਾਰ.prio;
	int32_t b2 = thread_2->ਅਧਾਰ.prio;

	ਜੇ (b1 != b2) {
		ਵਾਪਸ b2 - b1;
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_DEADLINE
	/* If we assume all deadlines live within the same "half" of
	 * the 32 bit modulus space (this is a documented API rule),
	 * then the latest deadline in the queue minus the earliest is
	 * guaranteed to be (2's complement) non-negative.  We can
	 * leverage that to compare the values without having to check
	 * the current time.
	 */
	uint32_t d1 = thread_1->ਅਧਾਰ.prio_deadline;
	uint32_t d2 = thread_2->ਅਧਾਰ.prio_deadline;

	ਜੇ (d1 != d2) {
		/* Sooner deadline means higher effective priority.
		 * Doing the calculation with unsigned types and casting
		 * to signed isn't perfect, but at least reduces this
		 * from UB on overflow to impdef.
		 */
		ਵਾਪਸ (int32_t)(d2 - d1);
	}
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_DEADLINE */
	ਵਾਪਸ 0;
}

ਸਥਿਰ ALWAYS_INLINE ਖਾਲੀ z_priq_simple_ਜੋੜੋ(sys_dlist_t *pq, ਬਣਤਰ k_thread *thread)
{
	ਬਣਤਰ k_thread *t;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, t, ਅਧਾਰ.qnode_dlist) {
		ਜੇ (z_sched_prio_cmp(thread, t) > 0) {
			sys_dlist_ਪਾਓ(&t->ਅਧਾਰ.qnode_dlist, &thread->ਅਧਾਰ.qnode_dlist);
			ਵਾਪਸ;
		}
	}

	sys_dlist_append(pq, &thread->ਅਧਾਰ.qnode_dlist);
}

ਸਥਿਰ ALWAYS_INLINE ਖਾਲੀ z_priq_simple_remove(sys_dlist_t *pq, ਬਣਤਰ k_thread *thread)
{
	ARG_ਅਣਵਰਤਿਆ(pq);

	sys_dlist_remove(&thread->ਅਧਾਰ.qnode_dlist);
}

ਸਥਿਰ ALWAYS_INLINE ਖਾਲੀ z_priq_simple_yield(sys_dlist_t *pq)
{
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
	sys_dnode_t *n;

	n = sys_dlist_peek_ਅਗਲਾ_no_check(pq, &_current->ਅਧਾਰ.qnode_dlist);

	sys_dlist_ਕਤਾਰ_ਤੋਂ_ਕੱਢੋ(&_current->ਅਧਾਰ.qnode_dlist);

	ਬਣਤਰ k_thread *t;

	/*
	 * As it is possible that the current thread was not at the head of
	 * the run queue, start searching from the present position for where
	 * to re-insert it.
	 */

	ਜਦੋਂ (n != ਨਿਰਾ) {
		t = CONTAINER_ਦਾ(n, ਬਣਤਰ k_thread, ਅਧਾਰ.qnode_dlist);
		ਜੇ (z_sched_prio_cmp(_current, t) > 0) {
			sys_dlist_ਪਾਓ(&t->ਅਧਾਰ.qnode_dlist,
					 &_current->ਅਧਾਰ.qnode_dlist);
			ਵਾਪਸ;
		}
		n = sys_dlist_peek_ਅਗਲਾ_no_check(pq, n);
	}

	sys_dlist_append(pq, &_current->ਅਧਾਰ.qnode_dlist);
#ਜੇ_ਅੰਤ
}

ਸਥਿਰ ALWAYS_INLINE ਬਣਤਰ k_thread *z_priq_simple_ਉੱਤਮ(sys_dlist_t *pq)
{
	ਬਣਤਰ k_thread *thread = ਨਿਰਾ;
	sys_dnode_t *n = sys_dlist_peek_ਸਿਰ(pq);

	ਜੇ (n != ਨਿਰਾ) {
		thread = CONTAINER_ਦਾ(n, ਬਣਤਰ k_thread, ਅਧਾਰ.qnode_dlist);
	}
	ਵਾਪਸ thread;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_CPU_ਨਕਾਬ
ਸਥਿਰ ALWAYS_INLINE ਬਣਤਰ k_thread *z_priq_simple_ਨਕਾਬ_ਉੱਤਮ(sys_dlist_t *pq)
{
	/* With masks enabled we need to be prepared to walk the list
	 * looking for one we can run
	 */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ current_cpu_ਦੋਆਧਾਰੀ_ਅੰਕ = ਦੋਆਧਾਰੀ_ਅੰਕ(_current_cpu->id);
	ਬਣਤਰ k_thread *thread;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, thread, ਅਧਾਰ.qnode_dlist) {
		ਜੇ ((thread->ਅਧਾਰ.cpu_ਨਕਾਬ & current_cpu_ਦੋਆਧਾਰੀ_ਅੰਕ) != 0) {
			ਵਾਪਸ thread;
		}
	}
	ਵਾਪਸ ਨਿਰਾ;
}
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_CPU_MASK */

#ਜੇ defined(CONFIG_SCHED_SCALABLE) || defined(CONFIG_WAITQ_SCALABLE)
ਸਥਿਰ ALWAYS_INLINE ਖਾਲੀ z_priq_rb_init(ਬਣਤਰ _priq_rb *pq)
{
	bool z_priq_rb_lessthan(ਬਣਤਰ rbnode *a, ਬਣਤਰ rbnode *b);

	*pq = (ਬਣਤਰ _priq_rb) {
		.ਰੁੱਖ = {
			.lessthan_fn = z_priq_rb_lessthan,
		}
	};
}

ਸਥਿਰ ALWAYS_INLINE ਖਾਲੀ z_priq_rb_ਜੋੜੋ(ਬਣਤਰ _priq_rb *pq, ਬਣਤਰ k_thread *thread)
{
	ਬਣਤਰ k_thread *t;

	thread->ਅਧਾਰ.ਕ੍ਰਮ_ਕੁੰਜੀ = pq->ਅਗਲਾ_ਕ੍ਰਮ_ਕੁੰਜੀ;
	++pq->ਅਗਲਾ_ਕ੍ਰਮ_ਕੁੰਜੀ;

	/* Renumber at wraparound.  This is tiny code, and in practice
	 * will almost never be hit on real systems.  BUT on very
	 * long-running systems where a priq never completely empties
	 * AND that contains very large numbers of threads, it can be
	 * a latency glitch to loop over all the threads like this.
	 */
	ਜੇ (!pq->ਅਗਲਾ_ਕ੍ਰਮ_ਕੁੰਜੀ) {
		RB_FOR_EACH_CONTAINER(&pq->ਰੁੱਖ, t, ਅਧਾਰ.qnode_rb) {
			t->ਅਧਾਰ.ਕ੍ਰਮ_ਕੁੰਜੀ = pq->ਅਗਲਾ_ਕ੍ਰਮ_ਕੁੰਜੀ;
			++pq->ਅਗਲਾ_ਕ੍ਰਮ_ਕੁੰਜੀ;
		}
	}

	rb_ਪਾਓ(&pq->ਰੁੱਖ, &thread->ਅਧਾਰ.qnode_rb);
}

ਸਥਿਰ ALWAYS_INLINE ਖਾਲੀ z_priq_rb_remove(ਬਣਤਰ _priq_rb *pq, ਬਣਤਰ k_thread *thread)
{
	rb_remove(&pq->ਰੁੱਖ, &thread->ਅਧਾਰ.qnode_rb);

	ਜੇ (!pq->ਰੁੱਖ.ਜੜ੍ਹ) {
		pq->ਅਗਲਾ_ਕ੍ਰਮ_ਕੁੰਜੀ = 0;
	}
}

ਸਥਿਰ ALWAYS_INLINE ਖਾਲੀ z_priq_rb_yield(ਬਣਤਰ _priq_rb *pq)
{
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
	z_priq_rb_remove(pq, _current);
	z_priq_rb_ਜੋੜੋ(pq, _current);
#ਜੇ_ਅੰਤ
}

ਸਥਿਰ ALWAYS_INLINE ਬਣਤਰ k_thread *z_priq_rb_ਉੱਤਮ(ਬਣਤਰ _priq_rb *pq)
{
	ਬਣਤਰ k_thread *thread = ਨਿਰਾ;
	ਬਣਤਰ rbnode *n = rb_get_min(&pq->ਰੁੱਖ);

	ਜੇ (n != ਨਿਰਾ) {
		thread = CONTAINER_ਦਾ(n, ਬਣਤਰ k_thread, ਅਧਾਰ.qnode_rb);
	}
	ਵਾਪਸ thread;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_CPU_ਨਕਾਬ
ਸਥਿਰ ALWAYS_INLINE ਬਣਤਰ k_thread *z_priq_rb_ਨਕਾਬ_ਉੱਤਮ(ਬਣਤਰ _priq_rb *pq)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ current_cpu_ਦੋਆਧਾਰੀ_ਅੰਕ = ਦੋਆਧਾਰੀ_ਅੰਕ(_current_cpu->id);
	ਬਣਤਰ k_thread *thread;

	RB_FOR_EACH_CONTAINER(&pq->ਰੁੱਖ, thread, ਅਧਾਰ.qnode_rb) {
		ਜੇ ((thread->ਅਧਾਰ.cpu_ਨਕਾਬ & current_cpu_ਦੋਆਧਾਰੀ_ਅੰਕ) != 0) {
			ਵਾਪਸ thread;
		}
	}
	ਵਾਪਸ ਨਿਰਾ;
}
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_CPU_MASK */
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_SCALABLE || CONFIG_WAITQ_SCALABLE */

ਬਣਤਰ prio_info {
	uint8_t ਵਿੱਥ_prio;
	uint8_t idx;
	uint8_t ਦੋਆਧਾਰੀ_ਅੰਕ_2;
};

ਸਥਿਰ ALWAYS_INLINE ਬਣਤਰ prio_info get_prio_info(int8_t old_prio)
{
	ਬਣਤਰ prio_info ret;

	ret.ਵਿੱਥ_prio = old_prio - K_HIGHEST_THREAD_PRIO;
	ret.idx = ret.ਵਿੱਥ_prio / NBITS;
	ret.ਦੋਆਧਾਰੀ_ਅੰਕ_2 = ret.ਵਿੱਥ_prio % NBITS;

	ਵਾਪਸ ret;
}

ਸਥਿਰ ALWAYS_INLINE ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ z_priq_mq_ਉੱਤਮ_ਕਤਾਰ_ਸੂਚਕਾਂਕ(ਬਣਤਰ _priq_mq *pq)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i = 0;

	ਕਰੋ {
		ਜੇ (likely(pq->bitmask[i])) {
			ਵਾਪਸ i * NBITS + TRAILING_ZEROS(pq->bitmask[i]);
		}
		i++;
	} ਜਦੋਂ (i < PRIQ_BITMAP_ਮਾਪ);

	ਵਾਪਸ K_ਅੰਕ_THREAD_PRIO - 1;
}

ਸਥਿਰ ALWAYS_INLINE ਖਾਲੀ z_priq_mq_init(ਬਣਤਰ _priq_mq *q)
{
	ਲਈ (size_t i = 0; i < ARRAY_ਮਾਪ(q->queues); i++) {
		sys_dlist_init(&q->queues[i]);
	}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
	q->cached_ਕਤਾਰ_ਸੂਚਕਾਂਕ = K_ਅੰਕ_THREAD_PRIO - 1;
#ਜੇ_ਅੰਤ
}

ਸਥਿਰ ALWAYS_INLINE ਖਾਲੀ z_priq_mq_ਜੋੜੋ(ਬਣਤਰ _priq_mq *pq,
					ਬਣਤਰ k_thread *thread)
{
	ਬਣਤਰ prio_info pos = get_prio_info(thread->ਅਧਾਰ.prio);

	sys_dlist_append(&pq->queues[pos.ਵਿੱਥ_prio], &thread->ਅਧਾਰ.qnode_dlist);
	pq->bitmask[pos.idx] |= ਦੋਆਧਾਰੀ_ਅੰਕ(pos.ਦੋਆਧਾਰੀ_ਅੰਕ_2);

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
	ਜੇ (pos.ਵਿੱਥ_prio < pq->cached_ਕਤਾਰ_ਸੂਚਕਾਂਕ) {
		pq->cached_ਕਤਾਰ_ਸੂਚਕਾਂਕ = pos.ਵਿੱਥ_prio;
	}
#ਜੇ_ਅੰਤ
}

ਸਥਿਰ ALWAYS_INLINE ਖਾਲੀ z_priq_mq_remove(ਬਣਤਰ _priq_mq *pq,
					   ਬਣਤਰ k_thread *thread)
{
	ਬਣਤਰ prio_info pos = get_prio_info(thread->ਅਧਾਰ.prio);

	sys_dlist_ਕਤਾਰ_ਤੋਂ_ਕੱਢੋ(&thread->ਅਧਾਰ.qnode_dlist);
	ਜੇ (unlikely(sys_dlist_is_ਖਾਲੀ_ਥਾਵਾਂ(&pq->queues[pos.ਵਿੱਥ_prio]))) {
		pq->bitmask[pos.idx] &= ~ਦੋਆਧਾਰੀ_ਅੰਕ(pos.ਦੋਆਧਾਰੀ_ਅੰਕ_2);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
		pq->cached_ਕਤਾਰ_ਸੂਚਕਾਂਕ = z_priq_mq_ਉੱਤਮ_ਕਤਾਰ_ਸੂਚਕਾਂਕ(pq);
#ਜੇ_ਅੰਤ
	}
}

ਸਥਿਰ ALWAYS_INLINE ਖਾਲੀ z_priq_mq_yield(ਬਣਤਰ _priq_mq *pq)
{
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
	ਬਣਤਰ prio_info pos = get_prio_info(_current->ਅਧਾਰ.prio);

	sys_dlist_ਕਤਾਰ_ਤੋਂ_ਕੱਢੋ(&_current->ਅਧਾਰ.qnode_dlist);
	sys_dlist_append(&pq->queues[pos.ਵਿੱਥ_prio],
			 &_current->ਅਧਾਰ.qnode_dlist);
#ਜੇ_ਅੰਤ
}

ਸਥਿਰ ALWAYS_INLINE ਬਣਤਰ k_thread *z_priq_mq_ਉੱਤਮ(ਬਣਤਰ _priq_mq *pq)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਸੂਚਕਾਂਕ = z_priq_mq_ਉੱਤਮ_ਕਤਾਰ_ਸੂਚਕਾਂਕ(pq);
#ਨਹੀਂਤਾਂ
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਸੂਚਕਾਂਕ = pq->cached_ਕਤਾਰ_ਸੂਚਕਾਂਕ;
#ਜੇ_ਅੰਤ

	sys_dnode_t *n = sys_dlist_peek_ਸਿਰ(&pq->queues[ਸੂਚਕਾਂਕ]);

	ਜੇ (likely(n != ਨਿਰਾ)) {
		ਵਾਪਸ CONTAINER_ਦਾ(n, ਬਣਤਰ k_thread, ਅਧਾਰ.qnode_dlist);
	}

	ਵਾਪਸ ਨਿਰਾ;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_CPU_ਨਕਾਬ
ਸਥਿਰ ALWAYS_INLINE ਬਣਤਰ k_thread *z_priq_mq_ਨਕਾਬ_ਉੱਤਮ(ਬਣਤਰ _priq_mq *pq)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ current_cpu_ਦੋਆਧਾਰੀ_ਅੰਕ = ਦੋਆਧਾਰੀ_ਅੰਕ(_current_cpu->id);

	/* Walk priority levels from highest to lowest, and within each
	 * level walk the thread list to find the first thread runnable
	 * on the current CPU.
	 */
	ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i = 0U; i < PRIQ_BITMAP_ਮਾਪ; i++) {
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ bitmask = pq->bitmask[i];

		ਜਦੋਂ (bitmask != 0UL) {
			ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਦੋਆਧਾਰੀ_ਅੰਕ_2 = TRAILING_ZEROS(bitmask);
			ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਸੂਚਕਾਂਕ = i * NBITS + ਦੋਆਧਾਰੀ_ਅੰਕ_2;
			ਬਣਤਰ k_thread *thread;

			SYS_DLIST_FOR_EACH_CONTAINER(&pq->queues[ਸੂਚਕਾਂਕ],
						     thread, ਅਧਾਰ.qnode_dlist) {
				ਜੇ ((thread->ਅਧਾਰ.cpu_ਨਕਾਬ &
				     current_cpu_ਦੋਆਧਾਰੀ_ਅੰਕ) != 0) {
					ਵਾਪਸ thread;
				}
			}
			bitmask &= (bitmask - 1UL);
		}
	}
	ਵਾਪਸ ਨਿਰਾ;
}
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_CPU_MASK */

#ਜੇ_ਅੰਤ /* ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_ */
