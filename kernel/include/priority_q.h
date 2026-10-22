/*
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#నిర్వచితం_కాకపోతే ZEPHYR_KERNEL_INCLUDE_ప్రాధాన్యత_Q_H
#నిర్వచించు ZEPHYR_KERNEL_INCLUDE_ప్రాధాన్యత_Q_H

#అన్తర్భావ <zephyr/sys/math_extras.h>
#అన్తర్భావ <zephyr/sys/dlist.h>

/* Simple Scheduling */
#ఒకవేళ defined(CONFIG_SCHED_SIMPLE)
#నిర్వచించు priq_నిరంతరం_init		z_priq_simple_init
#నిర్వచించు priq_నిరంతరం_కూడు		z_priq_simple_కూడు
#నిర్వచించు priq_నిరంతరం_remove	z_priq_simple_remove
#నిర్వచించు priq_నిరంతరం_yield         z_priq_simple_yield
# ఒకవేళ defined(CONFIG_SCHED_CPU_ముసుగు) && !defined(CONFIG_SCHED_CPU_ముసుగు_PIN_ONLY)
#  define priq_నిరంతరం_ఉత్తమ	z_priq_simple_ముసుగు_ఉత్తమ
# లేకపోతే
#  define priq_నిరంతరం_ఉత్తమ	z_priq_simple_ఉత్తమ
# endif /* CONFIG_SCHED_CPU_MASK */
/* Scalable Scheduling */
#లేదావేళ defined(CONFIG_SCHED_SCALABLE)
#నిర్వచించు priq_నిరంతరం_init		z_priq_rb_init
#నిర్వచించు priq_నిరంతరం_కూడు		z_priq_rb_కూడు
#నిర్వచించు priq_నిరంతరం_remove	z_priq_rb_remove
#నిర్వచించు priq_నిరంతరం_yield         z_priq_rb_yield
# ఒకవేళ defined(CONFIG_SCHED_CPU_ముసుగు) && !defined(CONFIG_SCHED_CPU_ముసుగు_PIN_ONLY)
#  define priq_నిరంతరం_ఉత్తమ	z_priq_rb_ముసుగు_ఉత్తమ
# లేకపోతే
#  define priq_నిరంతరం_ఉత్తమ	z_priq_rb_ఉత్తమ
# endif /* CONFIG_SCHED_CPU_MASK */
 /* Multi Queue Scheduling */
#లేదావేళ defined(CONFIG_SCHED_MULTIQ)
#నిర్వచించు priq_నిరంతరం_init		z_priq_mq_init
#నిర్వచించు priq_నిరంతరం_కూడు		z_priq_mq_కూడు
#నిర్వచించు priq_నిరంతరం_remove	z_priq_mq_remove
#నిర్వచించు priq_నిరంతరం_yield         z_priq_mq_yield
# ఒకవేళ defined(CONFIG_SCHED_CPU_ముసుగు) && !defined(CONFIG_SCHED_CPU_ముసుగు_PIN_ONLY)
#  define priq_నిరంతరం_ఉత్తమ	z_priq_mq_ముసుగు_ఉత్తమ
# లేకపోతే
#  define priq_నిరంతరం_ఉత్తమ	z_priq_mq_ఉత్తమ
# endif /* CONFIG_SCHED_CPU_MASK */
#అయితే_ముగింపు

/* Scalable Wait Queue */
#ఒకవేళ defined(CONFIG_WAITQ_SCALABLE)
#నిర్వచించు priq_wait_కూడు		z_priq_rb_కూడు
#నిర్వచించు _priq_wait_remove	z_priq_rb_remove
#నిర్వచించు priq_wait_ఉత్తమ		z_priq_rb_ఉత్తమ
/* Simple Wait Queue */
#లేదావేళ defined(CONFIG_WAITQ_SIMPLE)
#నిర్వచించు priq_wait_కూడు		z_priq_simple_కూడు
#నిర్వచించు _priq_wait_remove	z_priq_simple_remove
#నిర్వచించు priq_wait_ఉత్తమ		z_priq_simple_ఉత్తమ
#అయితే_ముగింపు

