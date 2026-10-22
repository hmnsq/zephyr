/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <kernel_internal.h>
#അന്തര്ഭാവ <zephyr/sys/__assert.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <zephyr/spinlock.h>
#അന്തര്ഭാവ <zephyr/sys/check.h>
#അന്തര്ഭാവ <zephyr/sys/libc-hooks.h>
#അന്തര്ഭാവ <zephyr/logging/log.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

ഘടന k_spinlock z_mem_domain_lock;
സ്ഥിതം uint8_t max_partitions;

ഘടന k_mem_domain k_mem_domain_default;

സ്ഥിതം bool check_add_partition(ഘടന k_mem_domain *domain,
				ഘടന k_mem_partition *part)
{

	പൂർണ്ണസംഖ്യ i;
	uintptr_t pstart, pend, dstart, dend;

	എങ്കിൽ (part == ശൂന്യ) {
		LOG_ERR("NULL k_mem_partition provided");
		തിരികെ false;
	}

#ifdef CONFIG_EXECUTE_XOR_WRITE
	/* Arches where execution cannot be disabled should always return
	 * false to this check
	 */
	എങ്കിൽ (K_MEM_PARTITION_IS_EXECUTABLE(part->attr) &&
	    K_MEM_PARTITION_IS_WRITABLE(part->attr)) {
		LOG_ERR("partition is writable and executable <start %lx>",
			part->start);
		തിരികെ false;
	}
#endif /* CONFIG_EXECUTE_XOR_WRITE */

	എങ്കിൽ (part->size == 0U) {
		LOG_ERR("zero sized partition at %p with base 0x%lx",
			part, part->start);
		തിരികെ false;
	}

	pstart = part->start;
	pend = part->start + part->size;

	എങ്കിൽ (pend <= pstart) {
		LOG_ERR("invalid partition %p, wraparound detected. base 0x%lx size %zu",
			part, part->start, part->size);
		തിരികെ false;
	}

	/* Check that this partition doesn't overlap any existing ones already
	 * in the domain
	 */
	വേണ്ടി (i = 0; i < domain->അക്കം_partitions; i++) {
		ഘടന k_mem_partition *dpart = &domain->partitions[i];

		എങ്കിൽ (dpart->size == 0U) {
			/* Unused slot */
			തുടരുക;
		}

		dstart = dpart->start;
		dend = dstart + dpart->size;

		എങ്കിൽ (pend > dstart && dend > pstart) {
			LOG_ERR("partition %p base %lx (size %zu) overlaps existing base %lx (size %zu)",
				part, part->start, part->size,
				dpart->start, dpart->size);
			തിരികെ false;
		}
	}

	തിരികെ true;
}

പൂർണ്ണസംഖ്യ k_mem_domain_init(ഘടന k_mem_domain *domain, uint8_t അക്കം_parts,
		      ഘടന k_mem_partition *parts[])
{
	k_spinlock_താക്കോൽ_t താക്കോൽ;
	പൂർണ്ണസംഖ്യ ret = 0;

	CHECKIF(domain == ശൂന്യ) {
		ret = -EINVAL;
		പോകുക out;
	}

	CHECKIF(!(അക്കം_parts == 0U || parts != ശൂന്യ)) {
		LOG_ERR("parts array is NULL and num_parts is nonzero");
		ret = -EINVAL;
		പോകുക out;
	}

	CHECKIF(!(അക്കം_parts <= max_partitions)) {
		LOG_ERR("num_parts of %d exceeds maximum allowable partitions (%d)",
			അക്കം_parts, max_partitions);
		ret = -EINVAL;
		പോകുക out;
	}

	താക്കോൽ = k_spin_lock(&z_mem_domain_lock);

	domain->അക്കം_partitions = 0U;
	(ശൂന്യം)memset(domain->partitions, 0, വലുപ്പം(domain->partitions));

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_init(&domain->thread_mem_domain_list);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#ifdef CONFIG_ARCH_MEM_DOMAIN_DATA
	ret = arch_mem_domain_init(domain);

	എങ്കിൽ (ret != 0) {
		LOG_ERR("architecture-specific initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		പോകുക unlock_out;
	}
#endif /* CONFIG_ARCH_MEM_DOMAIN_DATA */
	എങ്കിൽ (അക്കം_parts != 0U) {
		uint32_t i;

		വേണ്ടി (i = 0U; i < അക്കം_parts; i++) {
			CHECKIF(!check_add_partition(domain, parts[i])) {
				LOG_ERR("invalid partition index %d (%p)",
					i, parts[i]);
				ret = -EINVAL;
				പോകുക unlock_out;
			}

			domain->partitions[i] = *parts[i];
			domain->അക്കം_partitions++;
#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
			പൂർണ്ണസംഖ്യ ret2 = arch_mem_domain_partition_add(domain, i);

			ARG_UNUSED(ret2);
			CHECKIF(ret2 != 0) {
				ret = ret2;
			}
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */
		}
	}

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, താക്കോൽ);

out:
	തിരികെ ret;
}

