/*
 * Copyright (c) 2010-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 *
 * @brief dynamic-size QUEUE object.
 */


#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>

#ಅನ್ತರ್ಭಾವ <zephyr/toolchain.h>
#ಅನ್ತರ್ಭಾವ <wait_q.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <scheduler.h>
#ಅನ್ತರ್ಭಾವ <zephyr/init.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>
#ಅನ್ತರ್ಭಾವ <kernel_internal.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/check.h>

BUILD_ASSERT(SYS_SFLIST_FLAG_ಬಿಟ್ಟುಗಳು >= 1, "k_queue needs one sflist flag bit");

ರಚನೆ alloc_ಗಂಟು {
	sys_sfnode_t ಗಂಟು;
	ಶೂನ್ಯ *ದತ್ತಾಂಶ;
};

/* The queue must have its spinlock held before calling this function. */
ಸ್ಥಿರ ಶೂನ್ಯ *z_ಸರತಿ_ಗಂಟು_peek(sys_sfnode_t *ಗಂಟು, bool needs_free)
{
	ಶೂನ್ಯ *ret;

	ಆದರೆ ((ಗಂಟು != ಖಾಲಿ) && (sys_sfnode_flags_get(ಗಂಟು) != (uint8_t)0)) {
		/* If the flag is set, then the enqueue operation for this item
		 * did a behind-the scenes memory allocation of an alloc_node
		 * struct, which is what got put in the queue. Free it and pass
		 * back the data pointer.
		 */
		ರಚನೆ alloc_ಗಂಟು *anode;

		anode = CONTAINER_ದ(ಗಂಟು, ರಚನೆ alloc_ಗಂಟು, ಗಂಟು);
		ret = anode->ದತ್ತಾಂಶ;
		ಆದರೆ (needs_free) {
			k_free(anode);
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Data was directly placed in the queue, the first word
		 * reserved for the linked list. User mode isn't allowed to
		 * do this, although it can get data sent this way.
		 */
		ret = (ಶೂನ್ಯ *)ಗಂಟು;
	}

	ಹಿಂತಿರುಗಿ ret;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_ಸರತಿ
/* The queues embedded in statically defined FIFOs and LIFOs are permanent
 * objects of the queue type too.
 */
BUILD_ASSERT(offsetof(ರಚನೆ k_fifo, ಸರತಿ_2) == 0);
BUILD_ASSERT(offsetof(ರಚನೆ k_lifo, ಸರತಿ_2) == 0);
STRUCT_SECTION_ಆರಂಭ_EXTERN(k_fifo);
STRUCT_SECTION_ಕೊನೆ_EXTERN(k_fifo);
STRUCT_SECTION_ಆರಂಭ_EXTERN(k_lifo);
STRUCT_SECTION_ಕೊನೆ_EXTERN(k_lifo);
STRUCT_SECTION_ಆರಂಭ_EXTERN(k_ಸರತಿ);
STRUCT_SECTION_ಕೊನೆ_EXTERN(k_ಸರತಿ);
K_OBJ_TYPE_DEFINE_RANGES(obj_type_ಸರತಿ, k_ಸರತಿ, K_OBJ_TYPE_ಸರತಿ_ID, ಖಾಲಿ,
			 K_OBJ_RANGE_SECTION(k_ಸರತಿ), K_OBJ_RANGE_SECTION(k_fifo),
			 K_OBJ_RANGE_SECTION(k_lifo));
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_QUEUE */

ಶೂನ್ಯ z_impl_k_ಸರತಿ_init(ರಚನೆ k_ಸರತಿ *ಸರತಿ)
{
	sys_sflist_init(&ಸರತಿ->ದತ್ತಾಂಶ_q);
	ಸರತಿ->ಬೀಗ = (ರಚನೆ k_spinlock) {};
	z_waitq_init(&ಸರತಿ->wait_q);
#ಆದರೆ defined(CONFIG_POLL)
	sys_dlist_init(&ಸರತಿ->poll_events);
#ಆದರೆ_ಅಂತ್ಯ

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_INIT(k_ಸರತಿ, ಸರತಿ);

	k_object_init(ಸರತಿ);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_ಸರತಿ
	k_obj_core_init_and_link(K_OBJ_CORE(ಸರತಿ), &obj_type_ಸರತಿ);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_QUEUE */
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_vrfy_k_ಸರತಿ_init(ರಚನೆ k_ಸರತಿ *ಸರತಿ)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(ಸರತಿ, K_OBJ_ಸರತಿ));
	z_impl_k_ಸರತಿ_init(ಸರತಿ);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_queue_init_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool ಸರತಿ_handle_poll_events(ರಚನೆ k_ಸರತಿ *ಸರತಿ, uint32_t state)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_POLL
	ಹಿಂತಿರುಗಿ z_handle_obj_poll_events(&ಸರತಿ->poll_events, state);
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(ಸರತಿ);
	ARG_ಬಳಸದ(state);

	ಹಿಂತಿರುಗಿ false;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_POLL */
}

ಶೂನ್ಯ z_impl_k_ಸರತಿ_cancel_wait(ರಚನೆ k_ಸರತಿ *ಸರತಿ)
{
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC(k_ಸರತಿ, cancel_wait, ಸರತಿ);

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&ಸರತಿ->ಬೀಗ);
	bool resched = false;

	ಆದರೆ (z_sched_wake(&ಸರತಿ->wait_q, 0, ಖಾಲಿ)) {
		resched = true;
	}

	resched = ಸರತಿ_handle_poll_events(ಸರತಿ, K_POLL_STATE_CANCELLED) || resched;

	ಆದರೆ (resched) {
		z_reschedule(&ಸರತಿ->ಬೀಗ, ಕೀಲಿ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		k_spin_unlock(&ಸರತಿ->ಬೀಗ, ಕೀಲಿ);
	}
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_vrfy_k_ಸರತಿ_cancel_wait(ರಚನೆ k_ಸರತಿ *ಸರತಿ)
{
	K_OOPS(K_SYSCALL_OBJ(ಸರತಿ, K_OBJ_ಸರತಿ));
	z_impl_k_ಸರತಿ_cancel_wait(ಸರತಿ);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_queue_cancel_wait_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

/*
 * Common code for inserting a node into the queue.
 * The queue's spinlock must be held prior to entry.
 * However, that same spinlock will be unlocked before it returns.
 */
ಸ್ಥಿರ int32_t ಸರತಿ_ಸೇರಿಸು(ರಚನೆ k_ಸರತಿ *ಸರತಿ, ಶೂನ್ಯ *prev, ಶೂನ್ಯ *ದತ್ತಾಂಶ,
			    bool alloc, bool is_append, k_spinlock_ಕೀಲಿ_t ಕೀಲಿ)
{
	int32_t ಫಲಿತಾಂಶ = 0;
	bool resched = false;

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_ಸರತಿ, ಸರತಿ_ಸೇರಿಸು, ಸರತಿ, alloc);

	ಆದರೆ (is_append) {
		prev = sys_sflist_peek_ಬಾಲ(&ಸರತಿ->ದತ್ತಾಂಶ_q);
	}

	ಆದರೆ (z_sched_wake(&ಸರತಿ->wait_q, 0, ದತ್ತಾಂಶ)) {
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_BLOCKING(k_ಸರತಿ, ಸರತಿ_ಸೇರಿಸು, ಸರತಿ, alloc, K_FOREVER);
		resched = true;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	/* Only need to actually allocate if no threads are pending */
	ಆದರೆ (alloc) {
		ರಚನೆ alloc_ಗಂಟು *anode;

		anode = z_thread_malloc(ಗಾತ್ರ(*anode));
		ಆದರೆ (anode == ಖಾಲಿ) {
			ಫಲಿತಾಂಶ = -ENOMEM;
			ಹೋಗು ಹೊರಹೋಗು;
		}
		anode->ದತ್ತಾಂಶ = ದತ್ತಾಂಶ;
		sys_sfnode_init(&anode->ಗಂಟು, 0x1);
		ದತ್ತಾಂಶ = anode;
	} ಇಲ್ಲದಿದ್ದರೆ {
		sys_sfnode_init(ದತ್ತಾಂಶ, 0x0);
	}

	sys_sflist_ಸೇರಿಸು(&ಸರತಿ->ದತ್ತಾಂಶ_q, prev, ದತ್ತಾಂಶ);
	resched = ಸರತಿ_handle_poll_events(ಸರತಿ, K_POLL_STATE_ದತ್ತಾಂಶ_ಲಭ್ಯ);

ಹೊರಹೋಗು:
	ಆದರೆ (resched) {
		z_reschedule(&ಸರತಿ->ಬೀಗ, ಕೀಲಿ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		k_spin_unlock(&ಸರತಿ->ಬೀಗ, ಕೀಲಿ);
	}

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ಸರತಿ, ಸರತಿ_ಸೇರಿಸು, ಸರತಿ, alloc, ಫಲಿತಾಂಶ);

	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

ಶೂನ್ಯ k_ಸರತಿ_ಸೇರಿಸು(ರಚನೆ k_ಸರತಿ *ಸರತಿ, ಶೂನ್ಯ *prev, ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_ಸರತಿ, ಸೇರಿಸು, ಸರತಿ);

	(ಶೂನ್ಯ)ಸರತಿ_ಸೇರಿಸು(ಸರತಿ, prev, ದತ್ತಾಂಶ, false, false,
			   k_spin_ಬೀಗ(&ಸರತಿ->ಬೀಗ));

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ಸರತಿ, ಸೇರಿಸು, ಸರತಿ);
}

ಶೂನ್ಯ k_ಸರತಿ_append(ರಚನೆ k_ಸರತಿ *ಸರತಿ, ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_ಸರತಿ, append, ಸರತಿ);

	(ಶೂನ್ಯ)ಸರತಿ_ಸೇರಿಸು(ಸರತಿ, ಖಾಲಿ, ದತ್ತಾಂಶ, false, true,
			   k_spin_ಬೀಗ(&ಸರತಿ->ಬೀಗ));

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ಸರತಿ, append, ಸರತಿ);
}

