/*
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#جے_بنا_تعریف ZEPHYR_KERNEL_INCLUDE_ترجیح_Q_H
#تعریف_کرو ZEPHYR_KERNEL_INCLUDE_ترجیح_Q_H

#ضمن <zephyr/sys/math_extras.h>
#ضمن <zephyr/sys/dlist.h>

/* Simple Scheduling */
#إذا defined(CONFIG_SCHED_SIMPLE)
#تعریف_کرو priq_لگاتار_init		z_priq_simple_init
#تعریف_کرو priq_لگاتار_جوڑو		z_priq_simple_جوڑو
#تعریف_کرو priq_لگاتار_remove	z_priq_simple_remove
#تعریف_کرو priq_لگاتار_yield         z_priq_simple_yield
# إذا defined(CONFIG_SCHED_CPU_نقاب) && !defined(CONFIG_SCHED_CPU_نقاب_PIN_ONLY)
#  define priq_لگاتار_بہترین	z_priq_simple_نقاب_بہترین
# وإلا
#  define priq_لگاتار_بہترین	z_priq_simple_بہترین
# endif /* CONFIG_SCHED_CPU_MASK */
/* Scalable Scheduling */
#وإلاإذا defined(CONFIG_SCHED_SCALABLE)
#تعریف_کرو priq_لگاتار_init		z_priq_rb_init
#تعریف_کرو priq_لگاتار_جوڑو		z_priq_rb_جوڑو
#تعریف_کرو priq_لگاتار_remove	z_priq_rb_remove
#تعریف_کرو priq_لگاتار_yield         z_priq_rb_yield
# إذا defined(CONFIG_SCHED_CPU_نقاب) && !defined(CONFIG_SCHED_CPU_نقاب_PIN_ONLY)
#  define priq_لگاتار_بہترین	z_priq_rb_نقاب_بہترین
# وإلا
#  define priq_لگاتار_بہترین	z_priq_rb_بہترین
# endif /* CONFIG_SCHED_CPU_MASK */
 /* Multi Queue Scheduling */
#وإلاإذا defined(CONFIG_SCHED_MULTIQ)
#تعریف_کرو priq_لگاتار_init		z_priq_mq_init
#تعریف_کرو priq_لگاتار_جوڑو		z_priq_mq_جوڑو
#تعریف_کرو priq_لگاتار_remove	z_priq_mq_remove
#تعریف_کرو priq_لگاتار_yield         z_priq_mq_yield
# إذا defined(CONFIG_SCHED_CPU_نقاب) && !defined(CONFIG_SCHED_CPU_نقاب_PIN_ONLY)
#  define priq_لگاتار_بہترین	z_priq_mq_نقاب_بہترین
# وإلا
#  define priq_لگاتار_بہترین	z_priq_mq_بہترین
# endif /* CONFIG_SCHED_CPU_MASK */
#جے_انت

/* Scalable Wait Queue */
#إذا defined(CONFIG_WAITQ_SCALABLE)
#تعریف_کرو priq_wait_جوڑو		z_priq_rb_جوڑو
#تعریف_کرو _priq_wait_remove	z_priq_rb_remove
#تعریف_کرو priq_wait_بہترین		z_priq_rb_بہترین
/* Simple Wait Queue */
#وإلاإذا defined(CONFIG_WAITQ_SIMPLE)
#تعریف_کرو priq_wait_جوڑو		z_priq_simple_جوڑو
#تعریف_کرو _priq_wait_remove	z_priq_simple_remove
#تعریف_کرو priq_wait_بہترین		z_priq_simple_بہترین
#جے_انت

#إذا defined(CONFIG_64_ثنائی_ہندسہ)
#تعریف_کرو NBITS          64
#تعریف_کرو TRAILING_ZEROS u64_گنتی_trailing_zeros
#وإلا
#تعریف_کرو NBITS          32
#تعریف_کرو TRAILING_ZEROS u32_گنتی_trailing_zeros
#جے_انت /* CONFIG_64BIT */

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
	int32_t b1 = thread_1->ادھار.prio;
	int32_t b2 = thread_2->ادھار.prio;

	إذا (b1 != b2) {
		أرجع b2 - b1;
	}