പൂർണ്ണസംഖ്യ k_mem_domain_deinit(ഘടന k_mem_domain *domain)
{
#എങ്കിൽ defined(CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT)
	k_spinlock_താക്കോൽ_t താക്കോൽ;
	പൂർണ്ണസംഖ്യ ret = 0;

	CHECKIF(domain == ശൂന്യ) {
		ret = -EINVAL;
		പോകുക out;
	}

	എങ്കിൽ (domain == &k_mem_domain_default) {
		/* Default memory domain must be there forever. */
		ret = -EINVAL;
		പോകുക out;
	}

	താക്കോൽ = k_spin_lock(&z_mem_domain_lock);

	/* Must make sure there are no threads associated with this memory
	 * domain anymore. Or else these threads will run with an invalid
	 * memory domain.
	 */
	എങ്കിൽ (!sys_dlist_is_empty(&domain->thread_mem_domain_list)) {
		ret = -EBUSY;
		പോകുക unlock_out;
	}

	ret = arch_mem_domain_deinit(domain);
	എങ്കിൽ (ret != 0) {
		LOG_ERR("architecture-specific de-initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		പോകുക unlock_out;
	}

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, താക്കോൽ);

out:
	തിരികെ ret;
#അല്ലെങ്കിൽ  /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
	ARG_UNUSED(domain);
	തിരികെ -ENOTSUP;
#endif /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
}

പൂർണ്ണസംഖ്യ k_mem_domain_add_partition(ഘടന k_mem_domain *domain,
			       ഘടന k_mem_partition *part)
{
	പൂർണ്ണസംഖ്യ p_idx;
	k_spinlock_താക്കോൽ_t താക്കോൽ;
	പൂർണ്ണസംഖ്യ ret = 0;

	CHECKIF(domain == ശൂന്യ) {
		ret = -EINVAL;
		പോകുക out;
	}

	CHECKIF(!check_add_partition(domain, part)) {
		LOG_ERR("invalid partition %p", part);
		ret = -EINVAL;
		പോകുക out;
	}

	താക്കോൽ = k_spin_lock(&z_mem_domain_lock);

	വേണ്ടി (p_idx = 0; p_idx < max_partitions; p_idx++) {
		/* A zero-sized partition denotes it's a free partition */
		എങ്കിൽ (domain->partitions[p_idx].size == 0U) {
			നിർത്തുക;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no free partition slots available");
		ret = -ENOSPC;
		പോകുക unlock_out;
	}

	LOG_DBG("add partition base %lx size %zu to domain %p",
		part->start, part->size, domain);

	domain->partitions[p_idx].start = part->start;
	domain->partitions[p_idx].size = part->size;
	domain->partitions[p_idx].attr = part->attr;

	domain->അക്കം_partitions++;

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_partition_add(domain, p_idx);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, താക്കോൽ);

out:
	തിരികെ ret;
}

