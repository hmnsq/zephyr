/*
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#اگر_تعریف_نشده ZEPHYR_KERNEL_INCLUDE_اولویت_Q_H
#تعریف_کن ZEPHYR_KERNEL_INCLUDE_اولویت_Q_H

#ضمن <zephyr/sys/math_extras.h>
#ضمن <zephyr/sys/dlist.h>

/* Simple Scheduling */
#إذا defined(CONFIG_SCHED_SIMPLE)
#تعریف_کن priq_پیاپی_init		z_priq_simple_init
#تعریف_کن priq_پیاپی_جمع_کن		z_priq_simple_جمع_کن
#تعریف_کن priq_پیاپی_remove	z_priq_simple_remove
#تعریف_کن priq_پیاپی_yield         z_priq_simple_yield
# إذا defined(CONFIG_SCHED_CPU_ماسک) && !defined(CONFIG_SCHED_CPU_ماسک_PIN_ONLY)
#  define priq_پیاپی_بهترین	z_priq_simple_ماسک_بهترین
# وإلا
#  define priq_پیاپی_بهترین	z_priq_simple_بهترین
# endif /* CONFIG_SCHED_CPU_MASK */
/* Scalable Scheduling */
#وإلاإذا defined(CONFIG_SCHED_SCALABLE)
#تعریف_کن priq_پیاپی_init		z_priq_rb_init
#تعریف_کن priq_پیاپی_جمع_کن		z_priq_rb_جمع_کن
#تعریف_کن priq_پیاپی_remove	z_priq_rb_remove
#تعریف_کن priq_پیاپی_yield         z_priq_rb_yield
# إذا defined(CONFIG_SCHED_CPU_ماسک) && !defined(CONFIG_SCHED_CPU_ماسک_PIN_ONLY)
#  define priq_پیاپی_بهترین	z_priq_rb_ماسک_بهترین
# وإلا
#  define priq_پیاپی_بهترین	z_priq_rb_بهترین
# endif /* CONFIG_SCHED_CPU_MASK */
 /* Multi Queue Scheduling */
#وإلاإذا defined(CONFIG_SCHED_MULTIQ)
#تعریف_کن priq_پیاپی_init		z_priq_mq_init
#تعریف_کن priq_پیاپی_جمع_کن		z_priq_mq_جمع_کن
#تعریف_کن priq_پیاپی_remove	z_priq_mq_remove
#تعریف_کن priq_پیاپی_yield         z_priq_mq_yield
# إذا defined(CONFIG_SCHED_CPU_ماسک) && !defined(CONFIG_SCHED_CPU_ماسک_PIN_ONLY)
#  define priq_پیاپی_بهترین	z_priq_mq_ماسک_بهترین
# وإلا
#  define priq_پیاپی_بهترین	z_priq_mq_بهترین
# endif /* CONFIG_SCHED_CPU_MASK */
#پایان_اگر

/* Scalable Wait Queue */
#إذا defined(CONFIG_WAITQ_SCALABLE)
#تعریف_کن priq_wait_جمع_کن		z_priq_rb_جمع_کن
#تعریف_کن _priq_wait_remove	z_priq_rb_remove
#تعریف_کن priq_wait_بهترین		z_priq_rb_بهترین
/* Simple Wait Queue */
#وإلاإذا defined(CONFIG_WAITQ_SIMPLE)
#تعریف_کن priq_wait_جمع_کن		z_priq_simple_جمع_کن
#تعریف_کن _priq_wait_remove	z_priq_simple_remove
#تعریف_کن priq_wait_بهترین		z_priq_simple_بهترین
#پایان_اگر

#إذا defined(CONFIG_64_بیت)
#تعریف_کن NBITS          64
#تعریف_کن TRAILING_ZEROS u64_شمار_trailing_zeros
#وإلا
#تعریف_کن NBITS          32
#تعریف_کن TRAILING_ZEROS u32_شمار_trailing_zeros
#پایان_اگر /* CONFIG_64BIT */

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
	int32_t b1 = thread_1->پایه.prio;
	int32_t b2 = thread_2->پایه.prio;

	إذا (b1 != b2) {
		أرجع b2 - b1;
	}