#ఒకవేళ defined(CONFIG_64_బిట్టు)
#నిర్వచించు NBITS          64
#నిర్వచించు TRAILING_ZEROS u64_లెక్క_trailing_zeros
#లేకపోతే
#నిర్వచించు NBITS          32
#నిర్వచించు TRAILING_ZEROS u32_లెక్క_trailing_zeros
#అయితే_ముగింపు /* CONFIG_64BIT */

స్థిర ALWAYS_INLINE శూన్య z_priq_simple_init(sys_dlist_t *pq)
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
స్థిర ALWAYS_INLINE int32_t z_sched_prio_cmp(నిర్మాణం k_thread *thread_1, నిర్మాణం k_thread *thread_2)
{
	/* `prio` is <32b, so the below cannot overflow. */
	int32_t b1 = thread_1->పునాది.prio;
	int32_t b2 = thread_2->పునాది.prio;

	ఒకవేళ (b1 != b2) {
		తిరిగి b2 - b1;
	}

#నిర్వచితమైతే CONFIG_SCHED_DEADLINE
	/* If we assume all deadlines live within the same "half" of
	 * the 32 bit modulus space (this is a documented API rule),
	 * then the latest deadline in the queue minus the earliest is
	 * guaranteed to be (2's complement) non-negative.  We can
	 * leverage that to compare the values without having to check
	 * the current time.
	 */
	uint32_t d1 = thread_1->పునాది.prio_deadline;
	uint32_t d2 = thread_2->పునాది.prio_deadline;

	ఒకవేళ (d1 != d2) {
		/* Sooner deadline means higher effective priority.
		 * Doing the calculation with unsigned types and casting
		 * to signed isn't perfect, but at least reduces this
		 * from UB on overflow to impdef.
		 */
		తిరిగి (int32_t)(d2 - d1);
	}
#అయితే_ముగింపు /* CONFIG_SCHED_DEADLINE */
	తిరిగి 0;
}

స్థిర ALWAYS_INLINE శూన్య z_priq_simple_కూడు(sys_dlist_t *pq, నిర్మాణం k_thread *thread)
{
	నిర్మాణం k_thread *t;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, t, పునాది.qnode_dlist) {
		ఒకవేళ (z_sched_prio_cmp(thread, t) > 0) {
			sys_dlist_చొప్పించు(&t->పునాది.qnode_dlist, &thread->పునాది.qnode_dlist);
			తిరిగి;
		}
	}

	sys_dlist_append(pq, &thread->పునాది.qnode_dlist);
}

స్థిర ALWAYS_INLINE శూన్య z_priq_simple_remove(sys_dlist_t *pq, నిర్మాణం k_thread *thread)
{
	ARG_వాడని(pq);

	sys_dlist_remove(&thread->పునాది.qnode_dlist);
}

స్థిర ALWAYS_INLINE శూన్య z_priq_simple_yield(sys_dlist_t *pq)
{
#నిర్వచితం_కాకపోతే CONFIG_SMP
	sys_dnode_t *n;

	n = sys_dlist_peek_తదుపరి_no_check(pq, &_current->పునాది.qnode_dlist);

	sys_dlist_వరుస_నుండి_తీయి(&_current->పునాది.qnode_dlist);

	నిర్మాణం k_thread *t;

	/*
	 * As it is possible that the current thread was not at the head of
	 * the run queue, start searching from the present position for where
	 * to re-insert it.
	 */

	అయితే (n != ఖాళీ) {
		t = CONTAINER_యొక్క(n, నిర్మాణం k_thread, పునాది.qnode_dlist);
		ఒకవేళ (z_sched_prio_cmp(_current, t) > 0) {
			sys_dlist_చొప్పించు(&t->పునాది.qnode_dlist,
					 &_current->పునాది.qnode_dlist);
			తిరిగి;
		}
		n = sys_dlist_peek_తదుపరి_no_check(pq, n);
	}

	sys_dlist_append(pq, &_current->పునాది.qnode_dlist);
#అయితే_ముగింపు
}

