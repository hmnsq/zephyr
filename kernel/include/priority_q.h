/*
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#جيڪڏهن_وصف_نه_ٿيل ZEPHYR_KERNEL_INCLUDE_ترجيح_Q_H
#وصف_ڪريو ZEPHYR_KERNEL_INCLUDE_ترجيح_Q_H

#ضمن <zephyr/sys/math_extras.h>
#ضمن <zephyr/sys/dlist.h>

/* Simple Scheduling */
#إذا defined(CONFIG_SCHED_SIMPLE)
#وصف_ڪريو priq_لاڳيتو_init		z_priq_simple_init
#وصف_ڪريو priq_لاڳيتو_جوڙيو		z_priq_simple_جوڙيو
#وصف_ڪريو priq_لاڳيتو_remove	z_priq_simple_remove
#وصف_ڪريو priq_لاڳيتو_yield         z_priq_simple_yield
# إذا defined(CONFIG_SCHED_CPU_نقاب) && !defined(CONFIG_SCHED_CPU_نقاب_PIN_ONLY)
#  define priq_لاڳيتو_بهترين	z_priq_simple_نقاب_بهترين
# وإلا
#  define priq_لاڳيتو_بهترين	z_priq_simple_بهترين
# endif /* CONFIG_SCHED_CPU_MASK */
/* Scalable Scheduling */
#وإلاإذا defined(CONFIG_SCHED_SCALABLE)
#وصف_ڪريو priq_لاڳيتو_init		z_priq_rb_init
#وصف_ڪريو priq_لاڳيتو_جوڙيو		z_priq_rb_جوڙيو
#وصف_ڪريو priq_لاڳيتو_remove	z_priq_rb_remove
#وصف_ڪريو priq_لاڳيتو_yield         z_priq_rb_yield
# إذا defined(CONFIG_SCHED_CPU_نقاب) && !defined(CONFIG_SCHED_CPU_نقاب_PIN_ONLY)
#  define priq_لاڳيتو_بهترين	z_priq_rb_نقاب_بهترين
# وإلا
#  define priq_لاڳيتو_بهترين	z_priq_rb_بهترين
# endif /* CONFIG_SCHED_CPU_MASK */
 /* Multi Queue Scheduling */
#وإلاإذا defined(CONFIG_SCHED_MULTIQ)
#وصف_ڪريو priq_لاڳيتو_init		z_priq_mq_init
#وصف_ڪريو priq_لاڳيتو_جوڙيو		z_priq_mq_جوڙيو
#وصف_ڪريو priq_لاڳيتو_remove	z_priq_mq_remove
#وصف_ڪريو priq_لاڳيتو_yield         z_priq_mq_yield
# إذا defined(CONFIG_SCHED_CPU_نقاب) && !defined(CONFIG_SCHED_CPU_نقاب_PIN_ONLY)
#  define priq_لاڳيتو_بهترين	z_priq_mq_نقاب_بهترين
# وإلا
#  define priq_لاڳيتو_بهترين	z_priq_mq_بهترين
# endif /* CONFIG_SCHED_CPU_MASK */
#جيڪڏهن_پورو

/* Scalable Wait Queue */
#إذا defined(CONFIG_WAITQ_SCALABLE)
#وصف_ڪريو priq_wait_جوڙيو		z_priq_rb_جوڙيو
#وصف_ڪريو _priq_wait_remove	z_priq_rb_remove
#وصف_ڪريو priq_wait_بهترين		z_priq_rb_بهترين
/* Simple Wait Queue */
#وإلاإذا defined(CONFIG_WAITQ_SIMPLE)
#وصف_ڪريو priq_wait_جوڙيو		z_priq_simple_جوڙيو
#وصف_ڪريو _priq_wait_remove	z_priq_simple_remove
#وصف_ڪريو priq_wait_بهترين		z_priq_simple_بهترين
#جيڪڏهن_پورو

#إذا defined(CONFIG_64_ٻٽو_انگ)
#وصف_ڪريو NBITS          64
#وصف_ڪريو TRAILING_ZEROS u64_ڳڻپ_trailing_zeros
#وإلا
#وصف_ڪريو NBITS          32
#وصف_ڪريو TRAILING_ZEROS u32_ڳڻپ_trailing_zeros
#جيڪڏهن_پورو /* CONFIG_64BIT */

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
	int32_t b1 = thread_1->بنياد.prio;
	int32_t b2 = thread_2->بنياد.prio;

	إذا (b1 != b2) {
		أرجع b2 - b1;
	}