ಶೂನ್ಯ k_ಸರತಿ_prepend(ರಚನೆ k_ಸರತಿ *ಸರತಿ, ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_ಸರತಿ, prepend, ಸರತಿ);

	(ಶೂನ್ಯ)ಸರತಿ_ಸೇರಿಸು(ಸರತಿ, ಖಾಲಿ, ದತ್ತಾಂಶ, false, false,
			   k_spin_ಬೀಗ(&ಸರತಿ->ಬೀಗ));

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ಸರತಿ, prepend, ಸರತಿ);
}

int32_t z_impl_k_ಸರತಿ_alloc_append(ರಚನೆ k_ಸರತಿ *ಸರತಿ, ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_ಸರತಿ, alloc_append, ಸರತಿ);

	int32_t ret = ಸರತಿ_ಸೇರಿಸು(ಸರತಿ, ಖಾಲಿ, ದತ್ತಾಂಶ, true, true,
				   k_spin_ಬೀಗ(&ಸರತಿ->ಬೀಗ));

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ಸರತಿ, alloc_append, ಸರತಿ, ret);

	ಹಿಂತಿರುಗಿ ret;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ int32_t z_vrfy_k_ಸರತಿ_alloc_append(ರಚನೆ k_ಸರತಿ *ಸರತಿ,
						  ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
	K_OOPS(K_SYSCALL_OBJ(ಸರತಿ, K_OBJ_ಸರತಿ));
	ಹಿಂತಿರುಗಿ z_impl_k_ಸರತಿ_alloc_append(ಸರತಿ, ದತ್ತಾಂಶ);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_queue_alloc_append_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

int32_t z_impl_k_ಸರತಿ_alloc_prepend(ರಚನೆ k_ಸರತಿ *ಸರತಿ, ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_ಸರತಿ, alloc_prepend, ಸರತಿ);

	int32_t ret = ಸರತಿ_ಸೇರಿಸು(ಸರತಿ, ಖಾಲಿ, ದತ್ತಾಂಶ, true, false,
				   k_spin_ಬೀಗ(&ಸರತಿ->ಬೀಗ));

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ಸರತಿ, alloc_prepend, ಸರತಿ, ret);

	ಹಿಂತಿರುಗಿ ret;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ int32_t z_vrfy_k_ಸರತಿ_alloc_prepend(ರಚನೆ k_ಸರತಿ *ಸರತಿ,
						   ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
	K_OOPS(K_SYSCALL_OBJ(ಸರತಿ, K_OBJ_ಸರತಿ));
	ಹಿಂತಿರುಗಿ z_impl_k_ಸರತಿ_alloc_prepend(ಸರತಿ, ದತ್ತಾಂಶ);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_queue_alloc_prepend_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಪೂರ್ಣಾಂಕ k_ಸರತಿ_append_list(ರಚನೆ k_ಸರತಿ *ಸರತಿ, ಶೂನ್ಯ *ತಲೆ, ಶೂನ್ಯ *ಬಾಲ)
{
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_ಸರತಿ, append_list, ಸರತಿ);
	bool resched = false;

	/* invalid head or tail of list */
	CHECKIF((ತಲೆ == ಖಾಲಿ) || (ಬಾಲ == ಖಾಲಿ)) {
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ಸರತಿ, append_list, ಸರತಿ, -EINVAL);

		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&ಸರತಿ->ಬೀಗ);

	ಆಗ ((ತಲೆ != ಖಾಲಿ) && z_sched_wake(&ಸರತಿ->wait_q, 0, ತಲೆ)) {
		resched = true;
		ತಲೆ = *(ಶೂನ್ಯ **)ತಲೆ;
	}

	ಆದರೆ (ತಲೆ != ಖಾಲಿ) {
		sys_sflist_append_list(&ಸರತಿ->ದತ್ತಾಂಶ_q, ತಲೆ, ಬಾಲ);

		resched = ಸರತಿ_handle_poll_events(ಸರತಿ, K_POLL_STATE_ದತ್ತಾಂಶ_ಲಭ್ಯ) || resched;
	}

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ಸರತಿ, append_list, ಸರತಿ, 0);

	ಆದರೆ (resched) {
		z_reschedule(&ಸರತಿ->ಬೀಗ, ಕೀಲಿ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		k_spin_unlock(&ಸರತಿ->ಬೀಗ, ಕೀಲಿ);
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ k_ಸರತಿ_ವಿಲೀನ_slist(ರಚನೆ k_ಸರತಿ *ಸರತಿ, sys_slist_t *list)
{
	ಪೂರ್ಣಾಂಕ ret;

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_ಸರತಿ, ವಿಲೀನ_slist, ಸರತಿ);

	/* list must not be empty */
	CHECKIF(sys_slist_is_ಖಾಲಿಗಳು(list)) {
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ಸರತಿ, ವಿಲೀನ_slist, ಸರತಿ, -EINVAL);

		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	/*
	 * note: this works as long as:
	 * - the slist implementation keeps the next pointer as the first
	 *   field of the node object type
	 * - list->tail->next = NULL.
	 * - sflist implementation only differs from slist by stuffing
	 *   flag bytes in the lower order bits of the data pointer
	 * - source list is really an slist and not an sflist with flags set
	 */
	ret = k_ಸರತಿ_append_list(ಸರತಿ, list->ತಲೆ, list->ಬಾಲ);
	CHECKIF(ret != 0) {
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ಸರತಿ, ವಿಲೀನ_slist, ಸರತಿ, ret);

		ಹಿಂತಿರುಗಿ ret;
	}
	sys_slist_init(list);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ಸರತಿ, ವಿಲೀನ_slist, ಸರತಿ, 0);

	ಹಿಂತಿರುಗಿ 0;
}

