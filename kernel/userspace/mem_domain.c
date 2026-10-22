/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <kernel_internal.h>
#અન્તર્ભાવ <zephyr/sys/__assert.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <zephyr/spinlock.h>
#અન્તર્ભાવ <zephyr/sys/check.h>
#અન્તર્ભાવ <zephyr/sys/libc-hooks.h>
#અન્તર્ભાવ <zephyr/logging/log.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

સંરચના k_spinlock z_mem_domain_lock;
સ્થિર uint8_t max_partitions;

સંરચના k_mem_domain k_mem_domain_default;

સ્થિર bool check_add_partition(સંરચના k_mem_domain *domain,
				સંરચના k_mem_partition *part)
{

	પૂર્ણાંક i;
	uintptr_t pstart, pend, dstart, dend;

	જો (part == ખાલી) {
		LOG_ERR("NULL k_mem_partition provided");
		પાછા false;
	}

#ifdef CONFIG_EXECUTE_XOR_WRITE
	/* Arches where execution cannot be disabled should always return
	 * false to this check
	 */
	જો (K_MEM_PARTITION_IS_EXECUTABLE(part->attr) &&
	    K_MEM_PARTITION_IS_WRITABLE(part->attr)) {
		LOG_ERR("partition is writable and executable <start %lx>",
			part->start);
		પાછા false;
	}
#endif /* CONFIG_EXECUTE_XOR_WRITE */

	જો (part->size == 0U) {
		LOG_ERR("zero sized partition at %p with base 0x%lx",
			part, part->start);
		પાછા false;
	}

	pstart = part->start;
	pend = part->start + part->size;

	જો (pend <= pstart) {
		LOG_ERR("invalid partition %p, wraparound detected. base 0x%lx size %zu",
			part, part->start, part->size);
		પાછા false;
	}

	/* Check that this partition doesn't overlap any existing ones already
	 * in the domain
	 */
	માટે (i = 0; i < domain->અંક_partitions; i++) {
		સંરચના k_mem_partition *dpart = &domain->partitions[i];

		જો (dpart->size == 0U) {
			/* Unused slot */
			ચાલુ;
		}

		dstart = dpart->start;
		dend = dstart + dpart->size;

		જો (pend > dstart && dend > pstart) {
			LOG_ERR("partition %p base %lx (size %zu) overlaps existing base %lx (size %zu)",
				part, part->start, part->size,
				dpart->start, dpart->size);
			પાછા false;
		}
	}

	પાછા true;
}

પૂર્ણાંક k_mem_domain_init(સંરચના k_mem_domain *domain, uint8_t અંક_parts,
		      સંરચના k_mem_partition *parts[])
{
	k_spinlock_ચાવી_t ચાવી;
	પૂર્ણાંક ret = 0;

	CHECKIF(domain == ખાલી) {
		ret = -EINVAL;
		જાઓ out;
	}

	CHECKIF(!(અંક_parts == 0U || parts != ખાલી)) {
		LOG_ERR("parts array is NULL and num_parts is nonzero");
		ret = -EINVAL;
		જાઓ out;
	}

	CHECKIF(!(અંક_parts <= max_partitions)) {
		LOG_ERR("num_parts of %d exceeds maximum allowable partitions (%d)",
			અંક_parts, max_partitions);
		ret = -EINVAL;
		જાઓ out;
	}

	ચાવી = k_spin_lock(&z_mem_domain_lock);

	domain->અંક_partitions = 0U;
	(શૂન્ય)memset(domain->partitions, 0, કદ(domain->partitions));

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_init(&domain->thread_mem_domain_list);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#ifdef CONFIG_ARCH_MEM_DOMAIN_DATA
	ret = arch_mem_domain_init(domain);

	જો (ret != 0) {
		LOG_ERR("architecture-specific initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		જાઓ unlock_out;
	}
#endif /* CONFIG_ARCH_MEM_DOMAIN_DATA */
	જો (અંક_parts != 0U) {
		uint32_t i;

		માટે (i = 0U; i < અંક_parts; i++) {
			CHECKIF(!check_add_partition(domain, parts[i])) {
				LOG_ERR("invalid partition index %d (%p)",
					i, parts[i]);
				ret = -EINVAL;
				જાઓ unlock_out;
			}

			domain->partitions[i] = *parts[i];
			domain->અંક_partitions++;
#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
			પૂર્ણાંક ret2 = arch_mem_domain_partition_add(domain, i);

			ARG_UNUSED(ret2);
			CHECKIF(ret2 != 0) {
				ret = ret2;
			}
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */
		}
	}

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, ચાવી);

out:
	પાછા ret;
}

