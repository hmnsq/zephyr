/*
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#اگر_غیر_تعریف_شدہ ZEPHYR_KERNEL_INCLUDE_ترجیح_Q_H
#تعریف_کرو ZEPHYR_KERNEL_INCLUDE_ترجیح_Q_H

#شامل <zephyr/sys/math_extras.h>
#شامل <zephyr/sys/dlist.h>

/* Simple Scheduling */
#اگر defined(CONFIG_SCHED_SIMPLE)
#تعریف_کرو priq_تسلسل_init		z_priq_simple_init
#تعریف_کرو priq_تسلسل_جوڑو		z_priq_simple_جوڑو
#تعریف_کرو priq_تسلسل_remove	z_priq_simple_remove
#تعریف_کرو priq_تسلسل_yield         z_priq_simple_yield
# اگر defined(CONFIG_SCHED_CPU_نقاب) && !defined(CONFIG_SCHED_CPU_نقاب_PIN_ONLY)
#  define priq_تسلسل_بہترین	z_priq_simple_نقاب_بہترین
# ورنہ
#  define priq_تسلسل_بہترین	z_priq_simple_بہترین
# endif /* CONFIG_SCHED_CPU_MASK */
/* Scalable Scheduling */
#وگرنہ_اگر defined(CONFIG_SCHED_SCALABLE)
#تعریف_کرو priq_تسلسل_init		z_priq_rb_init
#تعریف_کرو priq_تسلسل_جوڑو		z_priq_rb_جوڑو
#تعریف_کرو priq_تسلسل_remove	z_priq_rb_remove
#تعریف_کرو priq_تسلسل_yield         z_priq_rb_yield
# اگر defined(CONFIG_SCHED_CPU_نقاب) && !defined(CONFIG_SCHED_CPU_نقاب_PIN_ONLY)
#  define priq_تسلسل_بہترین	z_priq_rb_نقاب_بہترین
# ورنہ
#  define priq_تسلسل_بہترین	z_priq_rb_بہترین
# endif /* CONFIG_SCHED_CPU_MASK */
 /* Multi Queue Scheduling */
#وگرنہ_اگر defined(CONFIG_SCHED_MULTIQ)
#تعریف_کرو priq_تسلسل_init		z_priq_mq_init
#تعریف_کرو priq_تسلسل_جوڑو		z_priq_mq_جوڑو
#تعریف_کرو priq_تسلسل_remove	z_priq_mq_remove
#تعریف_کرو priq_تسلسل_yield         z_priq_mq_yield
# اگر defined(CONFIG_SCHED_CPU_نقاب) && !defined(CONFIG_SCHED_CPU_نقاب_PIN_ONLY)
#  define priq_تسلسل_بہترین	z_priq_mq_نقاب_بہترین
# ورنہ
#  define priq_تسلسل_بہترین	z_priq_mq_بہترین
# endif /* CONFIG_SCHED_CPU_MASK */
#اگر_ختم

/* Scalable Wait Queue */
#اگر defined(CONFIG_WAITQ_SCALABLE)
#تعریف_کرو priq_wait_جوڑو		z_priq_rb_جوڑو
#تعریف_کرو _priq_wait_remove	z_priq_rb_remove
#تعریف_کرو priq_wait_بہترین		z_priq_rb_بہترین
/* Simple Wait Queue */
#وگرنہ_اگر defined(CONFIG_WAITQ_SIMPLE)
#تعریف_کرو priq_wait_جوڑو		z_priq_simple_جوڑو
#تعریف_کرو _priq_wait_remove	z_priq_simple_remove
#تعریف_کرو priq_wait_بہترین		z_priq_simple_بہترین
#اگر_ختم

#اگر defined(CONFIG_64_ثنائی_ہندسہ)
#تعریف_کرو NBITS          64
#تعریف_کرو TRAILING_ZEROS u64_گنتی_trailing_zeros
#ورنہ
#تعریف_کرو NBITS          32
#تعریف_کرو TRAILING_ZEROS u32_گنتی_trailing_zeros
#اگر_ختم /* CONFIG_64BIT */