പൂർണ്ണസംഖ്യ k_mem_domain_remove_partition(ഘടന k_mem_domain *domain,
				  ഘടന k_mem_partition *part)
{
	പൂർണ്ണസംഖ്യ p_idx;
	k_spinlock_താക്കോൽ_t താക്കോൽ;
	പൂർണ്ണസംഖ്യ ret = 0;

	CHECKIF((domain == ശൂന്യ) || (part == ശൂന്യ)) {
		ret = -EINVAL;
		പോകുക out;
	}

	താക്കോൽ = k_spin_lock(&z_mem_domain_lock);

	/* find a partition that matches the given start and size */
	വേണ്ടി (p_idx = 0; p_idx < max_partitions; p_idx++) {
		എങ്കിൽ ((domain->partitions[p_idx].start == part->start) &&
		    (domain->partitions[p_idx].size == part->size)) {
			നിർത്തുക;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no matching partition found");
		ret = -ENOENT;
		പോകുക unlock_out;
	}

	LOG_DBG("remove partition base %lx size %zu from domain %p",
		part->start, part->size, domain);

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_partition_remove(domain, p_idx);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	/* A zero-sized partition denotes it's a free partition */
	domain->partitions[p_idx].size = 0U;

	domain->അക്കം_partitions--;

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, താക്കോൽ);

out:
	തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ add_thread_locked(ഘടന k_mem_domain *domain,
			     k_tid_t thread)
{
	പൂർണ്ണസംഖ്യ ret = 0;

	__ASSERT_NO_MSG(domain != ശൂന്യ);
	__ASSERT_NO_MSG(thread != ശൂന്യ);

	LOG_DBG("add thread %p to domain %p", thread, domain);

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_append(&domain->thread_mem_domain_list,
			 &thread->mem_domain_info.thread_mem_domain_കണ്ണി);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

	thread->mem_domain_info.mem_domain = domain;

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_add(thread);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ remove_thread_locked(ഘടന k_thread *thread)
{
	പൂർണ്ണസംഖ്യ ret = 0;

	__ASSERT_NO_MSG(thread != ശൂന്യ);
	LOG_DBG("remove thread %p from memory domain %p",
		thread, thread->mem_domain_info.mem_domain);

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_remove(&thread->mem_domain_info.thread_mem_domain_കണ്ണി);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_remove(thread);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	തിരികെ ret;
}

/* Called from thread object initialization */
ശൂന്യം z_mem_domain_init_thread(ഘടന k_thread *thread)
{
	പൂർണ്ണസംഖ്യ ret;
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&z_mem_domain_lock);

	/* New threads inherit memory domain configuration from parent */
	ret = add_thread_locked(_current->mem_domain_info.mem_domain, thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_UNUSED(ret);

	k_spin_unlock(&z_mem_domain_lock, താക്കോൽ);
}

/* Called when thread aborts during teardown tasks. The scheduler's spinlock is held */
ശൂന്യം z_mem_domain_exit_thread(ഘടന k_thread *thread)
{
	പൂർണ്ണസംഖ്യ ret;

	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&z_mem_domain_lock);

	ret = remove_thread_locked(thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_UNUSED(ret);

	k_spin_unlock(&z_mem_domain_lock, താക്കോൽ);
}

പൂർണ്ണസംഖ്യ k_mem_domain_add_thread(ഘടന k_mem_domain *domain, k_tid_t thread)
{
	പൂർണ്ണസംഖ്യ ret = 0;
	k_spinlock_താക്കോൽ_t താക്കോൽ;

	താക്കോൽ = k_spin_lock(&z_mem_domain_lock);
	എങ്കിൽ (thread->mem_domain_info.mem_domain != domain) {
		ret = remove_thread_locked(thread);

		എങ്കിൽ (ret == 0) {
			ret = add_thread_locked(domain, thread);
		}
	}
	k_spin_unlock(&z_mem_domain_lock, താക്കോൽ);

	തിരികെ ret;
}

സ്ഥിതം ശൂന്യം init_mem_domain_module(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ ret;

	ARG_UNUSED(ret);

	max_partitions = arch_mem_domain_max_partitions_get();
	/*
	 * max_partitions must be less than or equal to
	 * CONFIG_MAX_DOMAIN_PARTITIONS, or would encounter array index
	 * out of bounds error.
	 */
	__ASSERT(max_partitions <= CONFIG_MAX_DOMAIN_PARTITIONS, "");

	ret = k_mem_domain_init(&k_mem_domain_default, 0, ശൂന്യ);
	__ASSERT(ret == 0, "failed to init default mem domain");

#ifdef Z_LIBC_PARTITION_EXISTS
	ret = k_mem_domain_add_partition(&k_mem_domain_default,
					 &z_libc_partition);
	__ASSERT(ret == 0, "failed to add default libc mem partition");
#endif /* Z_LIBC_PARTITION_EXISTS */
}

K_KERNEL_INIT_PRE(init_mem_domain_module);