પૂર્ણાંક k_mem_domain_deinit(સંરચના k_mem_domain *domain)
{
#જો defined(CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT)
	k_spinlock_ચાવી_t ચાવી;
	પૂર્ણાંક ret = 0;

	CHECKIF(domain == ખાલી) {
		ret = -EINVAL;
		જાઓ out;
	}

	જો (domain == &k_mem_domain_default) {
		/* Default memory domain must be there forever. */
		ret = -EINVAL;
		જાઓ out;
	}

	ચાવી = k_spin_lock(&z_mem_domain_lock);

	/* Must make sure there are no threads associated with this memory
	 * domain anymore. Or else these threads will run with an invalid
	 * memory domain.
	 */
	જો (!sys_dlist_is_empty(&domain->thread_mem_domain_list)) {
		ret = -EBUSY;
		જાઓ unlock_out;
	}

	ret = arch_mem_domain_deinit(domain);
	જો (ret != 0) {
		LOG_ERR("architecture-specific de-initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		જાઓ unlock_out;
	}

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, ચાવી);

out:
	પાછા ret;
#નહીંતર  /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
	ARG_UNUSED(domain);
	પાછા -ENOTSUP;
#endif /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
}

પૂર્ણાંક k_mem_domain_add_partition(સંરચના k_mem_domain *domain,
			       સંરચના k_mem_partition *part)
{
	પૂર્ણાંક p_idx;
	k_spinlock_ચાવી_t ચાવી;
	પૂર્ણાંક ret = 0;

	CHECKIF(domain == ખાલી) {
		ret = -EINVAL;
		જાઓ out;
	}

	CHECKIF(!check_add_partition(domain, part)) {
		LOG_ERR("invalid partition %p", part);
		ret = -EINVAL;
		જાઓ out;
	}

	ચાવી = k_spin_lock(&z_mem_domain_lock);

	માટે (p_idx = 0; p_idx < max_partitions; p_idx++) {
		/* A zero-sized partition denotes it's a free partition */
		જો (domain->partitions[p_idx].size == 0U) {
			તોડો;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no free partition slots available");
		ret = -ENOSPC;
		જાઓ unlock_out;
	}

	LOG_DBG("add partition base %lx size %zu to domain %p",
		part->start, part->size, domain);

	domain->partitions[p_idx].start = part->start;
	domain->partitions[p_idx].size = part->size;
	domain->partitions[p_idx].attr = part->attr;

	domain->અંક_partitions++;

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_partition_add(domain, p_idx);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, ચાવી);

out:
	પાછા ret;
}

પૂર્ણાંક k_mem_domain_remove_partition(સંરચના k_mem_domain *domain,
				  સંરચના k_mem_partition *part)
{
	પૂર્ણાંક p_idx;
	k_spinlock_ચાવી_t ચાવી;
	પૂર્ણાંક ret = 0;

	CHECKIF((domain == ખાલી) || (part == ખાલી)) {
		ret = -EINVAL;
		જાઓ out;
	}

	ચાવી = k_spin_lock(&z_mem_domain_lock);

	/* find a partition that matches the given start and size */
	માટે (p_idx = 0; p_idx < max_partitions; p_idx++) {
		જો ((domain->partitions[p_idx].start == part->start) &&
		    (domain->partitions[p_idx].size == part->size)) {
			તોડો;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no matching partition found");
		ret = -ENOENT;
		જાઓ unlock_out;
	}

	LOG_DBG("remove partition base %lx size %zu from domain %p",
		part->start, part->size, domain);

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_partition_remove(domain, p_idx);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	/* A zero-sized partition denotes it's a free partition */
	domain->partitions[p_idx].size = 0U;

	domain->અંક_partitions--;

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, ચાવી);

out:
	પાછા ret;
}

