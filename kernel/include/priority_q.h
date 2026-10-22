/*
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#यदि_अपरिभाषितम् ZEPHYR_KERNEL_INCLUDE_प्राथमिकता_Q_H
#परिभाषय ZEPHYR_KERNEL_INCLUDE_प्राथमिकता_Q_H

#अन्तर्भाव <zephyr/sys/math_extras.h>
#अन्तर्भाव <zephyr/sys/dlist.h>

/* Simple Scheduling */
#यदि defined(CONFIG_SCHED_SIMPLE)
#परिभाषय priq_सततम्_init		z_priq_simple_init
#परिभाषय priq_सततम्_सङ्कलय		z_priq_simple_सङ्कलय
#परिभाषय priq_सततम्_remove	z_priq_simple_remove
#परिभाषय priq_सततम्_yield         z_priq_simple_yield
# यदि defined(CONFIG_SCHED_CPU_आवरणम्) && !defined(CONFIG_SCHED_CPU_आवरणम्_PIN_ONLY)
#  define priq_सततम्_best	z_priq_simple_आवरणम्_best
# अन्यथा
#  define priq_सततम्_best	z_priq_simple_best
# endif /* CONFIG_SCHED_CPU_MASK */
/* Scalable Scheduling */
#नोचेद्यदि defined(CONFIG_SCHED_SCALABLE)
#परिभाषय priq_सततम्_init		z_priq_rb_init
#परिभाषय priq_सततम्_सङ्कलय		z_priq_rb_सङ्कलय
#परिभाषय priq_सततम्_remove	z_priq_rb_remove
#परिभाषय priq_सततम्_yield         z_priq_rb_yield
# यदि defined(CONFIG_SCHED_CPU_आवरणम्) && !defined(CONFIG_SCHED_CPU_आवरणम्_PIN_ONLY)
#  define priq_सततम्_best	z_priq_rb_आवरणम्_best
# अन्यथा
#  define priq_सततम्_best	z_priq_rb_best
# endif /* CONFIG_SCHED_CPU_MASK */
 /* Multi Queue Scheduling */
#नोचेद्यदि defined(CONFIG_SCHED_MULTIQ)
#परिभाषय priq_सततम्_init		z_priq_mq_init
#परिभाषय priq_सततम्_सङ्कलय		z_priq_mq_सङ्कलय
#परिभाषय priq_सततम्_remove	z_priq_mq_remove
#परिभाषय priq_सततम्_yield         z_priq_mq_yield
# यदि defined(CONFIG_SCHED_CPU_आवरणम्) && !defined(CONFIG_SCHED_CPU_आवरणम्_PIN_ONLY)
#  define priq_सततम्_best	z_priq_mq_आवरणम्_best
# अन्यथा
#  define priq_सततम्_best	z_priq_mq_best
# endif /* CONFIG_SCHED_CPU_MASK */
#यदि_अन्तः

/* Scalable Wait Queue */
#यदि defined(CONFIG_WAITQ_SCALABLE)
#परिभाषय priq_wait_सङ्कलय		z_priq_rb_सङ्कलय
#परिभाषय _priq_wait_remove	z_priq_rb_remove
#परिभाषय _priq_wait_best		z_priq_rb_best
/* Simple Wait Queue */
#नोचेद्यदि defined(CONFIG_WAITQ_SIMPLE)
#परिभाषय priq_wait_सङ्कलय		z_priq_simple_सङ्कलय
#परिभाषय _priq_wait_remove	z_priq_simple_remove
#परिभाषय _priq_wait_best		z_priq_simple_best
#यदि_अन्तः

#यदि defined(CONFIG_64BIT)
#परिभाषय NBITS          64
#परिभाषय TRAILING_ZEROS u64_गणनम्_trailing_zeros
#अन्यथा
#परिभाषय NBITS          32
#परिभाषय TRAILING_ZEROS u32_गणनम्_trailing_zeros
#यदि_अन्तः /* CONFIG_64BIT */

