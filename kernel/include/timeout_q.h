/*
 * Copyright (c) 2015 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अगर_अघोषित ZEPHYR_KERNEL_INCLUDE_TIMEOUT_Q_H_
#घोषणा ZEPHYR_KERNEL_INCLUDE_TIMEOUT_Q_H_

/**
 * @file
 * @brief timeout queue for threads on kernel objects
 */

#समावेश <zephyr/kernel.h>

#समावेश <stdbool.h>

#अगर_घोषित __cplusplus
बाह्य "C" {
#पूर्ण_अगर

#अगर_घोषित CONFIG_SYS_CLOCK_EXISTS

/*
 * Per-node timeout helpers (Kconfig choice TIMEOUT_BACKEND).
 *
 * z_init_timeout() and z_is_inactive_timeout() operate on a single
 * struct _timeout and are needed tree-wide (timer.c, work.c, poll.c, ...),
 * so they live here and depend only on the chosen backend's struct _timeout
 * fields (see kernel_structs.h). The queue itself (the z_timeout_q_*()
 * operations and the backend instance) is private to kernel/timeout.c, which
 * includes the matching backend implementation header directly.
 */
#अगर defined(CONFIG_TIMEOUT_BACKEND_MINHEAP)

अटल अंतरभूत व्योम z_init_timeout(काष्ठा _timeout *to)
{
	to->heap_handle.idx = 0U;
	to->abs_ticks = 0;
}

अटल अंतरभूत bool z_is_inactive_timeout(स्थिर काष्ठा _timeout *to)
{
	वापस to->heap_handle.idx == 0U;
}

#या_अगर defined(CONFIG_TIMEOUT_BACKEND_WHEEL)

अटल अंतरभूत व्योम z_init_timeout(काष्ठा _timeout *to)
{
	sys_dnode_init(&to->गाँठ);
	to->flags = 0U;
	to->dticks = 0;
}

अटल अंतरभूत bool z_is_inactive_timeout(स्थिर काष्ठा _timeout *to)
{
	वापस !sys_dnode_is_linked(&to->गाँठ);
}

#या_अगर defined(CONFIG_TIMEOUT_BACKEND_SKIPLIST)

अटल अंतरभूत व्योम z_init_timeout(काष्ठा _timeout *to)
{
	uint8_t i;

	to->height = 0;
	to->abs_ticks = 0;

	क्रम (i = 0; i < CONFIG_TIMEOUT_SKIPLIST_MAX_LEVEL; i++) {
		to->forward[i] = शून्य;
	}
}

अटल अंतरभूत bool z_is_inactive_timeout(स्थिर काष्ठा _timeout *to)
{
	वापस to->height == 0;
}

#अन्यथा /* CONFIG_TIMEOUT_BACKEND_DLIST or _BUCKET */

अटल अंतरभूत व्योम z_init_timeout(काष्ठा _timeout *to)
{
	sys_dnode_init(&to->गाँठ);
	to->dticks = 0;
}

अटल अंतरभूत bool z_is_inactive_timeout(स्थिर काष्ठा _timeout *to)
{
	वापस !sys_dnode_is_linked(&to->गाँठ);
}

#पूर्ण_अगर

/* Adds the timeout to the queue.
 *
 * @return Absolute tick value when timeout will expire.
 */
k_ticks_t z_add_timeout(काष्ठा _timeout *to, _timeout_func_t fn, k_timeout_t timeout);

/* Attempt to abort a timeout. Safe to use for callers that need to be
 * certain the handler is not in flight (e.g. when about to free the
 * timeout's storage), as long as the caller is prepared to drop any
 * locks the handler may need and retry.
 *
 * Returns:
 *   0       — the timeout was active and has been removed from the queue.
 *   -EINVAL — the timeout was not active (never linked, already done, or
 *             a same-CPU IRQ aborted while the handler was paused on this
 *             CPU; in that case the in-flight slot is marked superseded
 *             so handlers that need to bail may check via
 *             z_timeout_inflight_superseded()).
 *   -EAGAIN — the handler is currently in flight on another CPU. The caller
 *             must drop any lock that the handler may need and retry. A short
 *             arch_spin_relax() has already been done before this return.
 *
 * On -EAGAIN, the only correct response is to drop outer locks and call again.
 * After a non-EAGAIN return, the handler is guaranteed not to be in flight.
 */
पूर्णांक z_try_abort_timeout(काष्ठा _timeout *to);

/* True if @to is the currently in-flight timeout and a same-CPU aborter
 * has marked the slot as superseded. Handlers with non-idempotent side
 * effects (e.g. k_timer's expiry_fn) should call this at entry, after
 * taking their own lock, and bail if it returns true.
 */
bool z_timeout_inflight_superseded(स्थिर काष्ठा _timeout *to);

अटल अंतरभूत व्योम z_init_thread_timeout(काष्ठा _thread_base *thread_base)
{
	z_init_timeout(&thread_base->timeout);
}

बाह्य व्योम z_thread_timeout(काष्ठा _timeout *timeout);

अटल अंतरभूत k_ticks_t z_add_thread_timeout(काष्ठा k_thread *thread, k_timeout_t ticks)
{
	वापस z_add_timeout(&thread->base.timeout, z_thread_timeout, ticks);
}

अटल अंतरभूत पूर्णांक z_try_abort_thread_timeout(काष्ठा k_thread *thread)
{
	वापस z_try_abort_timeout(&thread->base.timeout);
}

uint32_t z_get_अग्र_timeout_expiry(व्योम);

k_ticks_t z_timeout_remaining(स्थिर काष्ठा _timeout *timeout);

#अन्यथा

/* Stubs when !CONFIG_SYS_CLOCK_EXISTS */
#घोषणा z_init_thread_timeout(thread_base) करो {} जबतक (false)
#घोषणा z_try_abort_thread_timeout(to) 0
#घोषणा z_is_inactive_timeout(to) 1
#घोषणा z_get_अग्र_timeout_expiry() ((uint32_t) K_TICKS_FOREVER)
#घोषणा z_set_timeout_expiry(ticks, is_idle) करो {} जबतक (false)

अटल अंतरभूत k_ticks_t z_add_thread_timeout(काष्ठा k_thread *thread, k_timeout_t ticks)
{
	ARG_UNUSED(thread);
	ARG_UNUSED(ticks);
	वापस 0;
}

#पूर्ण_अगर /* CONFIG_SYS_CLOCK_EXISTS */

#अगर_घोषित __cplusplus
}
#पूर्ण_अगर

#पूर्ण_अगर /* ZEPHYR_KERNEL_INCLUDE_TIMEOUT_Q_H_ */