ಶೂನ್ಯ *z_impl_k_ಸರತಿ_get(ರಚನೆ k_ಸರತಿ *ಸರತಿ, k_timeout_t timeout)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&ಸರತಿ->ಬೀಗ);
	ಶೂನ್ಯ *ದತ್ತಾಂಶ;

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_ಸರತಿ, get, ಸರತಿ, timeout);

	ಆದರೆ (likely(!sys_sflist_is_ಖಾಲಿಗಳು(&ಸರತಿ->ದತ್ತಾಂಶ_q))) {
		sys_sfnode_t *ಗಂಟು;

		ಗಂಟು = sys_sflist_get_not_ಖಾಲಿಗಳು(&ಸರತಿ->ದತ್ತಾಂಶ_q);
		ದತ್ತಾಂಶ = z_ಸರತಿ_ಗಂಟು_peek(ಗಂಟು, true);
		k_spin_unlock(&ಸರತಿ->ಬೀಗ, ಕೀಲಿ);

		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ಸರತಿ, get, ಸರತಿ, timeout, ದತ್ತಾಂಶ);

		ಹಿಂತಿರುಗಿ ದತ್ತಾಂಶ;
	}

	ಆದರೆ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&ಸರತಿ->ಬೀಗ, ಕೀಲಿ);

		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ಸರತಿ, get, ಸರತಿ, timeout, ಖಾಲಿ);

		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_BLOCKING(k_ಸರತಿ, get, ಸರತಿ, timeout);

	ಪೂರ್ಣಾಂಕ ret = z_pend_curr(&ಸರತಿ->ಬೀಗ, ಕೀಲಿ, &ಸರತಿ->wait_q, timeout);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ಸರತಿ, get, ಸರತಿ, timeout,
		(ret != 0) ? ಖಾಲಿ : _current->ತಳ.ಅದಲುಬದಲು_ದತ್ತಾಂಶ);

	ಹಿಂತಿರುಗಿ (ret != 0) ? ಖಾಲಿ : _current->ತಳ.ಅದಲುಬದಲು_ದತ್ತಾಂಶ;
}

