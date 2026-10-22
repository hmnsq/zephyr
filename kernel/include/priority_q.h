/*
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ZEPHYR_KERNEL_INCLUDE_ಆದ್ಯತೆ_Q_H
#ವ್ಯಾಖ್ಯಾನಿಸು ZEPHYR_KERNEL_INCLUDE_ಆದ್ಯತೆ_Q_H

#ಅನ್ತರ್ಭಾವ <zephyr/sys/math_extras.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/dlist.h>

/* Simple Scheduling */
#ಆದರೆ defined(CONFIG_SCHED_SIMPLE)
#ವ್ಯಾಖ್ಯಾನಿಸು priq_ಸತತ_init		z_priq_simple_init
#ವ್ಯಾಖ್ಯಾನಿಸು priq_ಸತತ_ಕೂಡಿಸು		z_priq_simple_ಕೂಡಿಸು
#ವ್ಯಾಖ್ಯಾನಿಸು priq_ಸತತ_remove	z_priq_simple_remove
#ವ್ಯಾಖ್ಯಾನಿಸು priq_ಸತತ_yield         z_priq_simple_yield
# ಆದರೆ defined(CONFIG_SCHED_CPU_ಮುಖವಾಡ) && !defined(CONFIG_SCHED_CPU_ಮುಖವಾಡ_PIN_ONLY)
#  define priq_ಸತತ_ಉತ್ತಮ	z_priq_simple_ಮುಖವಾಡ_ಉತ್ತಮ
# ಇಲ್ಲದಿದ್ದರೆ
#  define priq_ಸತತ_ಉತ್ತಮ	z_priq_simple_ಉತ್ತಮ
# endif /* CONFIG_SCHED_CPU_MASK */
/* Scalable Scheduling */
#ಅಥವಾಆದರೆ defined(CONFIG_SCHED_SCALABLE)
#ವ್ಯಾಖ್ಯಾನಿಸು priq_ಸತತ_init		z_priq_rb_init
#ವ್ಯಾಖ್ಯಾನಿಸು priq_ಸತತ_ಕೂಡಿಸು		z_priq_rb_ಕೂಡಿಸು
#ವ್ಯಾಖ್ಯಾನಿಸು priq_ಸತತ_remove	z_priq_rb_remove
#ವ್ಯಾಖ್ಯಾನಿಸು priq_ಸತತ_yield         z_priq_rb_yield
# ಆದರೆ defined(CONFIG_SCHED_CPU_ಮುಖವಾಡ) && !defined(CONFIG_SCHED_CPU_ಮುಖವಾಡ_PIN_ONLY)
#  define priq_ಸತತ_ಉತ್ತಮ	z_priq_rb_ಮುಖವಾಡ_ಉತ್ತಮ
# ಇಲ್ಲದಿದ್ದರೆ
#  define priq_ಸತತ_ಉತ್ತಮ	z_priq_rb_ಉತ್ತಮ
# endif /* CONFIG_SCHED_CPU_MASK */
 /* Multi Queue Scheduling */
#ಅಥವಾಆದರೆ defined(CONFIG_SCHED_MULTIQ)
#ವ್ಯಾಖ್ಯಾನಿಸು priq_ಸತತ_init		z_priq_mq_init
#ವ್ಯಾಖ್ಯಾನಿಸು priq_ಸತತ_ಕೂಡಿಸು		z_priq_mq_ಕೂಡಿಸು
#ವ್ಯಾಖ್ಯಾನಿಸು priq_ಸತತ_remove	z_priq_mq_remove
#ವ್ಯಾಖ್ಯಾನಿಸು priq_ಸತತ_yield         z_priq_mq_yield
# ಆದರೆ defined(CONFIG_SCHED_CPU_ಮುಖವಾಡ) && !defined(CONFIG_SCHED_CPU_ಮುಖವಾಡ_PIN_ONLY)
#  define priq_ಸತತ_ಉತ್ತಮ	z_priq_mq_ಮುಖವಾಡ_ಉತ್ತಮ
# ಇಲ್ಲದಿದ್ದರೆ
#  define priq_ಸತತ_ಉತ್ತಮ	z_priq_mq_ಉತ್ತಮ
# endif /* CONFIG_SCHED_CPU_MASK */
#ಆದರೆ_ಅಂತ್ಯ

