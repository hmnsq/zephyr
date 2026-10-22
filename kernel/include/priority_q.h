/*
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अगर_अघोषित ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_
#घोषणा ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_

#समावेश <zephyr/sys/math_extras.h>
#समावेश <zephyr/sys/dlist.h>

/* Simple Scheduling */
#अगर defined(CONFIG_SCHED_SIMPLE)
#घोषणा _priq_run_init		z_priq_simple_init
#घोषणा _priq_run_add		z_priq_simple_add
#घोषणा _priq_run_remove	z_priq_simple_remove
#घोषणा _priq_run_yield         z_priq_simple_yield
# अगर defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_simple_mask_best
# अन्यथा
#  define _priq_run_best	z_priq_simple_best
# endif /* CONFIG_SCHED_CPU_MASK */
/* Scalable Scheduling */
#या_अगर defined(CONFIG_SCHED_SCALABLE)
#घोषणा _priq_run_init		z_priq_rb_init
#घोषणा _priq_run_add		z_priq_rb_add
#घोषणा _priq_run_remove	z_priq_rb_remove
#घोषणा _priq_run_yield         z_priq_rb_yield
# अगर defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_rb_mask_best
# अन्यथा
#  define _priq_run_best	z_priq_rb_best
# endif /* CONFIG_SCHED_CPU_MASK */
 /* Multi Queue Scheduling */
#या_अगर defined(CONFIG_SCHED_MULTIQ)
#घोषणा _priq_run_init		z_priq_mq_init
#घोषणा _priq_run_add		z_priq_mq_add
#घोषणा _priq_run_remove	z_priq_mq_remove
#घोषणा _priq_run_yield         z_priq_mq_yield
# अगर defined(CONFIG_SCHED_CPU_MASK) && !defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
#  define _priq_run_best	z_priq_mq_mask_best
# अन्यथा
#  define _priq_run_best	z_priq_mq_best
# endif /* CONFIG_SCHED_CPU_MASK */
#पूर्ण_अगर

/* Scalable Wait Queue */
#अगर defined(CONFIG_WAITQ_SCALABLE)
#घोषणा _priq_wait_add		z_priq_rb_add
#घोषणा _priq_wait_remove	z_priq_rb_remove
#घोषणा _priq_wait_best		z_priq_rb_best
/* Simple Wait Queue */
#या_अगर defined(CONFIG_WAITQ_SIMPLE)
#घोषणा _priq_wait_add		z_priq_simple_add
#घोषणा _priq_wait_remove	z_priq_simple_remove
#घोषणा _priq_wait_best		z_priq_simple_best
#पूर्ण_अगर

#अगर defined(CONFIG_64BIT)
#घोषणा NBITS          64
#घोषणा TRAILING_ZEROS u64_गिनती_trailing_zeros
#अन्यथा
#घोषणा NBITS          32
#घोषणा TRAILING_ZEROS u32_गिनती_trailing_zeros
#पूर्ण_अगर /* CONFIG_64BIT */

अटल ALWAYS_INLINE व्योम z_priq_simple_init(sys_dlist_t *pq)
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
अटल ALWAYS_INLINE int32_t z_sched_prio_cmp(काष्ठा k_thread *thread_1, काष्ठा k_thread *thread_2)
{
	/* `prio` is <32b, so the below cannot overflow. */
	int32_t b1 = thread_1->base.prio;
	int32_t b2 = thread_2->base.prio;

	अगर (b1 != b2) {
		वापस b2 - b1;
	}

#अगर_घोषित CONFIG_SCHED_DEADLINE
	/* If we assume all deadlines live within the same "half" of
	 * the 32 bit modulus space (this is a documented API rule),
	 * then the latest deadline in the queue minus the earliest is
	 * guaranteed to be (2's complement) non-negative.  We can
	 * leverage that to compare the values without having to check
	 * the current time.
	 */
	uint32_t d1 = thread_1->base.prio_deadline;
	uint32_t d2 = thread_2->base.prio_deadline;

	अगर (d1 != d2) {
		/* Sooner deadline means higher effective priority.
		 * Doing the calculation with unsigned types and casting
		 * to signed isn't perfect, but at least reduces this
		 * from UB on overflow to impdef.
		 */
		वापस (int32_t)(d2 - d1);
	}
#पूर्ण_अगर /* CONFIG_SCHED_DEADLINE */
	वापस 0;
}