/* Remove a queue item by its data pointer and free any wrapper node. */
bool k_ಸರತಿ_remove(ರಚನೆ k_ಸರತಿ *ಸರತಿ, ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_ಸರತಿ, remove, ಸರತಿ);
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&ಸರತಿ->ಬೀಗ);
	sys_sfnode_t *prev = ಖಾಲಿ;
	sys_sfnode_t *ಗಂಟು = sys_sflist_peek_ತಲೆ(&ಸರತಿ->ದತ್ತಾಂಶ_q);
	bool ret = false;

	ಆಗ (ಗಂಟು != ಖಾಲಿ) {
		ಶೂನ್ಯ *peeked = z_ಸರತಿ_ಗಂಟು_peek(ಗಂಟು, false);

		ಆದರೆ (peeked == ದತ್ತಾಂಶ) {
			sys_sflist_remove(&ಸರತಿ->ದತ್ತಾಂಶ_q, prev, ಗಂಟು);
			(ಶೂನ್ಯ)z_ಸರತಿ_ಗಂಟು_peek(ಗಂಟು, true);
			ret = true;
			ನಿಲ್ಲಿಸು;
		}
		prev = ಗಂಟು;
		ಗಂಟು = sys_sflist_peek_ಮುಂದಿನ(ಗಂಟು);
	}

	k_spin_unlock(&ಸರತಿ->ಬೀಗ, ಕೀಲಿ);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ಸರತಿ, remove, ಸರತಿ, ret);

	ಹಿಂತಿರುಗಿ ret;
}