#اگر_تعریف_شده CONFIG_SCHED_DEADLINE
	/* If we assume all deadlines live within the same "half" of
	 * the 32 bit modulus space (this is a documented API rule),
	 * then the latest deadline in the queue minus the earliest is
	 * guaranteed to be (2's complement) non-negative.  We can
	 * leverage that to compare the values without having to check
	 * the current time.
	 */
	uint32_t d1 = thread_1->پایه.prio_deadline;
	uint32_t d2 = thread_2->پایه.prio_deadline;

	إذا (d1 != d2) {
		/* Sooner deadline means higher effective priority.
		 * Doing the calculation with unsigned types and casting
		 * to signed isn't perfect, but at least reduces this
		 * from UB on overflow to impdef.
		 */
		أرجع (int32_t)(d2 - d1);
	}
#پایان_اگر /* CONFIG_SCHED_DEADLINE */
	أرجع 0;
}

ساكن ALWAYS_INLINE فراغ z_priq_simple_جمع_کن(sys_dlist_t *pq, بنية k_thread *thread)
{
	بنية k_thread *t;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, t, پایه.qnode_dlist) {
		إذا (z_sched_prio_cmp(thread, t) > 0) {
			sys_dlist_درج(&t->پایه.qnode_dlist, &thread->پایه.qnode_dlist);
			أرجع;
		}
	}

	sys_dlist_append(pq, &thread->پایه.qnode_dlist);
}

ساكن ALWAYS_INLINE فراغ z_priq_simple_remove(sys_dlist_t *pq, بنية k_thread *thread)
{
	ARG_استفادهنشده(pq);

	sys_dlist_remove(&thread->پایه.qnode_dlist);
}

ساكن ALWAYS_INLINE فراغ z_priq_simple_yield(sys_dlist_t *pq)
{
#اگر_تعریف_نشده CONFIG_SMP
	sys_dnode_t *n;

	n = sys_dlist_peek_بعدی_no_check(pq, &_current->پایه.qnode_dlist);

	sys_dlist_خارج_از_صف(&_current->پایه.qnode_dlist);

	بنية k_thread *t;

	/*
	 * As it is possible that the current thread was not at the head of
	 * the run queue, start searching from the present position for where
	 * to re-insert it.
	 */

	طالما (n != عدم) {
		t = CONTAINER_از(n, بنية k_thread, پایه.qnode_dlist);
		إذا (z_sched_prio_cmp(_current, t) > 0) {
			sys_dlist_درج(&t->پایه.qnode_dlist,
					 &_current->پایه.qnode_dlist);
			أرجع;
		}
		n = sys_dlist_peek_بعدی_no_check(pq, n);
	}

	sys_dlist_append(pq, &_current->پایه.qnode_dlist);
#پایان_اگر
}

ساكن ALWAYS_INLINE بنية k_thread *z_priq_simple_بهترین(sys_dlist_t *pq)
{
	بنية k_thread *thread = عدم;
	sys_dnode_t *n = sys_dlist_peek_سر(pq);

	إذا (n != عدم) {
		thread = CONTAINER_از(n, بنية k_thread, پایه.qnode_dlist);
	}
	أرجع thread;
}

#اگر_تعریف_شده CONFIG_SCHED_CPU_ماسک
ساكن ALWAYS_INLINE بنية k_thread *z_priq_simple_ماسک_بهترین(sys_dlist_t *pq)
{
	/* With masks enabled we need to be prepared to walk the list
	 * looking for one we can run
	 */
	بیعلامت دراز current_cpu_بیت = بیت(_current_cpu->id);
	بنية k_thread *thread;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, thread, پایه.qnode_dlist) {
		إذا ((thread->پایه.cpu_ماسک & current_cpu_بیت) != 0) {
			أرجع thread;
		}
	}
	أرجع عدم;
}
#پایان_اگر /* CONFIG_SCHED_CPU_MASK */

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