/* Scalable Wait Queue */
#ಆದರೆ defined(CONFIG_WAITQ_SCALABLE)
#ವ್ಯಾಖ್ಯಾನಿಸು priq_wait_ಕೂಡಿಸು		z_priq_rb_ಕೂಡಿಸು
#ವ್ಯಾಖ್ಯಾನಿಸು _priq_wait_remove	z_priq_rb_remove
#ವ್ಯಾಖ್ಯಾನಿಸು priq_wait_ಉತ್ತಮ		z_priq_rb_ಉತ್ತಮ
/* Simple Wait Queue */
#ಅಥವಾಆದರೆ defined(CONFIG_WAITQ_SIMPLE)
#ವ್ಯಾಖ್ಯಾನಿಸು priq_wait_ಕೂಡಿಸು		z_priq_simple_ಕೂಡಿಸು
#ವ್ಯಾಖ್ಯಾನಿಸು _priq_wait_remove	z_priq_simple_remove
#ವ್ಯಾಖ್ಯಾನಿಸು priq_wait_ಉತ್ತಮ		z_priq_simple_ಉತ್ತಮ
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(CONFIG_64_ಬಿಟ್)
#ವ್ಯಾಖ್ಯಾನಿಸು NBITS          64
#ವ್ಯಾಖ್ಯಾನಿಸು TRAILING_ZEROS u64_ಎಣಿಕೆ_trailing_zeros
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು NBITS          32
#ವ್ಯಾಖ್ಯಾನಿಸು TRAILING_ZEROS u32_ಎಣಿಕೆ_trailing_zeros
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_64BIT */

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
ಸ್ಥಿರ ALWAYS_INLINE int32_t z_sched_prio_cmp(ರಚನೆ k_thread *thread_1, ರಚನೆ k_thread *thread_2)
{
	/* `prio` is <32b, so the below cannot overflow. */
	int32_t b1 = thread_1->ತಳ.prio;
	int32_t b2 = thread_2->ತಳ.prio;

	ಆದರೆ (b1 != b2) {
		ಹಿಂತಿರುಗಿ b2 - b1;
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_DEADLINE
	/* If we assume all deadlines live within the same "half" of
	 * the 32 bit modulus space (this is a documented API rule),
	 * then the latest deadline in the queue minus the earliest is
	 * guaranteed to be (2's complement) non-negative.  We can
	 * leverage that to compare the values without having to check
	 * the current time.
	 */
	uint32_t d1 = thread_1->ತಳ.prio_deadline;
	uint32_t d2 = thread_2->ತಳ.prio_deadline;

	ಆದರೆ (d1 != d2) {
		/* Sooner deadline means higher effective priority.
		 * Doing the calculation with unsigned types and casting
		 * to signed isn't perfect, but at least reduces this
		 * from UB on overflow to impdef.
		 */
		ಹಿಂತಿರುಗಿ (int32_t)(d2 - d1);
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_DEADLINE */
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ z_priq_simple_ಕೂಡಿಸು(sys_dlist_t *pq, ರಚನೆ k_thread *thread)
{
	ರಚನೆ k_thread *t;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, t, ತಳ.qnode_dlist) {
		ಆದರೆ (z_sched_prio_cmp(thread, t) > 0) {
			sys_dlist_ಸೇರಿಸು(&t->ತಳ.qnode_dlist, &thread->ತಳ.qnode_dlist);
			ಹಿಂತಿರುಗಿ;
		}
	}

	sys_dlist_append(pq, &thread->ತಳ.qnode_dlist);
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ z_priq_simple_remove(sys_dlist_t *pq, ರಚನೆ k_thread *thread)
{
	ARG_ಬಳಸದ(pq);

	sys_dlist_remove(&thread->ತಳ.qnode_dlist);
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ z_priq_simple_yield(sys_dlist_t *pq)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_SMP
	sys_dnode_t *n;

	n = sys_dlist_peek_ಮುಂದಿನ_no_check(pq, &_current->ತಳ.qnode_dlist);

	sys_dlist_ಸರತಿಯಿಂದ_ತೆಗೆ(&_current->ತಳ.qnode_dlist);

	ರಚನೆ k_thread *t;

	/*
	 * As it is possible that the current thread was not at the head of
	 * the run queue, start searching from the present position for where
	 * to re-insert it.
	 */

	ಆಗ (n != ಖಾಲಿ) {
		t = CONTAINER_ದ(n, ರಚನೆ k_thread, ತಳ.qnode_dlist);
		ಆದರೆ (z_sched_prio_cmp(_current, t) > 0) {
			sys_dlist_ಸೇರಿಸು(&t->ತಳ.qnode_dlist,
					 &_current->ತಳ.qnode_dlist);
			ಹಿಂತಿರುಗಿ;
		}
		n = sys_dlist_peek_ಮುಂದಿನ_no_check(pq, n);
	}

	sys_dlist_append(pq, &_current->ತಳ.qnode_dlist);
#ಆದರೆ_ಅಂತ್ಯ
}

ಸ್ಥಿರ ALWAYS_INLINE ರಚನೆ k_thread *z_priq_simple_ಉತ್ತಮ(sys_dlist_t *pq)
{
	ರಚನೆ k_thread *thread = ಖಾಲಿ;
	sys_dnode_t *n = sys_dlist_peek_ತಲೆ(pq);

	ಆದರೆ (n != ಖಾಲಿ) {
		thread = CONTAINER_ದ(n, ರಚನೆ k_thread, ತಳ.qnode_dlist);
	}
	ಹಿಂತಿರುಗಿ thread;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_CPU_ಮುಖವಾಡ
ಸ್ಥಿರ ALWAYS_INLINE ರಚನೆ k_thread *z_priq_simple_ಮುಖವಾಡ_ಉತ್ತಮ(sys_dlist_t *pq)
{
	/* With masks enabled we need to be prepared to walk the list
	 * looking for one we can run
	 */
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ current_cpu_ಬಿಟ್ = ಬಿಟ್(_current_cpu->id);
	ರಚನೆ k_thread *thread;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, thread, ತಳ.qnode_dlist) {
		ಆದರೆ ((thread->ತಳ.cpu_ಮುಖವಾಡ & current_cpu_ಬಿಟ್) != 0) {
			ಹಿಂತಿರುಗಿ thread;
		}
	}
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_CPU_MASK */

#ಆದರೆ defined(CONFIG_SCHED_SCALABLE) || defined(CONFIG_WAITQ_SCALABLE)
ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ z_priq_rb_init(ರಚನೆ _priq_rb *pq)
{
	bool z_priq_rb_lessthan(ರಚನೆ rbnode *a, ರಚನೆ rbnode *b);

	*pq = (ರಚನೆ _priq_rb) {
		.ವೃಕ್ಷ = {
			.lessthan_fn = z_priq_rb_lessthan,
		}
	};
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ z_priq_rb_ಕೂಡಿಸು(ರಚನೆ _priq_rb *pq, ರಚನೆ k_thread *thread)
{
	ರಚನೆ k_thread *t;

	thread->ತಳ.ಕ್ರಮ_ಕೀಲಿ = pq->ಮುಂದಿನ_ಕ್ರಮ_ಕೀಲಿ;
	++pq->ಮುಂದಿನ_ಕ್ರಮ_ಕೀಲಿ;

	/* Renumber at wraparound.  This is tiny code, and in practice
	 * will almost never be hit on real systems.  BUT on very
	 * long-running systems where a priq never completely empties
	 * AND that contains very large numbers of threads, it can be
	 * a latency glitch to loop over all the threads like this.
	 */
	ಆದರೆ (!pq->ಮುಂದಿನ_ಕ್ರಮ_ಕೀಲಿ) {
		RB_FOR_EACH_CONTAINER(&pq->ವೃಕ್ಷ, t, ತಳ.qnode_rb) {
			t->ತಳ.ಕ್ರಮ_ಕೀಲಿ = pq->ಮುಂದಿನ_ಕ್ರಮ_ಕೀಲಿ;
			++pq->ಮುಂದಿನ_ಕ್ರಮ_ಕೀಲಿ;
		}
	}

	rb_ಸೇರಿಸು(&pq->ವೃಕ್ಷ, &thread->ತಳ.qnode_rb);
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ z_priq_rb_remove(ರಚನೆ _priq_rb *pq, ರಚನೆ k_thread *thread)
{
	rb_remove(&pq->ವೃಕ್ಷ, &thread->ತಳ.qnode_rb);

	ಆದರೆ (!pq->ವೃಕ್ಷ.ಬೇರು) {
		pq->ಮುಂದಿನ_ಕ್ರಮ_ಕೀಲಿ = 0;
	}
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ z_priq_rb_yield(ರಚನೆ _priq_rb *pq)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_SMP
	z_priq_rb_remove(pq, _current);
	z_priq_rb_ಕೂಡಿಸು(pq, _current);
#ಆದರೆ_ಅಂತ್ಯ
}

ಸ್ಥಿರ ALWAYS_INLINE ರಚನೆ k_thread *z_priq_rb_ಉತ್ತಮ(ರಚನೆ _priq_rb *pq)
{
	ರಚನೆ k_thread *thread = ಖಾಲಿ;
	ರಚನೆ rbnode *n = rb_get_min(&pq->ವೃಕ್ಷ);

	ಆದರೆ (n != ಖಾಲಿ) {
		thread = CONTAINER_ದ(n, ರಚನೆ k_thread, ತಳ.qnode_rb);
	}
	ಹಿಂತಿರುಗಿ thread;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_CPU_ಮುಖವಾಡ
ಸ್ಥಿರ ALWAYS_INLINE ರಚನೆ k_thread *z_priq_rb_ಮುಖವಾಡ_ಉತ್ತಮ(ರಚನೆ _priq_rb *pq)
{
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ current_cpu_ಬಿಟ್ = ಬಿಟ್(_current_cpu->id);
	ರಚನೆ k_thread *thread;

	RB_FOR_EACH_CONTAINER(&pq->ವೃಕ್ಷ, thread, ತಳ.qnode_rb) {
		ಆದರೆ ((thread->ತಳ.cpu_ಮುಖವಾಡ & current_cpu_ಬಿಟ್) != 0) {
			ಹಿಂತಿರುಗಿ thread;
		}
	}
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_CPU_MASK */
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_SCALABLE || CONFIG_WAITQ_SCALABLE */

ರಚನೆ prio_info {
	uint8_t ಸ್ಥಾನಾಂತರ_prio;
	uint8_t idx;
	uint8_t ಬಿಟ್_2;
};

ಸ್ಥಿರ ALWAYS_INLINE ರಚನೆ prio_info get_prio_info(int8_t old_prio)
{
	ರಚನೆ prio_info ret;

	ret.ಸ್ಥಾನಾಂತರ_prio = old_prio - K_HIGHEST_THREAD_PRIO;
	ret.idx = ret.ಸ್ಥಾನಾಂತರ_prio / NBITS;
	ret.ಬಿಟ್_2 = ret.ಸ್ಥಾನಾಂತರ_prio % NBITS;

	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ALWAYS_INLINE ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ z_priq_mq_ಉತ್ತಮ_ಸರತಿ_ಸೂಚ್ಯಂಕ(ರಚನೆ _priq_mq *pq)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i = 0;

	ಮಾಡು {
		ಆದರೆ (likely(pq->bitmask[i])) {
			ಹಿಂತಿರುಗಿ i * NBITS + TRAILING_ZEROS(pq->bitmask[i]);
		}
		i++;
	} ಆಗ (i < PRIQ_BITMAP_ಗಾತ್ರ);

	ಹಿಂತಿರುಗಿ K_ಅಂಕೆ_THREAD_PRIO - 1;
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ z_priq_mq_init(ರಚನೆ _priq_mq *q)
{
	ಗಾಗಿ (size_t i = 0; i < ARRAY_ಗಾತ್ರ(q->queues); i++) {
		sys_dlist_init(&q->queues[i]);
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_SMP
	q->cached_ಸರತಿ_ಸೂಚ್ಯಂಕ = K_ಅಂಕೆ_THREAD_PRIO - 1;
#ಆದರೆ_ಅಂತ್ಯ
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ z_priq_mq_ಕೂಡಿಸು(ರಚನೆ _priq_mq *pq,
					ರಚನೆ k_thread *thread)
{
	ರಚನೆ prio_info pos = get_prio_info(thread->ತಳ.prio);

	sys_dlist_append(&pq->queues[pos.ಸ್ಥಾನಾಂತರ_prio], &thread->ತಳ.qnode_dlist);
	pq->bitmask[pos.idx] |= ಬಿಟ್(pos.ಬಿಟ್_2);

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_SMP
	ಆದರೆ (pos.ಸ್ಥಾನಾಂತರ_prio < pq->cached_ಸರತಿ_ಸೂಚ್ಯಂಕ) {
		pq->cached_ಸರತಿ_ಸೂಚ್ಯಂಕ = pos.ಸ್ಥಾನಾಂತರ_prio;
	}
#ಆದರೆ_ಅಂತ್ಯ
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ z_priq_mq_remove(ರಚನೆ _priq_mq *pq,
					   ರಚನೆ k_thread *thread)
{
	ರಚನೆ prio_info pos = get_prio_info(thread->ತಳ.prio);

	sys_dlist_ಸರತಿಯಿಂದ_ತೆಗೆ(&thread->ತಳ.qnode_dlist);
	ಆದರೆ (unlikely(sys_dlist_is_ಖಾಲಿಗಳು(&pq->queues[pos.ಸ್ಥಾನಾಂತರ_prio]))) {
		pq->bitmask[pos.idx] &= ~ಬಿಟ್(pos.ಬಿಟ್_2);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_SMP
		pq->cached_ಸರತಿ_ಸೂಚ್ಯಂಕ = z_priq_mq_ಉತ್ತಮ_ಸರತಿ_ಸೂಚ್ಯಂಕ(pq);
#ಆದರೆ_ಅಂತ್ಯ
	}
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ z_priq_mq_yield(ರಚನೆ _priq_mq *pq)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_SMP
	ರಚನೆ prio_info pos = get_prio_info(_current->ತಳ.prio);

	sys_dlist_ಸರತಿಯಿಂದ_ತೆಗೆ(&_current->ತಳ.qnode_dlist);
	sys_dlist_append(&pq->queues[pos.ಸ್ಥಾನಾಂತರ_prio],
			 &_current->ತಳ.qnode_dlist);
#ಆದರೆ_ಅಂತ್ಯ
}

ಸ್ಥಿರ ALWAYS_INLINE ರಚನೆ k_thread *z_priq_mq_ಉತ್ತಮ(ರಚನೆ _priq_mq *pq)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ = z_priq_mq_ಉತ್ತಮ_ಸರತಿ_ಸೂಚ್ಯಂಕ(pq);
#ಇಲ್ಲದಿದ್ದರೆ
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ = pq->cached_ಸರತಿ_ಸೂಚ್ಯಂಕ;
#ಆದರೆ_ಅಂತ್ಯ

	sys_dnode_t *n = sys_dlist_peek_ತಲೆ(&pq->queues[ಸೂಚ್ಯಂಕ]);

	ಆದರೆ (likely(n != ಖಾಲಿ)) {
		ಹಿಂತಿರುಗಿ CONTAINER_ದ(n, ರಚನೆ k_thread, ತಳ.qnode_dlist);
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_CPU_ಮುಖವಾಡ
ಸ್ಥಿರ ALWAYS_INLINE ರಚನೆ k_thread *z_priq_mq_ಮುಖವಾಡ_ಉತ್ತಮ(ರಚನೆ _priq_mq *pq)
{
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ current_cpu_ಬಿಟ್ = ಬಿಟ್(_current_cpu->id);

	/* Walk priority levels from highest to lowest, and within each
	 * level walk the thread list to find the first thread runnable
	 * on the current CPU.
	 */
	ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i = 0U; i < PRIQ_BITMAP_ಗಾತ್ರ; i++) {
		ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ bitmask = pq->bitmask[i];

		ಆಗ (bitmask != 0UL) {
			ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಬಿಟ್_2 = TRAILING_ZEROS(bitmask);
			ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ = i * NBITS + ಬಿಟ್_2;
			ರಚನೆ k_thread *thread;

			SYS_DLIST_FOR_EACH_CONTAINER(&pq->queues[ಸೂಚ್ಯಂಕ],
						     thread, ತಳ.qnode_dlist) {
				ಆದರೆ ((thread->ತಳ.cpu_ಮುಖವಾಡ &
				     current_cpu_ಬಿಟ್) != 0) {
					ಹಿಂತಿರುಗಿ thread;
				}
			}
			bitmask &= (bitmask - 1UL);
		}
	}
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_CPU_MASK */

#ಆದರೆ_ಅಂತ್ಯ /* ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_ */