bool k_ಸರತಿ_unique_append(ರಚನೆ k_ಸರತಿ *ಸರತಿ, ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_ಸರತಿ, unique_append, ಸರತಿ);
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&ಸರತಿ->ಬೀಗ);

	sys_sfnode_t *test;

	SYS_SFLIST_FOR_EACH_ಗಂಟು(&ಸರತಿ->ದತ್ತಾಂಶ_q, test) {
		ಆದರೆ (test == (sys_sfnode_t *) ದತ್ತಾಂಶ) {
			k_spin_unlock(&ಸರತಿ->ಬೀಗ, ಕೀಲಿ);
			SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ಸರತಿ, unique_append, ಸರತಿ, false);

			ಹಿಂತಿರುಗಿ false;
		}
	}

	(ಶೂನ್ಯ)ಸರತಿ_ಸೇರಿಸು(ಸರತಿ, ಖಾಲಿ, ದತ್ತಾಂಶ, false, true, ಕೀಲಿ);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ಸರತಿ, unique_append, ಸರತಿ, true);

	ಹಿಂತಿರುಗಿ true;
}

ಶೂನ್ಯ *z_impl_k_ಸರತಿ_peek_ತಲೆ(ರಚನೆ k_ಸರತಿ *ಸರತಿ)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&ಸರತಿ->ಬೀಗ);
	ಶೂನ್ಯ *ret = z_ಸರತಿ_ಗಂಟು_peek(sys_sflist_peek_ತಲೆ(&ಸರತಿ->ದತ್ತಾಂಶ_q), false);

	k_spin_unlock(&ಸರತಿ->ಬೀಗ, ಕೀಲಿ);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC(k_ಸರತಿ, peek_ತಲೆ, ಸರತಿ, ret);

	ಹಿಂತಿರುಗಿ ret;
}