अटल ALWAYS_INLINE व्योम z_priq_simple_add(sys_dlist_t *pq, काष्ठा k_thread *thread)
{
	काष्ठा k_thread *t;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, t, base.qnode_dlist) {
		अगर (z_sched_prio_cmp(thread, t) > 0) {
			sys_dlist_जोड़ो(&t->base.qnode_dlist, &thread->base.qnode_dlist);
			वापस;
		}
	}

	sys_dlist_append(pq, &thread->base.qnode_dlist);
}

अटल ALWAYS_INLINE व्योम z_priq_simple_remove(sys_dlist_t *pq, काष्ठा k_thread *thread)
{
	ARG_UNUSED(pq);

	sys_dlist_remove(&thread->base.qnode_dlist);
}

अटल ALWAYS_INLINE व्योम z_priq_simple_yield(sys_dlist_t *pq)
{
#अगर_अघोषित CONFIG_SMP
	sys_dnode_t *n;

	n = sys_dlist_peek_अग्र_no_check(pq, &_current->base.qnode_dlist);

	sys_dlist_dequeue(&_current->base.qnode_dlist);

	काष्ठा k_thread *t;

	/*
	 * As it is possible that the current thread was not at the head of
	 * the run queue, start searching from the present position for where
	 * to re-insert it.
	 */

	जबतक (n != शून्य) {
		t = CONTAINER_OF(n, काष्ठा k_thread, base.qnode_dlist);
		अगर (z_sched_prio_cmp(_current, t) > 0) {
			sys_dlist_जोड़ो(&t->base.qnode_dlist,
					 &_current->base.qnode_dlist);
			वापस;
		}
		n = sys_dlist_peek_अग्र_no_check(pq, n);
	}

	sys_dlist_append(pq, &_current->base.qnode_dlist);
#पूर्ण_अगर
}

अटल ALWAYS_INLINE काष्ठा k_thread *z_priq_simple_best(sys_dlist_t *pq)
{
	काष्ठा k_thread *thread = शून्य;
	sys_dnode_t *n = sys_dlist_peek_शीर्ष(pq);

	अगर (n != शून्य) {
		thread = CONTAINER_OF(n, काष्ठा k_thread, base.qnode_dlist);
	}
	वापस thread;
}

#अगर_घोषित CONFIG_SCHED_CPU_MASK
अटल ALWAYS_INLINE काष्ठा k_thread *z_priq_simple_mask_best(sys_dlist_t *pq)
{
	/* With masks enabled we need to be prepared to walk the list
	 * looking for one we can run
	 */
	अचिन्हित दीर्घ current_cpu_bit = BIT(_current_cpu->id);
	काष्ठा k_thread *thread;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, thread, base.qnode_dlist) {
		अगर ((thread->base.cpu_mask & current_cpu_bit) != 0) {
			वापस thread;
		}
	}
	वापस शून्य;
}
#पूर्ण_अगर /* CONFIG_SCHED_CPU_MASK */

#अगर defined(CONFIG_SCHED_SCALABLE) || defined(CONFIG_WAITQ_SCALABLE)
अटल ALWAYS_INLINE व्योम z_priq_rb_init(काष्ठा _priq_rb *pq)
{
	bool z_priq_rb_lessthan(काष्ठा rbnode *a, काष्ठा rbnode *b);

	*pq = (काष्ठा _priq_rb) {
		.वृक्ष = {
			.lessthan_fn = z_priq_rb_lessthan,
		}
	};
}