#جيڪڏهن_وصف_ٿيل CONFIG_SCHED_DEADLINE
	/* If we assume all deadlines live within the same "half" of
	 * the 32 bit modulus space (this is a documented API rule),
	 * then the latest deadline in the queue minus the earliest is
	 * guaranteed to be (2's complement) non-negative.  We can
	 * leverage that to compare the values without having to check
	 * the current time.
	 */
	uint32_t d1 = thread_1->بنياد.prio_deadline;
	uint32_t d2 = thread_2->بنياد.prio_deadline;

	إذا (d1 != d2) {
		/* Sooner deadline means higher effective priority.
		 * Doing the calculation with unsigned types and casting
		 * to signed isn't perfect, but at least reduces this
		 * from UB on overflow to impdef.
		 */
		أرجع (int32_t)(d2 - d1);
	}
#جيڪڏهن_پورو /* CONFIG_SCHED_DEADLINE */
	أرجع 0;
}

ساكن ALWAYS_INLINE فراغ z_priq_simple_جوڙيو(sys_dlist_t *pq, بنية k_thread *thread)
{
	بنية k_thread *t;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, t, بنياد.qnode_dlist) {
		إذا (z_sched_prio_cmp(thread, t) > 0) {
			sys_dlist_وجهو(&t->بنياد.qnode_dlist, &thread->بنياد.qnode_dlist);
			أرجع;
		}
	}

	sys_dlist_append(pq, &thread->بنياد.qnode_dlist);
}

ساكن ALWAYS_INLINE فراغ z_priq_simple_remove(sys_dlist_t *pq, بنية k_thread *thread)
{
	ARG_اڻ_استعمال_ٿيل(pq);

	sys_dlist_remove(&thread->بنياد.qnode_dlist);
}

ساكن ALWAYS_INLINE فراغ z_priq_simple_yield(sys_dlist_t *pq)
{
#جيڪڏهن_وصف_نه_ٿيل CONFIG_SMP
	sys_dnode_t *n;

	n = sys_dlist_peek_ايندڙ_no_check(pq, &_current->بنياد.qnode_dlist);

	sys_dlist_قطار_مان_ڪڍو(&_current->بنياد.qnode_dlist);

	بنية k_thread *t;

	/*
	 * As it is possible that the current thread was not at the head of
	 * the run queue, start searching from the present position for where
	 * to re-insert it.
	 */

	طالما (n != عدم) {
		t = CONTAINER_جو(n, بنية k_thread, بنياد.qnode_dlist);
		إذا (z_sched_prio_cmp(_current, t) > 0) {
			sys_dlist_وجهو(&t->بنياد.qnode_dlist,
					 &_current->بنياد.qnode_dlist);
			أرجع;
		}
		n = sys_dlist_peek_ايندڙ_no_check(pq, n);
	}

	sys_dlist_append(pq, &_current->بنياد.qnode_dlist);
#جيڪڏهن_پورو
}

ساكن ALWAYS_INLINE بنية k_thread *z_priq_simple_بهترين(sys_dlist_t *pq)
{
	بنية k_thread *thread = عدم;
	sys_dnode_t *n = sys_dlist_peek_سر(pq);

	إذا (n != عدم) {
		thread = CONTAINER_جو(n, بنية k_thread, بنياد.qnode_dlist);
	}
	أرجع thread;
}

#جيڪڏهن_وصف_ٿيل CONFIG_SCHED_CPU_نقاب
ساكن ALWAYS_INLINE بنية k_thread *z_priq_simple_نقاب_بهترين(sys_dlist_t *pq)
{
	/* With masks enabled we need to be prepared to walk the list
	 * looking for one we can run
	 */
	بنانشان ڊگهو current_cpu_ٻٽو_انگ = ٻٽو_انگ(_current_cpu->id);
	بنية k_thread *thread;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, thread, بنياد.qnode_dlist) {
		إذا ((thread->بنياد.cpu_نقاب & current_cpu_ٻٽو_انگ) != 0) {
			أرجع thread;
		}
	}
	أرجع عدم;
}
#جيڪڏهن_پورو /* CONFIG_SCHED_CPU_MASK */

#إذا defined(CONFIG_SCHED_SCALABLE) || defined(CONFIG_WAITQ_SCALABLE)
ساكن ALWAYS_INLINE فراغ z_priq_rb_init(بنية _priq_rb *pq)
{
	bool z_priq_rb_lessthan(بنية rbnode *a, بنية rbnode *b);

	*pq = (بنية _priq_rb) {
		.وڻ = {
			.lessthan_fn = z_priq_rb_lessthan,
		}
	};
}