స్థిర ALWAYS_INLINE నిర్మాణం k_thread *z_priq_simple_ఉత్తమ(sys_dlist_t *pq)
{
	నిర్మాణం k_thread *thread = ఖాళీ;
	sys_dnode_t *n = sys_dlist_peek_తల(pq);

	ఒకవేళ (n != ఖాళీ) {
		thread = CONTAINER_యొక్క(n, నిర్మాణం k_thread, పునాది.qnode_dlist);
	}
	తిరిగి thread;
}

#నిర్వచితమైతే CONFIG_SCHED_CPU_ముసుగు
స్థిర ALWAYS_INLINE నిర్మాణం k_thread *z_priq_simple_ముసుగు_ఉత్తమ(sys_dlist_t *pq)
{
	/* With masks enabled we need to be prepared to walk the list
	 * looking for one we can run
	 */
	గుర్తులేని దీర్ఘ current_cpu_బిట్టు = బిట్టు(_current_cpu->id);
	నిర్మాణం k_thread *thread;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, thread, పునాది.qnode_dlist) {
		ఒకవేళ ((thread->పునాది.cpu_ముసుగు & current_cpu_బిట్టు) != 0) {
			తిరిగి thread;
		}
	}
	తిరిగి ఖాళీ;
}
#అయితే_ముగింపు /* CONFIG_SCHED_CPU_MASK */

#ఒకవేళ defined(CONFIG_SCHED_SCALABLE) || defined(CONFIG_WAITQ_SCALABLE)
స్థిర ALWAYS_INLINE శూన్య z_priq_rb_init(నిర్మాణం _priq_rb *pq)
{
	bool z_priq_rb_lessthan(నిర్మాణం rbnode *a, నిర్మాణం rbnode *b);

	*pq = (నిర్మాణం _priq_rb) {
		.వృక్షం = {
			.lessthan_fn = z_priq_rb_lessthan,
		}
	};
}

స్థిర ALWAYS_INLINE శూన్య z_priq_rb_కూడు(నిర్మాణం _priq_rb *pq, నిర్మాణం k_thread *thread)
{
	నిర్మాణం k_thread *t;

	thread->పునాది.క్రమం_కీలకం = pq->తదుపరి_క్రమం_కీలకం;
	++pq->తదుపరి_క్రమం_కీలకం;

	/* Renumber at wraparound.  This is tiny code, and in practice
	 * will almost never be hit on real systems.  BUT on very
	 * long-running systems where a priq never completely empties
	 * AND that contains very large numbers of threads, it can be
	 * a latency glitch to loop over all the threads like this.
	 */
	ఒకవేళ (!pq->తదుపరి_క్రమం_కీలకం) {
		RB_FOR_EACH_CONTAINER(&pq->వృక్షం, t, పునాది.qnode_rb) {
			t->పునాది.క్రమం_కీలకం = pq->తదుపరి_క్రమం_కీలకం;
			++pq->తదుపరి_క్రమం_కీలకం;
		}
	}

	rb_చొప్పించు(&pq->వృక్షం, &thread->పునాది.qnode_rb);
}

స్థిర ALWAYS_INLINE శూన్య z_priq_rb_remove(నిర్మాణం _priq_rb *pq, నిర్మాణం k_thread *thread)
{
	rb_remove(&pq->వృక్షం, &thread->పునాది.qnode_rb);

	ఒకవేళ (!pq->వృక్షం.మూలం) {
		pq->తదుపరి_క్రమం_కీలకం = 0;
	}
}

స్థిర ALWAYS_INLINE శూన్య z_priq_rb_yield(నిర్మాణం _priq_rb *pq)
{
#నిర్వచితం_కాకపోతే CONFIG_SMP
	z_priq_rb_remove(pq, _current);
	z_priq_rb_కూడు(pq, _current);
#అయితే_ముగింపు
}