स्थिर ALWAYS_INLINE शून्य z_priq_simple_init(sys_dlist_t *pq)
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
स्थिर ALWAYS_INLINE int32_t z_sched_prio_cmp(संरचना k_thread *thread_1, संरचना k_thread *thread_2)
{
	/* `prio` is <32b, so the below cannot overflow. */
	int32_t b1 = thread_1->आधारः.prio;
	int32_t b2 = thread_2->आधारः.prio;

	यदि (b1 != b2) {
		प्रत्यागम b2 - b1;
	}

#यदि_परिभाषितम् CONFIG_SCHED_DEADLINE
	/* If we assume all deadlines live within the same "half" of
	 * the 32 bit modulus space (this is a documented API rule),
	 * then the latest deadline in the queue minus the earliest is
	 * guaranteed to be (2's complement) non-negative.  We can
	 * leverage that to compare the values without having to check
	 * the current time.
	 */
	uint32_t d1 = thread_1->आधारः.prio_deadline;
	uint32_t d2 = thread_2->आधारः.prio_deadline;

	यदि (d1 != d2) {
		/* Sooner deadline means higher effective priority.
		 * Doing the calculation with unsigned types and casting
		 * to signed isn't perfect, but at least reduces this
		 * from UB on overflow to impdef.
		 */
		प्रत्यागम (int32_t)(d2 - d1);
	}
#यदि_अन्तः /* CONFIG_SCHED_DEADLINE */
	प्रत्यागम 0;
}

स्थिर ALWAYS_INLINE शून्य z_priq_simple_सङ्कलय(sys_dlist_t *pq, संरचना k_thread *thread)
{
	संरचना k_thread *t;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, t, आधारः.qnode_dlist) {
		यदि (z_sched_prio_cmp(thread, t) > 0) {
			sys_dlist_निवेशय(&t->आधारः.qnode_dlist, &thread->आधारः.qnode_dlist);
			प्रत्यागम;
		}
	}

	sys_dlist_append(pq, &thread->आधारः.qnode_dlist);
}

स्थिर ALWAYS_INLINE शून्य z_priq_simple_remove(sys_dlist_t *pq, संरचना k_thread *thread)
{
	ARG_अप्रयुक्तम्(pq);

	sys_dlist_remove(&thread->आधारः.qnode_dlist);
}

स्थिर ALWAYS_INLINE शून्य z_priq_simple_yield(sys_dlist_t *pq)
{
#यदि_अपरिभाषितम् CONFIG_SMP
	sys_dnode_t *n;

	n = sys_dlist_peek_अग्र_no_check(pq, &_current->आधारः.qnode_dlist);

	sys_dlist_पङ्क्तेः_निष्कासय(&_current->आधारः.qnode_dlist);

	संरचना k_thread *t;

	/*
	 * As it is possible that the current thread was not at the head of
	 * the run queue, start searching from the present position for where
	 * to re-insert it.
	 */

	यावत् (n != रिक्त) {
		t = CONTAINER_सम्बन्धि(n, संरचना k_thread, आधारः.qnode_dlist);
		यदि (z_sched_prio_cmp(_current, t) > 0) {
			sys_dlist_निवेशय(&t->आधारः.qnode_dlist,
					 &_current->आधारः.qnode_dlist);
			प्रत्यागम;
		}
		n = sys_dlist_peek_अग्र_no_check(pq, n);
	}

	sys_dlist_append(pq, &_current->आधारः.qnode_dlist);
#यदि_अन्तः
}

स्थिर ALWAYS_INLINE संरचना k_thread *z_priq_simple_best(sys_dlist_t *pq)
{
	संरचना k_thread *thread = रिक्त;
	sys_dnode_t *n = sys_dlist_peek_शीर्ष(pq);

	यदि (n != रिक्त) {
		thread = CONTAINER_सम्बन्धि(n, संरचना k_thread, आधारः.qnode_dlist);
	}
	प्रत्यागम thread;
}

#यदि_परिभाषितम् CONFIG_SCHED_CPU_आवरणम्
स्थिर ALWAYS_INLINE संरचना k_thread *z_priq_simple_आवरणम्_best(sys_dlist_t *pq)
{
	/* With masks enabled we need to be prepared to walk the list
	 * looking for one we can run
	 */
	अचिह्नित दीर्घ current_cpu_bit = BIT(_current_cpu->id);
	संरचना k_thread *thread;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, thread, आधारः.qnode_dlist) {
		यदि ((thread->आधारः.cpu_आवरणम् & current_cpu_bit) != 0) {
			प्रत्यागम thread;
		}
	}
	प्रत्यागम रिक्त;
}
#यदि_अन्तः /* CONFIG_SCHED_CPU_MASK */

#यदि defined(CONFIG_SCHED_SCALABLE) || defined(CONFIG_WAITQ_SCALABLE)
स्थिर ALWAYS_INLINE शून्य z_priq_rb_init(संरचना _priq_rb *pq)
{
	bool z_priq_rb_lessthan(संरचना rbnode *a, संरचना rbnode *b);

	*pq = (संरचना _priq_rb) {
		.वृक्षः = {
			.lessthan_fn = z_priq_rb_lessthan,
		}
	};
}