ساكن ALWAYS_INLINE فراغ z_priq_rb_جوڙيو(بنية _priq_rb *pq, بنية k_thread *thread)
{
	بنية k_thread *t;

	thread->بنياد.ترتيب_ڪنجي = pq->ايندڙ_ترتيب_ڪنجي;
	++pq->ايندڙ_ترتيب_ڪنجي;

	/* Renumber at wraparound.  This is tiny code, and in practice
	 * will almost never be hit on real systems.  BUT on very
	 * long-running systems where a priq never completely empties
	 * AND that contains very large numbers of threads, it can be
	 * a latency glitch to loop over all the threads like this.
	 */
	إذا (!pq->ايندڙ_ترتيب_ڪنجي) {
		RB_FOR_EACH_CONTAINER(&pq->وڻ, t, بنياد.qnode_rb) {
			t->بنياد.ترتيب_ڪنجي = pq->ايندڙ_ترتيب_ڪنجي;
			++pq->ايندڙ_ترتيب_ڪنجي;
		}
	}

	rb_وجهو(&pq->وڻ, &thread->بنياد.qnode_rb);
}

ساكن ALWAYS_INLINE فراغ z_priq_rb_remove(بنية _priq_rb *pq, بنية k_thread *thread)
{
	rb_remove(&pq->وڻ, &thread->بنياد.qnode_rb);

	إذا (!pq->وڻ.پاڙ) {
		pq->ايندڙ_ترتيب_ڪنجي = 0;
	}
}

ساكن ALWAYS_INLINE فراغ z_priq_rb_yield(بنية _priq_rb *pq)
{
#جيڪڏهن_وصف_نه_ٿيل CONFIG_SMP
	z_priq_rb_remove(pq, _current);
	z_priq_rb_جوڙيو(pq, _current);
#جيڪڏهن_پورو
}

ساكن ALWAYS_INLINE بنية k_thread *z_priq_rb_بهترين(بنية _priq_rb *pq)
{
	بنية k_thread *thread = عدم;
	بنية rbnode *n = rb_get_min(&pq->وڻ);

	إذا (n != عدم) {
		thread = CONTAINER_جو(n, بنية k_thread, بنياد.qnode_rb);
	}
	أرجع thread;
}

#جيڪڏهن_وصف_ٿيل CONFIG_SCHED_CPU_نقاب
ساكن ALWAYS_INLINE بنية k_thread *z_priq_rb_نقاب_بهترين(بنية _priq_rb *pq)
{
	بنانشان ڊگهو current_cpu_ٻٽو_انگ = ٻٽو_انگ(_current_cpu->id);
	بنية k_thread *thread;

	RB_FOR_EACH_CONTAINER(&pq->وڻ, thread, بنياد.qnode_rb) {
		إذا ((thread->بنياد.cpu_نقاب & current_cpu_ٻٽو_انگ) != 0) {
			أرجع thread;
		}
	}
	أرجع عدم;
}
#جيڪڏهن_پورو /* CONFIG_SCHED_CPU_MASK */
#جيڪڏهن_پورو /* CONFIG_SCHED_SCALABLE || CONFIG_WAITQ_SCALABLE */

بنية prio_info {
	uint8_t وٿي_prio;
	uint8_t idx;
	uint8_t ٻٽو_انگ_2;
};

ساكن ALWAYS_INLINE بنية prio_info get_prio_info(int8_t old_prio)
{
	بنية prio_info ret;

	ret.وٿي_prio = old_prio - K_HIGHEST_THREAD_PRIO;
	ret.idx = ret.وٿي_prio / NBITS;
	ret.ٻٽو_انگ_2 = ret.وٿي_prio % NBITS;

	أرجع ret;
}

ساكن ALWAYS_INLINE بنانشان صحيح z_priq_mq_بهترين_قطار_اشاريو(بنية _priq_mq *pq)
{
	بنانشان صحيح i = 0;

	افعل {
		إذا (likely(pq->bitmask[i])) {
			أرجع i * NBITS + TRAILING_ZEROS(pq->bitmask[i]);
		}
		i++;
	} طالما (i < PRIQ_BITMAP_سائيز);

	أرجع K_انگ_THREAD_PRIO - 1;
}

ساكن ALWAYS_INLINE فراغ z_priq_mq_init(بنية _priq_mq *q)
{
	لكل (size_t i = 0; i < ARRAY_سائيز(q->queues); i++) {
		sys_dlist_init(&q->queues[i]);
	}

#جيڪڏهن_وصف_نه_ٿيل CONFIG_SMP
	q->cached_قطار_اشاريو = K_انگ_THREAD_PRIO - 1;
#جيڪڏهن_پورو
}