స్థిర ALWAYS_INLINE నిర్మాణం k_thread *z_priq_rb_ఉత్తమ(నిర్మాణం _priq_rb *pq)
{
	నిర్మాణం k_thread *thread = ఖాళీ;
	నిర్మాణం rbnode *n = rb_get_min(&pq->వృక్షం);

	ఒకవేళ (n != ఖాళీ) {
		thread = CONTAINER_యొక్క(n, నిర్మాణం k_thread, పునాది.qnode_rb);
	}
	తిరిగి thread;
}

#నిర్వచితమైతే CONFIG_SCHED_CPU_ముసుగు
స్థిర ALWAYS_INLINE నిర్మాణం k_thread *z_priq_rb_ముసుగు_ఉత్తమ(నిర్మాణం _priq_rb *pq)
{
	గుర్తులేని దీర్ఘ current_cpu_బిట్టు = బిట్టు(_current_cpu->id);
	నిర్మాణం k_thread *thread;

	RB_FOR_EACH_CONTAINER(&pq->వృక్షం, thread, పునాది.qnode_rb) {
		ఒకవేళ ((thread->పునాది.cpu_ముసుగు & current_cpu_బిట్టు) != 0) {
			తిరిగి thread;
		}
	}
	తిరిగి ఖాళీ;
}
#అయితే_ముగింపు /* CONFIG_SCHED_CPU_MASK */
#అయితే_ముగింపు /* CONFIG_SCHED_SCALABLE || CONFIG_WAITQ_SCALABLE */

నిర్మాణం prio_info {
	uint8_t స్థానభ్రంశం_prio;
	uint8_t idx;
	uint8_t బిట్టు_2;
};

స్థిర ALWAYS_INLINE నిర్మాణం prio_info get_prio_info(int8_t old_prio)
{
	నిర్మాణం prio_info ret;

	ret.స్థానభ్రంశం_prio = old_prio - K_HIGHEST_THREAD_PRIO;
	ret.idx = ret.స్థానభ్రంశం_prio / NBITS;
	ret.బిట్టు_2 = ret.స్థానభ్రంశం_prio % NBITS;

	తిరిగి ret;
}

స్థిర ALWAYS_INLINE గుర్తులేని పూర్ణాంకం z_priq_mq_ఉత్తమ_వరుస_సూచీ(నిర్మాణం _priq_mq *pq)
{
	గుర్తులేని పూర్ణాంకం i = 0;

	చేయి {
		ఒకవేళ (likely(pq->bitmask[i])) {
			తిరిగి i * NBITS + TRAILING_ZEROS(pq->bitmask[i]);
		}
		i++;
	} అయితే (i < PRIQ_BITMAP_కొలత);

	తిరిగి K_అంకె_THREAD_PRIO - 1;
}

స్థిర ALWAYS_INLINE శూన్య z_priq_mq_init(నిర్మాణం _priq_mq *q)
{
	కొరకు (size_t i = 0; i < ARRAY_కొలత(q->queues); i++) {
		sys_dlist_init(&q->queues[i]);
	}

#నిర్వచితం_కాకపోతే CONFIG_SMP
	q->cached_వరుస_సూచీ = K_అంకె_THREAD_PRIO - 1;
#అయితే_ముగింపు
}

స్థిర ALWAYS_INLINE శూన్య z_priq_mq_కూడు(నిర్మాణం _priq_mq *pq,
					నిర్మాణం k_thread *thread)
{
	నిర్మాణం prio_info pos = get_prio_info(thread->పునాది.prio);

	sys_dlist_append(&pq->queues[pos.స్థానభ్రంశం_prio], &thread->పునాది.qnode_dlist);
	pq->bitmask[pos.idx] |= బిట్టు(pos.బిట్టు_2);

#నిర్వచితం_కాకపోతే CONFIG_SMP
	ఒకవేళ (pos.స్థానభ్రంశం_prio < pq->cached_వరుస_సూచీ) {
		pq->cached_వరుస_సూచీ = pos.స్థానభ్రంశం_prio;
	}
#అయితే_ముగింపు
}