अटल ALWAYS_INLINE व्योम z_priq_rb_add(काष्ठा _priq_rb *pq, काष्ठा k_thread *thread)
{
	काष्ठा k_thread *t;

	thread->base.order_कुंजी = pq->अग्र_order_कुंजी;
	++pq->अग्र_order_कुंजी;

	/* Renumber at wraparound.  This is tiny code, and in practice
	 * will almost never be hit on real systems.  BUT on very
	 * long-running systems where a priq never completely empties
	 * AND that contains very large numbers of threads, it can be
	 * a latency glitch to loop over all the threads like this.
	 */
	अगर (!pq->अग्र_order_कुंजी) {
		RB_FOR_EACH_CONTAINER(&pq->वृक्ष, t, base.qnode_rb) {
			t->base.order_कुंजी = pq->अग्र_order_कुंजी;
			++pq->अग्र_order_कुंजी;
		}
	}

	rb_जोड़ो(&pq->वृक्ष, &thread->base.qnode_rb);
}

अटल ALWAYS_INLINE व्योम z_priq_rb_remove(काष्ठा _priq_rb *pq, काष्ठा k_thread *thread)
{
	rb_remove(&pq->वृक्ष, &thread->base.qnode_rb);

	अगर (!pq->वृक्ष.मूल) {
		pq->अग्र_order_कुंजी = 0;
	}
}

अटल ALWAYS_INLINE व्योम z_priq_rb_yield(काष्ठा _priq_rb *pq)
{
#अगर_अघोषित CONFIG_SMP
	z_priq_rb_remove(pq, _current);
	z_priq_rb_add(pq, _current);
#पूर्ण_अगर
}

अटल ALWAYS_INLINE काष्ठा k_thread *z_priq_rb_best(काष्ठा _priq_rb *pq)
{
	काष्ठा k_thread *thread = शून्य;
	काष्ठा rbnode *n = rb_get_min(&pq->वृक्ष);

	अगर (n != शून्य) {
		thread = CONTAINER_OF(n, काष्ठा k_thread, base.qnode_rb);
	}
	वापस thread;
}

#अगर_घोषित CONFIG_SCHED_CPU_MASK
अटल ALWAYS_INLINE काष्ठा k_thread *z_priq_rb_mask_best(काष्ठा _priq_rb *pq)
{
	अचिन्हित दीर्घ current_cpu_bit = BIT(_current_cpu->id);
	काष्ठा k_thread *thread;

	RB_FOR_EACH_CONTAINER(&pq->वृक्ष, thread, base.qnode_rb) {
		अगर ((thread->base.cpu_mask & current_cpu_bit) != 0) {
			वापस thread;
		}
	}
	वापस शून्य;
}
#पूर्ण_अगर /* CONFIG_SCHED_CPU_MASK */
#पूर्ण_अगर /* CONFIG_SCHED_SCALABLE || CONFIG_WAITQ_SCALABLE */

काष्ठा prio_info {
	uint8_t offset_prio;
	uint8_t idx;
	uint8_t bit;
};

अटल ALWAYS_INLINE काष्ठा prio_info get_prio_info(int8_t old_prio)
{
	काष्ठा prio_info ret;

	ret.offset_prio = old_prio - K_HIGHEST_THREAD_PRIO;
	ret.idx = ret.offset_prio / NBITS;
	ret.bit = ret.offset_prio % NBITS;

	वापस ret;
}

अटल ALWAYS_INLINE अचिन्हित पूर्णांक z_priq_mq_best_queue_सूचकांक(काष्ठा _priq_mq *pq)
{
	अचिन्हित पूर्णांक i = 0;

	करो {
		अगर (likely(pq->bitmask[i])) {
			वापस i * NBITS + TRAILING_ZEROS(pq->bitmask[i]);
		}
		i++;
	} जबतक (i < PRIQ_BITMAP_आकार);

	वापस K_अंक_THREAD_PRIO - 1;
}

अटल ALWAYS_INLINE व्योम z_priq_mq_init(काष्ठा _priq_mq *q)
{
	क्रम (माप_प्रकार i = 0; i < ARRAY_आकार(q->queues); i++) {
		sys_dlist_init(&q->queues[i]);
	}

#अगर_अघोषित CONFIG_SMP
	q->cached_queue_सूचकांक = K_अंक_THREAD_PRIO - 1;
#पूर्ण_अगर
}