ساكن ALWAYS_INLINE فراغ z_priq_rb_جمع_کن(بنية _priq_rb *pq, بنية k_thread *thread)
{
	بنية k_thread *t;

	thread->پایه.ترتیب_کلید = pq->بعدی_ترتیب_کلید;
	++pq->بعدی_ترتیب_کلید;

	/* Renumber at wraparound.  This is tiny code, and in practice
	 * will almost never be hit on real systems.  BUT on very
	 * long-running systems where a priq never completely empties
	 * AND that contains very large numbers of threads, it can be
	 * a latency glitch to loop over all the threads like this.
	 */
	إذا (!pq->بعدی_ترتیب_کلید) {
		RB_FOR_EACH_CONTAINER(&pq->درخت, t, پایه.qnode_rb) {
			t->پایه.ترتیب_کلید = pq->بعدی_ترتیب_کلید;
			++pq->بعدی_ترتیب_کلید;
		}
	}

	rb_درج(&pq->درخت, &thread->پایه.qnode_rb);
}

ساكن ALWAYS_INLINE فراغ z_priq_rb_remove(بنية _priq_rb *pq, بنية k_thread *thread)
{
	rb_remove(&pq->درخت, &thread->پایه.qnode_rb);

	إذا (!pq->درخت.ریشه) {
		pq->بعدی_ترتیب_کلید = 0;
	}
}

ساكن ALWAYS_INLINE فراغ z_priq_rb_yield(بنية _priq_rb *pq)
{
#اگر_تعریف_نشده CONFIG_SMP
	z_priq_rb_remove(pq, _current);
	z_priq_rb_جمع_کن(pq, _current);
#پایان_اگر
}

ساكن ALWAYS_INLINE بنية k_thread *z_priq_rb_بهترین(بنية _priq_rb *pq)
{
	بنية k_thread *thread = عدم;
	بنية rbnode *n = rb_get_min(&pq->درخت);

	إذا (n != عدم) {
		thread = CONTAINER_از(n, بنية k_thread, پایه.qnode_rb);
	}
	أرجع thread;
}

#اگر_تعریف_شده CONFIG_SCHED_CPU_ماسک
ساكن ALWAYS_INLINE بنية k_thread *z_priq_rb_ماسک_بهترین(بنية _priq_rb *pq)
{
	بیعلامت دراز current_cpu_بیت = بیت(_current_cpu->id);
	بنية k_thread *thread;

	RB_FOR_EACH_CONTAINER(&pq->درخت, thread, پایه.qnode_rb) {
		إذا ((thread->پایه.cpu_ماسک & current_cpu_بیت) != 0) {
			أرجع thread;
		}
	}
	أرجع عدم;
}
#پایان_اگر /* CONFIG_SCHED_CPU_MASK */
#پایان_اگر /* CONFIG_SCHED_SCALABLE || CONFIG_WAITQ_SCALABLE */

بنية prio_info {
	uint8_t جابه_جایی_prio;
	uint8_t idx;
	uint8_t بیت_2;
};

ساكن ALWAYS_INLINE بنية prio_info get_prio_info(int8_t old_prio)
{
	بنية prio_info ret;

	ret.جابه_جایی_prio = old_prio - K_HIGHEST_THREAD_PRIO;
	ret.idx = ret.جابه_جایی_prio / NBITS;
	ret.بیت_2 = ret.جابه_جایی_prio % NBITS;

	أرجع ret;
}

ساكن ALWAYS_INLINE بیعلامت صحيح z_priq_mq_بهترین_صف_شاخص(بنية _priq_mq *pq)
{
	بیعلامت صحيح i = 0;

	افعل {
		إذا (likely(pq->bitmask[i])) {
			أرجع i * NBITS + TRAILING_ZEROS(pq->bitmask[i]);
		}
		i++;
	} طالما (i < PRIQ_BITMAP_اندازه);

	أرجع K_رقم_THREAD_PRIO - 1;
}

ساكن ALWAYS_INLINE فراغ z_priq_mq_init(بنية _priq_mq *q)
{
	لكل (size_t i = 0; i < ARRAY_اندازه(q->queues); i++) {
		sys_dlist_init(&q->queues[i]);
	}

#اگر_تعریف_نشده CONFIG_SMP
	q->cached_صف_شاخص = K_رقم_THREAD_PRIO - 1;
#پایان_اگر
}