#جے_تعریف_شدہ CONFIG_SCHED_DEADLINE
	/* If we assume all deadlines live within the same "half" of
	 * the 32 bit modulus space (this is a documented API rule),
	 * then the latest deadline in the queue minus the earliest is
	 * guaranteed to be (2's complement) non-negative.  We can
	 * leverage that to compare the values without having to check
	 * the current time.
	 */
	uint32_t d1 = thread_1->ادھار.prio_deadline;
	uint32_t d2 = thread_2->ادھار.prio_deadline;

	إذا (d1 != d2) {
		/* Sooner deadline means higher effective priority.
		 * Doing the calculation with unsigned types and casting
		 * to signed isn't perfect, but at least reduces this
		 * from UB on overflow to impdef.
		 */
		أرجع (int32_t)(d2 - d1);
	}
#جے_انت /* CONFIG_SCHED_DEADLINE */
	أرجع 0;
}

ساكن ALWAYS_INLINE فراغ z_priq_simple_جوڑو(sys_dlist_t *pq, بنية k_thread *thread)
{
	بنية k_thread *t;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, t, ادھار.qnode_dlist) {
		إذا (z_sched_prio_cmp(thread, t) > 0) {
			sys_dlist_داخل_کرو(&t->ادھار.qnode_dlist, &thread->ادھار.qnode_dlist);
			أرجع;
		}
	}

	sys_dlist_append(pq, &thread->ادھار.qnode_dlist);
}

ساكن ALWAYS_INLINE فراغ z_priq_simple_remove(sys_dlist_t *pq, بنية k_thread *thread)
{
	ARG_بن_ورتیا(pq);

	sys_dlist_remove(&thread->ادھار.qnode_dlist);
}

ساكن ALWAYS_INLINE فراغ z_priq_simple_yield(sys_dlist_t *pq)
{
#جے_بنا_تعریف CONFIG_SMP
	sys_dnode_t *n;

	n = sys_dlist_peek_اگلا_no_check(pq, &_current->ادھار.qnode_dlist);

	sys_dlist_قطار_توں_کڈھو(&_current->ادھار.qnode_dlist);

	بنية k_thread *t;

	/*
	 * As it is possible that the current thread was not at the head of
	 * the run queue, start searching from the present position for where
	 * to re-insert it.
	 */

	طالما (n != عدم) {
		t = CONTAINER_دا(n, بنية k_thread, ادھار.qnode_dlist);
		إذا (z_sched_prio_cmp(_current, t) > 0) {
			sys_dlist_داخل_کرو(&t->ادھار.qnode_dlist,
					 &_current->ادھار.qnode_dlist);
			أرجع;
		}
		n = sys_dlist_peek_اگلا_no_check(pq, n);
	}

	sys_dlist_append(pq, &_current->ادھار.qnode_dlist);
#جے_انت
}

ساكن ALWAYS_INLINE بنية k_thread *z_priq_simple_بہترین(sys_dlist_t *pq)
{
	بنية k_thread *thread = عدم;
	sys_dnode_t *n = sys_dlist_peek_سر(pq);

	إذا (n != عدم) {
		thread = CONTAINER_دا(n, بنية k_thread, ادھار.qnode_dlist);
	}
	أرجع thread;
}

#جے_تعریف_شدہ CONFIG_SCHED_CPU_نقاب
ساكن ALWAYS_INLINE بنية k_thread *z_priq_simple_نقاب_بہترین(sys_dlist_t *pq)
{
	/* With masks enabled we need to be prepared to walk the list
	 * looking for one we can run
	 */
	بنانشانی لما current_cpu_ثنائی_ہندسہ = ثنائی_ہندسہ(_current_cpu->id);
	بنية k_thread *thread;

	SYS_DLIST_FOR_EACH_CONTAINER(pq, thread, ادھار.qnode_dlist) {
		إذا ((thread->ادھار.cpu_نقاب & current_cpu_ثنائی_ہندسہ) != 0) {
			أرجع thread;
		}
	}
	أرجع عدم;
}
#جے_انت /* CONFIG_SCHED_CPU_MASK */

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