స్థిర ALWAYS_INLINE శూన్య z_priq_mq_remove(నిర్మాణం _priq_mq *pq,
					   నిర్మాణం k_thread *thread)
{
	నిర్మాణం prio_info pos = get_prio_info(thread->పునాది.prio);

	sys_dlist_వరుస_నుండి_తీయి(&thread->పునాది.qnode_dlist);
	ఒకవేళ (unlikely(sys_dlist_is_ఖాళీలు(&pq->queues[pos.స్థానభ్రంశం_prio]))) {
		pq->bitmask[pos.idx] &= ~బిట్టు(pos.బిట్టు_2);
#నిర్వచితం_కాకపోతే CONFIG_SMP
		pq->cached_వరుస_సూచీ = z_priq_mq_ఉత్తమ_వరుస_సూచీ(pq);
#అయితే_ముగింపు
	}
}

స్థిర ALWAYS_INLINE శూన్య z_priq_mq_yield(నిర్మాణం _priq_mq *pq)
{
#నిర్వచితం_కాకపోతే CONFIG_SMP
	నిర్మాణం prio_info pos = get_prio_info(_current->పునాది.prio);

	sys_dlist_వరుస_నుండి_తీయి(&_current->పునాది.qnode_dlist);
	sys_dlist_append(&pq->queues[pos.స్థానభ్రంశం_prio],
			 &_current->పునాది.qnode_dlist);
#అయితే_ముగింపు
}

స్థిర ALWAYS_INLINE నిర్మాణం k_thread *z_priq_mq_ఉత్తమ(నిర్మాణం _priq_mq *pq)
{
#నిర్వచితమైతే CONFIG_SMP
	గుర్తులేని పూర్ణాంకం సూచీ = z_priq_mq_ఉత్తమ_వరుస_సూచీ(pq);
#లేకపోతే
	గుర్తులేని పూర్ణాంకం సూచీ = pq->cached_వరుస_సూచీ;
#అయితే_ముగింపు

	sys_dnode_t *n = sys_dlist_peek_తల(&pq->queues[సూచీ]);

	ఒకవేళ (likely(n != ఖాళీ)) {
		తిరిగి CONTAINER_యొక్క(n, నిర్మాణం k_thread, పునాది.qnode_dlist);
	}

	తిరిగి ఖాళీ;
}

#నిర్వచితమైతే CONFIG_SCHED_CPU_ముసుగు
స్థిర ALWAYS_INLINE నిర్మాణం k_thread *z_priq_mq_ముసుగు_ఉత్తమ(నిర్మాణం _priq_mq *pq)
{
	గుర్తులేని దీర్ఘ current_cpu_బిట్టు = బిట్టు(_current_cpu->id);

	/* Walk priority levels from highest to lowest, and within each
	 * level walk the thread list to find the first thread runnable
	 * on the current CPU.
	 */
	కొరకు (గుర్తులేని పూర్ణాంకం i = 0U; i < PRIQ_BITMAP_కొలత; i++) {
		గుర్తులేని దీర్ఘ bitmask = pq->bitmask[i];

		అయితే (bitmask != 0UL) {
			గుర్తులేని పూర్ణాంకం బిట్టు_2 = TRAILING_ZEROS(bitmask);
			గుర్తులేని పూర్ణాంకం సూచీ = i * NBITS + బిట్టు_2;
			నిర్మాణం k_thread *thread;

			SYS_DLIST_FOR_EACH_CONTAINER(&pq->queues[సూచీ],
						     thread, పునాది.qnode_dlist) {
				ఒకవేళ ((thread->పునాది.cpu_ముసుగు &
				     current_cpu_బిట్టు) != 0) {
					తిరిగి thread;
				}
			}
			bitmask &= (bitmask - 1UL);
		}
	}
	తిరిగి ఖాళీ;
}
#అయితే_ముగింపు /* CONFIG_SCHED_CPU_MASK */

#అయితే_ముగింపు /* ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_ */