ساکن ALWAYS_INLINE خالی z_priq_simple_init(sys_dlist_t *pq)
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
ساکن ALWAYS_INLINE int32_t z_sched_prio_cmp(ساخت k_thread *thread_1, ساخت k_thread *thread_2)
{
	/* `prio` is <32b, so the below cannot overflow. */
	int32_t b1 = thread_1->بنیاد.prio;
	int32_t b2 = thread_2->بنیاد.prio;

	اگر (b1 != b2) {
		واپس b2 - b1;
	}

#اگر_تعریف_شدہ CONFIG_SCHED_DEADLINE
	/* If we assume all deadlines live within the same "half" of
	 * the 32 bit modulus space (this is a documented API rule),
	 * then the latest deadline in the queue minus the earliest is
	 * guaranteed to be (2's complement) non-negative.  We can
	 * leverage that to compare the values without having to check
	 * the current time.
	 */
	uint32_t d1 = thread_1->بنیاد.prio_deadline;
	uint32_t d2 = thread_2->بنیاد.prio_deadline;

	اگر (d1 != d2) {
		/* Sooner deadline means higher effective priority.
		 * Doing the calculation with unsigned types and casting
		 * to signed isn't perfect, but at least reduces this
		 * from UB on overflow to impdef.
		 */
		واپس (int32_t)(d2 - d1);
	}
#اگر_ختم /* CONFIG_SCHED_DEADLINE */
	واپس 0;
}

ساکن ALWAYS_INLINE خالی z_priq_simple_جوڑو(sys_dlist_t *pq, ساخت k_thread *thread)
{
	ساخت k_thread *t;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, t, بنیاد.qnode_dlist) {
		اگر (z_sched_prio_cmp(thread, t) > 0) {
			sys_dlist_داخل_کرو(&t->بنیاد.qnode_dlist, &thread->بنیاد.qnode_dlist);
			واپس;
		}
	}

	sys_dlist_append(pq, &thread->بنیاد.qnode_dlist);
}

ساکن ALWAYS_INLINE خالی z_priq_simple_remove(sys_dlist_t *pq, ساخت k_thread *thread)
{
	ARG_غیر_مستعمل(pq);

	sys_dlist_remove(&thread->بنیاد.qnode_dlist);
}

ساکن ALWAYS_INLINE خالی z_priq_simple_yield(sys_dlist_t *pq)
{
#اگر_غیر_تعریف_شدہ CONFIG_SMP
	sys_dnode_t *n;

	n = sys_dlist_peek_اگلا_no_check(pq, &_current->بنیاد.qnode_dlist);

	sys_dlist_قطار_سے_نکالو(&_current->بنیاد.qnode_dlist);

	ساخت k_thread *t;

	/*
	 * As it is possible that the current thread was not at the head of
	 * the run queue, start searching from the present position for where
	 * to re-insert it.
	 */

	جب (n != تہی) {
		t = CONTAINER_کا(n, ساخت k_thread, بنیاد.qnode_dlist);
		اگر (z_sched_prio_cmp(_current, t) > 0) {
			sys_dlist_داخل_کرو(&t->بنیاد.qnode_dlist,
					 &_current->بنیاد.qnode_dlist);
			واپس;
		}
		n = sys_dlist_peek_اگلا_no_check(pq, n);
	}

	sys_dlist_append(pq, &_current->بنیاد.qnode_dlist);
#اگر_ختم
}

ساکن ALWAYS_INLINE ساخت k_thread *z_priq_simple_بہترین(sys_dlist_t *pq)
{
	ساخت k_thread *thread = تہی;
	sys_dnode_t *n = sys_dlist_peek_سر(pq);

	اگر (n != تہی) {
		thread = CONTAINER_کا(n, ساخت k_thread, بنیاد.qnode_dlist);
	}
	واپس thread;
}

#اگر_تعریف_شدہ CONFIG_SCHED_CPU_نقاب
ساکن ALWAYS_INLINE ساخت k_thread *z_priq_simple_نقاب_بہترین(sys_dlist_t *pq)
{
	/* With masks enabled we need to be prepared to walk the list
	 * looking for one we can run
	 */
	غیرعلامتی طویل current_cpu_ثنائی_ہندسہ = ثنائی_ہندسہ(_current_cpu->id);
	ساخت k_thread *thread;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, thread, بنیاد.qnode_dlist) {
		اگر ((thread->بنیاد.cpu_نقاب & current_cpu_ثنائی_ہندسہ) != 0) {
			واپس thread;
		}
	}
	واپس تہی;
}
#اگر_ختم /* CONFIG_SCHED_CPU_MASK */

#اگر defined(CONFIG_SCHED_SCALABLE) || defined(CONFIG_WAITQ_SCALABLE)
ساکن ALWAYS_INLINE خالی z_priq_rb_init(ساخت _priq_rb *pq)
{
	bool z_priq_rb_lessthan(ساخت rbnode *a, ساخت rbnode *b);

	*pq = (ساخت _priq_rb) {
		.درخت = {
			.lessthan_fn = z_priq_rb_lessthan,
		}
	};
}