ساكن ALWAYS_INLINE فراغ z_priq_rb_جوڑو(بنية _priq_rb *pq, بنية k_thread *thread)
{
	بنية k_thread *t;

	thread->ادھار.ترتیب_کلید = pq->اگلا_ترتیب_کلید;
	++pq->اگلا_ترتیب_کلید;

	/* Renumber at wraparound.  This is tiny code, and in practice
	 * will almost never be hit on real systems.  BUT on very
	 * long-running systems where a priq never completely empties
	 * AND that contains very large numbers of threads, it can be
	 * a latency glitch to loop over all the threads like this.
	 */
	إذا (!pq->اگلا_ترتیب_کلید) {
		RB_FOR_EACH_CONTAINER(&pq->درخت, t, ادھار.qnode_rb) {
			t->ادھار.ترتیب_کلید = pq->اگلا_ترتیب_کلید;
			++pq->اگلا_ترتیب_کلید;
		}
	}

	rb_داخل_کرو(&pq->درخت, &thread->ادھار.qnode_rb);
}

ساكن ALWAYS_INLINE فراغ z_priq_rb_remove(بنية _priq_rb *pq, بنية k_thread *thread)
{
	rb_remove(&pq->درخت, &thread->ادھار.qnode_rb);

	إذا (!pq->درخت.جڑ) {
		pq->اگلا_ترتیب_کلید = 0;
	}
}

ساكن ALWAYS_INLINE فراغ z_priq_rb_yield(بنية _priq_rb *pq)
{
#جے_بنا_تعریف CONFIG_SMP
	z_priq_rb_remove(pq, _current);
	z_priq_rb_جوڑو(pq, _current);
#جے_انت
}

ساكن ALWAYS_INLINE بنية k_thread *z_priq_rb_بہترین(بنية _priq_rb *pq)
{
	بنية k_thread *thread = عدم;
	بنية rbnode *n = rb_get_min(&pq->درخت);

	إذا (n != عدم) {
		thread = CONTAINER_دا(n, بنية k_thread, ادھار.qnode_rb);
	}
	أرجع thread;
}

#جے_تعریف_شدہ CONFIG_SCHED_CPU_نقاب
ساكن ALWAYS_INLINE بنية k_thread *z_priq_rb_نقاب_بہترین(بنية _priq_rb *pq)
{
	بنانشانی لما current_cpu_ثنائی_ہندسہ = ثنائی_ہندسہ(_current_cpu->id);
	بنية k_thread *thread;

	RB_FOR_EACH_CONTAINER(&pq->درخت, thread, ادھار.qnode_rb) {
		إذا ((thread->ادھار.cpu_نقاب & current_cpu_ثنائی_ہندسہ) != 0) {
			أرجع thread;
		}
	}
	أرجع عدم;
}
#جے_انت /* CONFIG_SCHED_CPU_MASK */
#جے_انت /* CONFIG_SCHED_SCALABLE || CONFIG_WAITQ_SCALABLE */

بنية prio_info {
	uint8_t وتھ_prio;
	uint8_t idx;
	uint8_t ثنائی_ہندسہ_2;
};

ساكن ALWAYS_INLINE بنية prio_info get_prio_info(int8_t old_prio)
{
	بنية prio_info ret;

	ret.وتھ_prio = old_prio - K_HIGHEST_THREAD_PRIO;
	ret.idx = ret.وتھ_prio / NBITS;
	ret.ثنائی_ہندسہ_2 = ret.وتھ_prio % NBITS;

	أرجع ret;
}

ساكن ALWAYS_INLINE بنانشانی صحيح z_priq_mq_بہترین_قطار_اشاریہ(بنية _priq_mq *pq)
{
	بنانشانی صحيح i = 0;

	افعل {
		إذا (likely(pq->bitmask[i])) {
			أرجع i * NBITS + TRAILING_ZEROS(pq->bitmask[i]);
		}
		i++;
	} طالما (i < PRIQ_BITMAP_ناپ);

	أرجع K_عدد_THREAD_PRIO - 1;
}

ساكن ALWAYS_INLINE فراغ z_priq_mq_init(بنية _priq_mq *q)
{
	لكل (size_t i = 0; i < ARRAY_ناپ(q->queues); i++) {
		sys_dlist_init(&q->queues[i]);
	}

#جے_بنا_تعریف CONFIG_SMP
	q->cached_قطار_اشاریہ = K_عدد_THREAD_PRIO - 1;
#جے_انت
}