स्थिर ALWAYS_INLINE शून्य z_priq_rb_सङ्कलय(संरचना _priq_rb *pq, संरचना k_thread *thread)
{
	संरचना k_thread *t;

	thread->आधारः.क्रमः_कुञ्चिका = pq->अग्र_क्रमः_कुञ्चिका;
	++pq->अग्र_क्रमः_कुञ्चिका;

	/* Renumber at wraparound.  This is tiny code, and in practice
	 * will almost never be hit on real systems.  BUT on very
	 * long-running systems where a priq never completely empties
	 * AND that contains very large numbers of threads, it can be
	 * a latency glitch to loop over all the threads like this.
	 */
	यदि (!pq->अग्र_क्रमः_कुञ्चिका) {
		RB_FOR_EACH_CONTAINER(&pq->वृक्षः, t, आधारः.qnode_rb) {
			t->आधारः.क्रमः_कुञ्चिका = pq->अग्र_क्रमः_कुञ्चिका;
			++pq->अग्र_क्रमः_कुञ्चिका;
		}
	}

	rb_निवेशय(&pq->वृक्षः, &thread->आधारः.qnode_rb);
}

स्थिर ALWAYS_INLINE शून्य z_priq_rb_remove(संरचना _priq_rb *pq, संरचना k_thread *thread)
{
	rb_remove(&pq->वृक्षः, &thread->आधारः.qnode_rb);

	यदि (!pq->वृक्षः.मूलम्) {
		pq->अग्र_क्रमः_कुञ्चिका = 0;
	}
}

स्थिर ALWAYS_INLINE शून्य z_priq_rb_yield(संरचना _priq_rb *pq)
{
#यदि_अपरिभाषितम् CONFIG_SMP
	z_priq_rb_remove(pq, _current);
	z_priq_rb_सङ्कलय(pq, _current);
#यदि_अन्तः
}

स्थिर ALWAYS_INLINE संरचना k_thread *z_priq_rb_best(संरचना _priq_rb *pq)
{
	संरचना k_thread *thread = रिक्त;
	संरचना rbnode *n = rb_get_min(&pq->वृक्षः);

	यदि (n != रिक्त) {
		thread = CONTAINER_सम्बन्धि(n, संरचना k_thread, आधारः.qnode_rb);
	}
	प्रत्यागम thread;
}

#यदि_परिभाषितम् CONFIG_SCHED_CPU_आवरणम्
स्थिर ALWAYS_INLINE संरचना k_thread *z_priq_rb_आवरणम्_best(संरचना _priq_rb *pq)
{
	अचिह्नित दीर्घ current_cpu_bit = BIT(_current_cpu->id);
	संरचना k_thread *thread;

	RB_FOR_EACH_CONTAINER(&pq->वृक्षः, thread, आधारः.qnode_rb) {
		यदि ((thread->आधारः.cpu_आवरणम् & current_cpu_bit) != 0) {
			प्रत्यागम thread;
		}
	}
	प्रत्यागम रिक्त;
}
#यदि_अन्तः /* CONFIG_SCHED_CPU_MASK */
#यदि_अन्तः /* CONFIG_SCHED_SCALABLE || CONFIG_WAITQ_SCALABLE */

संरचना prio_info {
	uint8_t विस्थापनम्_prio;
	uint8_t idx;
	uint8_t bit;
};

स्थिर ALWAYS_INLINE संरचना prio_info get_prio_info(int8_t old_prio)
{
	संरचना prio_info ret;

	ret.विस्थापनम्_prio = old_prio - K_HIGHEST_THREAD_PRIO;
	ret.idx = ret.विस्थापनम्_prio / NBITS;
	ret.bit = ret.विस्थापनम्_prio % NBITS;

	प्रत्यागम ret;
}

स्थिर ALWAYS_INLINE अचिह्नित पूर्णाङ्क z_priq_mq_best_queue_सूचकांक(संरचना _priq_mq *pq)
{
	अचिह्नित पूर्णाङ्क i = 0;

	कुरु {
		यदि (likely(pq->bitmask[i])) {
			प्रत्यागम i * NBITS + TRAILING_ZEROS(pq->bitmask[i]);
		}
		i++;
	} यावत् (i < PRIQ_BITMAP_आकार);

	प्रत्यागम K_अङ्कः_THREAD_PRIO - 1;
}