ساکن ALWAYS_INLINE خالی z_priq_rb_جوڑو(ساخت _priq_rb *pq, ساخت k_thread *thread)
{
	ساخت k_thread *t;

	thread->بنیاد.ترتیب_کلید = pq->اگلا_ترتیب_کلید;
	++pq->اگلا_ترتیب_کلید;

	/* Renumber at wraparound.  This is tiny code, and in practice
	 * will almost never be hit on real systems.  BUT on very
	 * long-running systems where a priq never completely empties
	 * AND that contains very large numbers of threads, it can be
	 * a latency glitch to loop over all the threads like this.
	 */
	اگر (!pq->اگلا_ترتیب_کلید) {
		RB_FOR_EACH_CONTAINER(&pq->درخت, t, بنیاد.qnode_rb) {
			t->بنیاد.ترتیب_کلید = pq->اگلا_ترتیب_کلید;
			++pq->اگلا_ترتیب_کلید;
		}
	}

	rb_داخل_کرو(&pq->درخت, &thread->بنیاد.qnode_rb);
}

ساکن ALWAYS_INLINE خالی z_priq_rb_remove(ساخت _priq_rb *pq, ساخت k_thread *thread)
{
	rb_remove(&pq->درخت, &thread->بنیاد.qnode_rb);

	اگر (!pq->درخت.جڑ) {
		pq->اگلا_ترتیب_کلید = 0;
	}
}

ساکن ALWAYS_INLINE خالی z_priq_rb_yield(ساخت _priq_rb *pq)
{
#اگر_غیر_تعریف_شدہ CONFIG_SMP
	z_priq_rb_remove(pq, _current);
	z_priq_rb_جوڑو(pq, _current);
#اگر_ختم
}

ساکن ALWAYS_INLINE ساخت k_thread *z_priq_rb_بہترین(ساخت _priq_rb *pq)
{
	ساخت k_thread *thread = تہی;
	ساخت rbnode *n = rb_get_min(&pq->درخت);

	اگر (n != تہی) {
		thread = CONTAINER_کا(n, ساخت k_thread, بنیاد.qnode_rb);
	}
	واپس thread;
}

#اگر_تعریف_شدہ CONFIG_SCHED_CPU_نقاب
ساکن ALWAYS_INLINE ساخت k_thread *z_priq_rb_نقاب_بہترین(ساخت _priq_rb *pq)
{
	غیرعلامتی طویل current_cpu_ثنائی_ہندسہ = ثنائی_ہندسہ(_current_cpu->id);
	ساخت k_thread *thread;

	RB_FOR_EACH_CONTAINER(&pq->درخت, thread, بنیاد.qnode_rb) {
		اگر ((thread->بنیاد.cpu_نقاب & current_cpu_ثنائی_ہندسہ) != 0) {
			واپس thread;
		}
	}
	واپس تہی;
}
#اگر_ختم /* CONFIG_SCHED_CPU_MASK */
#اگر_ختم /* CONFIG_SCHED_SCALABLE || CONFIG_WAITQ_SCALABLE */

ساخت prio_info {
	uint8_t انحراف_prio;
	uint8_t idx;
	uint8_t ثنائی_ہندسہ_2;
};

ساکن ALWAYS_INLINE ساخت prio_info get_prio_info(int8_t old_prio)
{
	ساخت prio_info ret;

	ret.انحراف_prio = old_prio - K_HIGHEST_THREAD_PRIO;
	ret.idx = ret.انحراف_prio / NBITS;
	ret.ثنائی_ہندسہ_2 = ret.انحراف_prio % NBITS;

	واپس ret;
}

ساکن ALWAYS_INLINE غیرعلامتی صحیح z_priq_mq_بہترین_قطار_اشاریہ(ساخت _priq_mq *pq)
{
	غیرعلامتی صحیح i = 0;

	کرو {
		اگر (likely(pq->bitmask[i])) {
			واپس i * NBITS + TRAILING_ZEROS(pq->bitmask[i]);
		}
		i++;
	} جب (i < PRIQ_BITMAP_سائز);

	واپس K_عدد_THREAD_PRIO - 1;
}

ساکن ALWAYS_INLINE خالی z_priq_mq_init(ساخت _priq_mq *q)
{
	برائے (size_t i = 0; i < ARRAY_سائز(q->queues); i++) {
		sys_dlist_init(&q->queues[i]);
	}

#اگر_غیر_تعریف_شدہ CONFIG_SMP
	q->cached_قطار_اشاریہ = K_عدد_THREAD_PRIO - 1;
#اگر_ختم
}