ಶೂನ್ಯ *z_impl_k_ಸರತಿ_peek_ಬಾಲ(ರಚನೆ k_ಸರತಿ *ಸರತಿ)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&ಸರತಿ->ಬೀಗ);
	ಶೂನ್ಯ *ret = z_ಸರತಿ_ಗಂಟು_peek(sys_sflist_peek_ಬಾಲ(&ಸರತಿ->ದತ್ತಾಂಶ_q), false);

	k_spin_unlock(&ಸರತಿ->ಬೀಗ, ಕೀಲಿ);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC(k_ಸರತಿ, peek_ಬಾಲ, ಸರತಿ, ret);

	ಹಿಂತಿರುಗಿ ret;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *z_vrfy_k_ಸರತಿ_get(ರಚನೆ k_ಸರತಿ *ಸರತಿ,
				       k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(ಸರತಿ, K_OBJ_ಸರತಿ));
	ಹಿಂತಿರುಗಿ z_impl_k_ಸರತಿ_get(ಸರತಿ, timeout);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_queue_get_mrsh.c>

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_vrfy_k_ಸರತಿ_is_ಖಾಲಿಗಳು(ರಚನೆ k_ಸರತಿ *ಸರತಿ)
{
	K_OOPS(K_SYSCALL_OBJ(ಸರತಿ, K_OBJ_ಸರತಿ));
	ಹಿಂತಿರುಗಿ z_impl_k_ಸರತಿ_is_ಖಾಲಿಗಳು(ಸರತಿ);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_queue_is_empty_mrsh.c>

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *z_vrfy_k_ಸರತಿ_peek_ತಲೆ(ರಚನೆ k_ಸರತಿ *ಸರತಿ)
{
	K_OOPS(K_SYSCALL_OBJ(ಸರತಿ, K_OBJ_ಸರತಿ));
	ಹಿಂತಿರುಗಿ z_impl_k_ಸರತಿ_peek_ತಲೆ(ಸರತಿ);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_queue_peek_head_mrsh.c>

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *z_vrfy_k_ಸರತಿ_peek_ಬಾಲ(ರಚನೆ k_ಸರತಿ *ಸರತಿ)
{
	K_OOPS(K_SYSCALL_OBJ(ಸರತಿ, K_OBJ_ಸರತಿ));
	ಹಿಂತಿರುಗಿ z_impl_k_ಸರತಿ_peek_ಬಾಲ(ಸರತಿ);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_queue_peek_tail_mrsh.c>

#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_FIFO
/* Referenced by k_fifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_ಆರಂಭ_EXTERN(k_fifo);
STRUCT_SECTION_ಕೊನೆ_EXTERN(k_fifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_fifo) =
	K_OBJ_TYPE_INITIALIZER(k_fifo, K_OBJ_TYPE_FIFO_ID, ಖಾಲಿ, 0, 0,
			       K_OBJ_RANGE_SECTION(k_fifo));
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_FIFO */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_LIFO
/* Referenced by k_lifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_ಆರಂಭ_EXTERN(k_lifo);
STRUCT_SECTION_ಕೊನೆ_EXTERN(k_lifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_lifo) =
	K_OBJ_TYPE_INITIALIZER(k_lifo, K_OBJ_TYPE_LIFO_ID, ಖಾಲಿ, 0, 0,
			       K_OBJ_RANGE_SECTION(k_lifo));
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_LIFO */