ساكن ALWAYS_INLINE فراغ z_priq_mq_جمع_کن(بنية _priq_mq *pq,
					بنية k_thread *thread)
{
	بنية prio_info pos = get_prio_info(thread->پایه.prio);

	sys_dlist_append(&pq->queues[pos.جابه_جایی_prio], &thread->پایه.qnode_dlist);
	pq->bitmask[pos.idx] |= بیت(pos.بیت_2);

#اگر_تعریف_نشده CONFIG_SMP
	إذا (pos.جابه_جایی_prio < pq->cached_صف_شاخص) {
		pq->cached_صف_شاخص = pos.جابه_جایی_prio;
	}
#پایان_اگر
}

ساكن ALWAYS_INLINE فراغ z_priq_mq_remove(بنية _priq_mq *pq,
					   بنية k_thread *thread)
{
	بنية prio_info pos = get_prio_info(thread->پایه.prio);

	sys_dlist_خارج_از_صف(&thread->پایه.qnode_dlist);
	إذا (unlikely(sys_dlist_is_خالی(&pq->queues[pos.جابه_جایی_prio]))) {
		pq->bitmask[pos.idx] &= ~بیت(pos.بیت_2);
#اگر_تعریف_نشده CONFIG_SMP
		pq->cached_صف_شاخص = z_priq_mq_بهترین_صف_شاخص(pq);
#پایان_اگر
	}
}

ساكن ALWAYS_INLINE فراغ z_priq_mq_yield(بنية _priq_mq *pq)
{
#اگر_تعریف_نشده CONFIG_SMP
	بنية prio_info pos = get_prio_info(_current->پایه.prio);

	sys_dlist_خارج_از_صف(&_current->پایه.qnode_dlist);
	sys_dlist_append(&pq->queues[pos.جابه_جایی_prio],
			 &_current->پایه.qnode_dlist);
#پایان_اگر
}

ساكن ALWAYS_INLINE بنية k_thread *z_priq_mq_بهترین(بنية _priq_mq *pq)
{
#اگر_تعریف_شده CONFIG_SMP
	بیعلامت صحيح شاخص = z_priq_mq_بهترین_صف_شاخص(pq);
#وإلا
	بیعلامت صحيح شاخص = pq->cached_صف_شاخص;
#پایان_اگر

	sys_dnode_t *n = sys_dlist_peek_سر(&pq->queues[شاخص]);

	إذا (likely(n != عدم)) {
		أرجع CONTAINER_از(n, بنية k_thread, پایه.qnode_dlist);
	}

	أرجع عدم;
}

#اگر_تعریف_شده CONFIG_SCHED_CPU_ماسک
ساكن ALWAYS_INLINE بنية k_thread *z_priq_mq_ماسک_بهترین(بنية _priq_mq *pq)
{
	بیعلامت دراز current_cpu_بیت = بیت(_current_cpu->id);

	/* Walk priority levels from highest to lowest, and within each
	 * level walk the thread list to find the first thread runnable
	 * on the current CPU.
	 */
	لكل (بیعلامت صحيح i = 0U; i < PRIQ_BITMAP_اندازه; i++) {
		بیعلامت دراز bitmask = pq->bitmask[i];

		طالما (bitmask != 0UL) {
			بیعلامت صحيح بیت_2 = TRAILING_ZEROS(bitmask);
			بیعلامت صحيح شاخص = i * NBITS + بیت_2;
			بنية k_thread *thread;

			SYS_DLIST_FOR_EACH_CONTAINER(&pq->queues[شاخص],
						     thread, پایه.qnode_dlist) {
				إذا ((thread->پایه.cpu_ماسک &
				     current_cpu_بیت) != 0) {
					أرجع thread;
				}
			}
			bitmask &= (bitmask - 1UL);
		}
	}
	أرجع عدم;
}
#پایان_اگر /* CONFIG_SCHED_CPU_MASK */

#پایان_اگر /* ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_ */