स्थिर ALWAYS_INLINE शून्य z_priq_mq_init(संरचना _priq_mq *q)
{
	प्रति (size_t i = 0; i < ARRAY_आकार(q->queues); i++) {
		sys_dlist_init(&q->queues[i]);
	}

#यदि_अपरिभाषितम् CONFIG_SMP
	q->cached_queue_सूचकांक = K_अङ्कः_THREAD_PRIO - 1;
#यदि_अन्तः
}

स्थिर ALWAYS_INLINE शून्य z_priq_mq_सङ्कलय(संरचना _priq_mq *pq,
					संरचना k_thread *thread)
{
	संरचना prio_info pos = get_prio_info(thread->आधारः.prio);

	sys_dlist_append(&pq->queues[pos.विस्थापनम्_prio], &thread->आधारः.qnode_dlist);
	pq->bitmask[pos.idx] |= BIT(pos.bit);

#यदि_अपरिभाषितम् CONFIG_SMP
	यदि (pos.विस्थापनम्_prio < pq->cached_queue_सूचकांक) {
		pq->cached_queue_सूचकांक = pos.विस्थापनम्_prio;
	}
#यदि_अन्तः
}

स्थिर ALWAYS_INLINE शून्य z_priq_mq_remove(संरचना _priq_mq *pq,
					   संरचना k_thread *thread)
{
	संरचना prio_info pos = get_prio_info(thread->आधारः.prio);

	sys_dlist_पङ्क्तेः_निष्कासय(&thread->आधारः.qnode_dlist);
	यदि (unlikely(sys_dlist_is_रिक्तानि(&pq->queues[pos.विस्थापनम्_prio]))) {
		pq->bitmask[pos.idx] &= ~BIT(pos.bit);
#यदि_अपरिभाषितम् CONFIG_SMP
		pq->cached_queue_सूचकांक = z_priq_mq_best_queue_सूचकांक(pq);
#यदि_अन्तः
	}
}

स्थिर ALWAYS_INLINE शून्य z_priq_mq_yield(संरचना _priq_mq *pq)
{
#यदि_अपरिभाषितम् CONFIG_SMP
	संरचना prio_info pos = get_prio_info(_current->आधारः.prio);

	sys_dlist_पङ्क्तेः_निष्कासय(&_current->आधारः.qnode_dlist);
	sys_dlist_append(&pq->queues[pos.विस्थापनम्_prio],
			 &_current->आधारः.qnode_dlist);
#यदि_अन्तः
}

स्थिर ALWAYS_INLINE संरचना k_thread *z_priq_mq_best(संरचना _priq_mq *pq)
{
#यदि_परिभाषितम् CONFIG_SMP
	अचिह्नित पूर्णाङ्क सूचकांक = z_priq_mq_best_queue_सूचकांक(pq);
#अन्यथा
	अचिह्नित पूर्णाङ्क सूचकांक = pq->cached_queue_सूचकांक;
#यदि_अन्तः

	sys_dnode_t *n = sys_dlist_peek_शीर्ष(&pq->queues[सूचकांक]);

	यदि (likely(n != रिक्त)) {
		प्रत्यागम CONTAINER_सम्बन्धि(n, संरचना k_thread, आधारः.qnode_dlist);
	}

	प्रत्यागम रिक्त;
}

#यदि_परिभाषितम् CONFIG_SCHED_CPU_आवरणम्
स्थिर ALWAYS_INLINE संरचना k_thread *z_priq_mq_आवरणम्_best(संरचना _priq_mq *pq)
{
	अचिह्नित दीर्घ current_cpu_bit = BIT(_current_cpu->id);

	/* Walk priority levels from highest to lowest, and within each
	 * level walk the thread list to find the first thread runnable
	 * on the current CPU.
	 */
	प्रति (अचिह्नित पूर्णाङ्क i = 0U; i < PRIQ_BITMAP_आकार; i++) {
		अचिह्नित दीर्घ bitmask = pq->bitmask[i];

		यावत् (bitmask != 0UL) {
			अचिह्नित पूर्णाङ्क bit = TRAILING_ZEROS(bitmask);
			अचिह्नित पूर्णाङ्क सूचकांक = i * NBITS + bit;
			संरचना k_thread *thread;

			SYS_DLIST_FOR_EACH_CONTAINER(&pq->queues[सूचकांक],
						     thread, आधारः.qnode_dlist) {
				यदि ((thread->आधारः.cpu_आवरणम् &
				     current_cpu_bit) != 0) {
					प्रत्यागम thread;
				}
			}
			bitmask &= (bitmask - 1UL);
		}
	}
	प्रत्यागम रिक्त;
}
#यदि_अन्तः /* CONFIG_SCHED_CPU_MASK */

#यदि_अन्तः /* ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_ */