ساکن ALWAYS_INLINE خالی z_priq_mq_جوڑو(ساخت _priq_mq *pq,
					ساخت k_thread *thread)
{
	ساخت prio_info pos = get_prio_info(thread->بنیاد.prio);

	sys_dlist_append(&pq->queues[pos.انحراف_prio], &thread->بنیاد.qnode_dlist);
	pq->bitmask[pos.idx] |= ثنائی_ہندسہ(pos.ثنائی_ہندسہ_2);

#اگر_غیر_تعریف_شدہ CONFIG_SMP
	اگر (pos.انحراف_prio < pq->cached_قطار_اشاریہ) {
		pq->cached_قطار_اشاریہ = pos.انحراف_prio;
	}
#اگر_ختم
}

ساکن ALWAYS_INLINE خالی z_priq_mq_remove(ساخت _priq_mq *pq,
					   ساخت k_thread *thread)
{
	ساخت prio_info pos = get_prio_info(thread->بنیاد.prio);

	sys_dlist_قطار_سے_نکالو(&thread->بنیاد.qnode_dlist);
	اگر (unlikely(sys_dlist_is_خالی_خانے(&pq->queues[pos.انحراف_prio]))) {
		pq->bitmask[pos.idx] &= ~ثنائی_ہندسہ(pos.ثنائی_ہندسہ_2);
#اگر_غیر_تعریف_شدہ CONFIG_SMP
		pq->cached_قطار_اشاریہ = z_priq_mq_بہترین_قطار_اشاریہ(pq);
#اگر_ختم
	}
}

ساکن ALWAYS_INLINE خالی z_priq_mq_yield(ساخت _priq_mq *pq)
{
#اگر_غیر_تعریف_شدہ CONFIG_SMP
	ساخت prio_info pos = get_prio_info(_current->بنیاد.prio);

	sys_dlist_قطار_سے_نکالو(&_current->بنیاد.qnode_dlist);
	sys_dlist_append(&pq->queues[pos.انحراف_prio],
			 &_current->بنیاد.qnode_dlist);
#اگر_ختم
}

ساکن ALWAYS_INLINE ساخت k_thread *z_priq_mq_بہترین(ساخت _priq_mq *pq)
{
#اگر_تعریف_شدہ CONFIG_SMP
	غیرعلامتی صحیح اشاریہ = z_priq_mq_بہترین_قطار_اشاریہ(pq);
#ورنہ
	غیرعلامتی صحیح اشاریہ = pq->cached_قطار_اشاریہ;
#اگر_ختم

	sys_dnode_t *n = sys_dlist_peek_سر(&pq->queues[اشاریہ]);

	اگر (likely(n != تہی)) {
		واپس CONTAINER_کا(n, ساخت k_thread, بنیاد.qnode_dlist);
	}

	واپس تہی;
}

#اگر_تعریف_شدہ CONFIG_SCHED_CPU_نقاب
ساکن ALWAYS_INLINE ساخت k_thread *z_priq_mq_نقاب_بہترین(ساخت _priq_mq *pq)
{
	غیرعلامتی طویل current_cpu_ثنائی_ہندسہ = ثنائی_ہندسہ(_current_cpu->id);

	/* Walk priority levels from highest to lowest, and within each
	 * level walk the thread list to find the first thread runnable
	 * on the current CPU.
	 */
	برائے (غیرعلامتی صحیح i = 0U; i < PRIQ_BITMAP_سائز; i++) {
		غیرعلامتی طویل bitmask = pq->bitmask[i];

		جب (bitmask != 0UL) {
			غیرعلامتی صحیح ثنائی_ہندسہ_2 = TRAILING_ZEROS(bitmask);
			غیرعلامتی صحیح اشاریہ = i * NBITS + ثنائی_ہندسہ_2;
			ساخت k_thread *thread;

			SYS_DLIST_FOR_EACH_CONTAINER(&pq->queues[اشاریہ],
						     thread, بنیاد.qnode_dlist) {
				اگر ((thread->بنیاد.cpu_نقاب &
				     current_cpu_ثنائی_ہندسہ) != 0) {
					واپس thread;
				}
			}
			bitmask &= (bitmask - 1UL);
		}
	}
	واپس تہی;
}
#اگر_ختم /* CONFIG_SCHED_CPU_MASK */

#اگر_ختم /* ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_ */