ساكن ALWAYS_INLINE فراغ z_priq_mq_جوڑو(بنية _priq_mq *pq,
					بنية k_thread *thread)
{
	بنية prio_info pos = get_prio_info(thread->ادھار.prio);

	sys_dlist_append(&pq->queues[pos.وتھ_prio], &thread->ادھار.qnode_dlist);
	pq->bitmask[pos.idx] |= ثنائی_ہندسہ(pos.ثنائی_ہندسہ_2);

#جے_بنا_تعریف CONFIG_SMP
	إذا (pos.وتھ_prio < pq->cached_قطار_اشاریہ) {
		pq->cached_قطار_اشاریہ = pos.وتھ_prio;
	}
#جے_انت
}

ساكن ALWAYS_INLINE فراغ z_priq_mq_remove(بنية _priq_mq *pq,
					   بنية k_thread *thread)
{
	بنية prio_info pos = get_prio_info(thread->ادھار.prio);

	sys_dlist_قطار_توں_کڈھو(&thread->ادھار.qnode_dlist);
	إذا (unlikely(sys_dlist_is_خالی_تھاواں(&pq->queues[pos.وتھ_prio]))) {
		pq->bitmask[pos.idx] &= ~ثنائی_ہندسہ(pos.ثنائی_ہندسہ_2);
#جے_بنا_تعریف CONFIG_SMP
		pq->cached_قطار_اشاریہ = z_priq_mq_بہترین_قطار_اشاریہ(pq);
#جے_انت
	}
}

ساكن ALWAYS_INLINE فراغ z_priq_mq_yield(بنية _priq_mq *pq)
{
#جے_بنا_تعریف CONFIG_SMP
	بنية prio_info pos = get_prio_info(_current->ادھار.prio);

	sys_dlist_قطار_توں_کڈھو(&_current->ادھار.qnode_dlist);
	sys_dlist_append(&pq->queues[pos.وتھ_prio],
			 &_current->ادھار.qnode_dlist);
#جے_انت
}

ساكن ALWAYS_INLINE بنية k_thread *z_priq_mq_بہترین(بنية _priq_mq *pq)
{
#جے_تعریف_شدہ CONFIG_SMP
	بنانشانی صحيح اشاریہ = z_priq_mq_بہترین_قطار_اشاریہ(pq);
#وإلا
	بنانشانی صحيح اشاریہ = pq->cached_قطار_اشاریہ;
#جے_انت

	sys_dnode_t *n = sys_dlist_peek_سر(&pq->queues[اشاریہ]);

	إذا (likely(n != عدم)) {
		أرجع CONTAINER_دا(n, بنية k_thread, ادھار.qnode_dlist);
	}

	أرجع عدم;
}

#جے_تعریف_شدہ CONFIG_SCHED_CPU_نقاب
ساكن ALWAYS_INLINE بنية k_thread *z_priq_mq_نقاب_بہترین(بنية _priq_mq *pq)
{
	بنانشانی لما current_cpu_ثنائی_ہندسہ = ثنائی_ہندسہ(_current_cpu->id);

	/* Walk priority levels from highest to lowest, and within each
	 * level walk the thread list to find the first thread runnable
	 * on the current CPU.
	 */
	لكل (بنانشانی صحيح i = 0U; i < PRIQ_BITMAP_ناپ; i++) {
		بنانشانی لما bitmask = pq->bitmask[i];

		طالما (bitmask != 0UL) {
			بنانشانی صحيح ثنائی_ہندسہ_2 = TRAILING_ZEROS(bitmask);
			بنانشانی صحيح اشاریہ = i * NBITS + ثنائی_ہندسہ_2;
			بنية k_thread *thread;

			SYS_DLIST_FOR_EACH_CONTAINER(&pq->queues[اشاریہ],
						     thread, ادھار.qnode_dlist) {
				إذا ((thread->ادھار.cpu_نقاب &
				     current_cpu_ثنائی_ہندسہ) != 0) {
					أرجع thread;
				}
			}
			bitmask &= (bitmask - 1UL);
		}
	}
	أرجع عدم;
}
#جے_انت /* CONFIG_SCHED_CPU_MASK */

#جے_انت /* ZEPHYR_KERNEL_INCLUDE_PRIORITY_Q_H_ */