સ્થિર પૂર્ણાંક add_thread_locked(સંરચના k_mem_domain *domain,
			     k_tid_t thread)
{
	પૂર્ણાંક ret = 0;

	__ASSERT_NO_MSG(domain != ખાલી);
	__ASSERT_NO_MSG(thread != ખાલી);

	LOG_DBG("add thread %p to domain %p", thread, domain);

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_append(&domain->thread_mem_domain_list,
			 &thread->mem_domain_info.thread_mem_domain_ગાંઠ);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

	thread->mem_domain_info.mem_domain = domain;

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_add(thread);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	પાછા ret;
}

સ્થિર પૂર્ણાંક remove_thread_locked(સંરચના k_thread *thread)
{
	પૂર્ણાંક ret = 0;

	__ASSERT_NO_MSG(thread != ખાલી);
	LOG_DBG("remove thread %p from memory domain %p",
		thread, thread->mem_domain_info.mem_domain);

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_remove(&thread->mem_domain_info.thread_mem_domain_ગાંઠ);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_remove(thread);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	પાછા ret;
}

/* Called from thread object initialization */
શૂન્ય z_mem_domain_init_thread(સંરચના k_thread *thread)
{
	પૂર્ણાંક ret;
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&z_mem_domain_lock);

	/* New threads inherit memory domain configuration from parent */
	ret = add_thread_locked(_current->mem_domain_info.mem_domain, thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_UNUSED(ret);

	k_spin_unlock(&z_mem_domain_lock, ચાવી);
}

/* Called when thread aborts during teardown tasks. The scheduler's spinlock is held */
શૂન્ય z_mem_domain_exit_thread(સંરચના k_thread *thread)
{
	પૂર્ણાંક ret;

	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&z_mem_domain_lock);

	ret = remove_thread_locked(thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_UNUSED(ret);

	k_spin_unlock(&z_mem_domain_lock, ચાવી);
}

પૂર્ણાંક k_mem_domain_add_thread(સંરચના k_mem_domain *domain, k_tid_t thread)
{
	પૂર્ણાંક ret = 0;
	k_spinlock_ચાવી_t ચાવી;

	ચાવી = k_spin_lock(&z_mem_domain_lock);
	જો (thread->mem_domain_info.mem_domain != domain) {
		ret = remove_thread_locked(thread);

		જો (ret == 0) {
			ret = add_thread_locked(domain, thread);
		}
	}
	k_spin_unlock(&z_mem_domain_lock, ચાવી);

	પાછા ret;
}

સ્થિર શૂન્ય init_mem_domain_module(શૂન્ય)
{
	પૂર્ણાંક ret;

	ARG_UNUSED(ret);

	max_partitions = arch_mem_domain_max_partitions_get();
	/*
	 * max_partitions must be less than or equal to
	 * CONFIG_MAX_DOMAIN_PARTITIONS, or would encounter array index
	 * out of bounds error.
	 */
	__ASSERT(max_partitions <= CONFIG_MAX_DOMAIN_PARTITIONS, "");

	ret = k_mem_domain_init(&k_mem_domain_default, 0, ખાલી);
	__ASSERT(ret == 0, "failed to init default mem domain");

#ifdef Z_LIBC_PARTITION_EXISTS
	ret = k_mem_domain_add_partition(&k_mem_domain_default,
					 &z_libc_partition);
	__ASSERT(ret == 0, "failed to add default libc mem partition");
#endif /* Z_LIBC_PARTITION_EXISTS */
}

K_KERNEL_INIT_PRE(init_mem_domain_module);