अटल ALWAYS_INLINE व्योम z_priq_mq_add(काष्ठा _priq_mq *pq,
					काष्ठा k_thread *thread)
{
	काष्ठा prio_info pos = get_prio_info(thread->base.prio);

	sys_dlist_append(&pq->queues[pos.offset_prio], &thread->base.qnode_dlist);
	pq->bitmask[pos.idx] |= BIT(pos.bit);

#अगर_अघोषित CONFIG_SMP
	अगर (pos.offset_prio < pq->cached_queue_सूचकांक) {
		pq->cached_queue_सूचकांक = pos.offset_prio;
	}
#पूर्ण_अगर
}

अटल ALWAYS_INLINE व्योम z_priq_mq_remove(काष्ठा _priq_mq *pq,
					   काष्ठा k_thread *thread)
{
	काष्ठा prio_info pos = get_prio_info(thread->base.prio);

	sys_dlist_dequeue(&thread->base.qnode_dlist);
	अगर (unlikely(sys_dlist_is_empty(&pq->queues[pos.offset_prio]))) {
		pq->bitmask[pos.idx] &= ~BIT(pos.bit);
#अगर_अघोषित CONFIG_SMP
		pq->cached_queue_सूचकांक = z_priq_mq_best_queue_सूचकांक(pq);
#पूर्ण_अगर
	}
}

अटल ALWAYS_INLINE व्योम z_priq_mq_yield(काष्ठा _priq_mq *pq)
{
#अगर_अघोषित CONFIG_SMP
	काष्ठा prio_info pos = get_prio_info(_current->base.prio);

	sys_dlist_dequeue(&_current->base.qnode_dlist);
	sys_dlist_append(&pq->queues[pos.offset_prio],
			 &_current->base.qnode_dlist);
#पूर्ण_अगर
}

अटल ALWAYS_INLINE काष्ठा k_thread *z_priq_mq_best(काष्ठा _priq_mq *pq)
{
#अगर_घोषित CONFIG_SMP
	अचिन्हित पूर्णांक सूचकांक = z_priq_mq_best_queue_सूचकांक(pq);
#अन्यथा
	अचिन्हित पूर्णांक सूचकांक = pq->cached_queue_सूचकांक;
#पूर्ण_अगर

	sys_dnode_t *n = sys_dlist_peek_शीर्ष(&pq->queues[सूचकांक]);

	अगर (likely(n != शून्य)) {
		वापस CONTAINER_OF(n, काष्ठा k_thread, base.qnode_dlist);
	}

	वापस शून्य;
}

#अगर_घोषित CONFIG_SCHED_CPU_MASK
अटल ALWAYS_INLINE काष्ठा k_thread *z_priq_mq_mask_best(काष्ठा _priq_mq *pq)
{
	अचिन्हित दीर्घ current_cpu_bit = BIT(_current_cpu->id);

	/* Walk priority levels from highest to lowest, and within each
	 * level walk the thread list to find the first thread runnable
	 * on the current CPU.
	 */
	क्रम (अचिन्हित पूर्णांक i = 0U; i < PRIQ_BITMAP_आकार; i++) {
		अचिन्हित दीर्घ bitmask = pq->bitmask[i];

		जबतक (bitmask != 0UL) {
			अचिन्हित पूर्णांक bit = TRAILING_ZEROS(bitmask);
			अचिन्हित पूर्णांक सूचकांक = i * NBITS + bit;
			काष्ठा k_thread *thread;

			SYS_DLIST_FOR_EACH_CONTAINER(&pq->queues[सूचकांक],
						     thread, base.qnode_dlist) {
				अगर ((thread->base.cpu_mask &
				     current_cpu_bit) != 0) {
					वापस thread;
				}
			}
			bitmask &= (bitmask - 1UL);
		}
	}
	वापस शून्य;
}
#पूर्ण_अगर /* CONFIG_SCHED_CPU_MASK */

#पूर्ण_अगर /* ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_ */