ساكن ALWAYS_INLINE فراغ z_priq_mq_جوڙيو(بنية _priq_mq *pq,
					بنية k_thread *thread)
{
	بنية prio_info pos = get_prio_info(thread->بنياد.prio);

	sys_dlist_append(&pq->queues[pos.وٿي_prio], &thread->بنياد.qnode_dlist);
	pq->bitmask[pos.idx] |= ٻٽو_انگ(pos.ٻٽو_انگ_2);

#جيڪڏهن_وصف_نه_ٿيل CONFIG_SMP
	إذا (pos.وٿي_prio < pq->cached_قطار_اشاريو) {
		pq->cached_قطار_اشاريو = pos.وٿي_prio;
	}
#جيڪڏهن_پورو
}

ساكن ALWAYS_INLINE فراغ z_priq_mq_remove(بنية _priq_mq *pq,
					   بنية k_thread *thread)
{
	بنية prio_info pos = get_prio_info(thread->بنياد.prio);

	sys_dlist_قطار_مان_ڪڍو(&thread->بنياد.qnode_dlist);
	إذا (unlikely(sys_dlist_is_خالي_جايون(&pq->queues[pos.وٿي_prio]))) {
		pq->bitmask[pos.idx] &= ~ٻٽو_انگ(pos.ٻٽو_انگ_2);
#جيڪڏهن_وصف_نه_ٿيل CONFIG_SMP
		pq->cached_قطار_اشاريو = z_priq_mq_بهترين_قطار_اشاريو(pq);
#جيڪڏهن_پورو
	}
}

ساكن ALWAYS_INLINE فراغ z_priq_mq_yield(بنية _priq_mq *pq)
{
#جيڪڏهن_وصف_نه_ٿيل CONFIG_SMP
	بنية prio_info pos = get_prio_info(_current->بنياد.prio);

	sys_dlist_قطار_مان_ڪڍو(&_current->بنياد.qnode_dlist);
	sys_dlist_append(&pq->queues[pos.وٿي_prio],
			 &_current->بنياد.qnode_dlist);
#جيڪڏهن_پورو
}

ساكن ALWAYS_INLINE بنية k_thread *z_priq_mq_بهترين(بنية _priq_mq *pq)
{
#جيڪڏهن_وصف_ٿيل CONFIG_SMP
	بنانشان صحيح اشاريو = z_priq_mq_بهترين_قطار_اشاريو(pq);
#وإلا
	بنانشان صحيح اشاريو = pq->cached_قطار_اشاريو;
#جيڪڏهن_پورو

	sys_dnode_t *n = sys_dlist_peek_سر(&pq->queues[اشاريو]);

	إذا (likely(n != عدم)) {
		أرجع CONTAINER_جو(n, بنية k_thread, بنياد.qnode_dlist);
	}

	أرجع عدم;
}

#جيڪڏهن_وصف_ٿيل CONFIG_SCHED_CPU_نقاب
ساكن ALWAYS_INLINE بنية k_thread *z_priq_mq_نقاب_بهترين(بنية _priq_mq *pq)
{
	بنانشان ڊگهو current_cpu_ٻٽو_انگ = ٻٽو_انگ(_current_cpu->id);

	/* Walk priority levels from highest to lowest, and within each
	 * level walk the thread list to find the first thread runnable
	 * on the current CPU.
	 */
	لكل (بنانشان صحيح i = 0U; i < PRIQ_BITMAP_سائيز; i++) {
		بنانشان ڊگهو bitmask = pq->bitmask[i];

		طالما (bitmask != 0UL) {
			بنانشان صحيح ٻٽو_انگ_2 = TRAILING_ZEROS(bitmask);
			بنانشان صحيح اشاريو = i * NBITS + ٻٽو_انگ_2;
			بنية k_thread *thread;

			SYS_DLIST_FOR_EACH_CONTAINER(&pq->queues[اشاريو],
						     thread, بنياد.qnode_dlist) {
				إذا ((thread->بنياد.cpu_نقاب &
				     current_cpu_ٻٽو_انگ) != 0) {
					أرجع thread;
				}
			}
			bitmask &= (bitmask - 1UL);
		}
	}
	أرجع عدم;
}
#جيڪڏهن_پورو /* CONFIG_SCHED_CPU_MASK */

#جيڪڏهن_پورو /* ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_ */
