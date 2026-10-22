/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <kernel_internal.h>
#அந்தர்பாவ <zephyr/sys/__assert.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <zephyr/spinlock.h>
#அந்தர்பாவ <zephyr/sys/check.h>
#அந்தர்பாவ <zephyr/sys/libc-hooks.h>
#அந்தர்பாவ <zephyr/logging/log.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

கட்டமைப்பு k_spinlock z_mem_domain_lock;
நிலையான uint8_t max_partitions;

கட்டமைப்பு k_mem_domain k_mem_domain_default;

நிலையான bool check_add_partition(கட்டமைப்பு k_mem_domain *domain,
				கட்டமைப்பு k_mem_partition *part)
{

	முழு i;
	uintptr_t pstart, pend, dstart, dend;

	எனில் (part == வெற்று) {
		LOG_ERR("NULL k_mem_partition provided");
		திரும்பு false;
	}

#ifdef CONFIG_EXECUTE_XOR_WRITE
	/* Arches where execution cannot be disabled should always return
	 * false to this check
	 */
	எனில் (K_MEM_PARTITION_IS_EXECUTABLE(part->attr) &&
	    K_MEM_PARTITION_IS_WRITABLE(part->attr)) {
		LOG_ERR("partition is writable and executable <start %lx>",
			part->start);
		திரும்பு false;
	}
#endif /* CONFIG_EXECUTE_XOR_WRITE */

	எனில் (part->size == 0U) {
		LOG_ERR("zero sized partition at %p with base 0x%lx",
			part, part->start);
		திரும்பு false;
	}

	pstart = part->start;
	pend = part->start + part->size;

	எனில் (pend <= pstart) {
		LOG_ERR("invalid partition %p, wraparound detected. base 0x%lx size %zu",
			part, part->start, part->size);
		திரும்பு false;
	}

	/* Check that this partition doesn't overlap any existing ones already
	 * in the domain
	 */
	க்கு (i = 0; i < domain->இலக்கம்_partitions; i++) {
		கட்டமைப்பு k_mem_partition *dpart = &domain->partitions[i];

		எனில் (dpart->size == 0U) {
			/* Unused slot */
			தொடர்;
		}

		dstart = dpart->start;
		dend = dstart + dpart->size;

		எனில் (pend > dstart && dend > pstart) {
			LOG_ERR("partition %p base %lx (size %zu) overlaps existing base %lx (size %zu)",
				part, part->start, part->size,
				dpart->start, dpart->size);
			திரும்பு false;
		}
	}

	திரும்பு true;
}

முழு k_mem_domain_init(கட்டமைப்பு k_mem_domain *domain, uint8_t இலக்கம்_parts,
		      கட்டமைப்பு k_mem_partition *parts[])
{
	k_spinlock_சாவி_t சாவி;
	முழு ret = 0;

	CHECKIF(domain == வெற்று) {
		ret = -EINVAL;
		செல் out;
	}

	CHECKIF(!(இலக்கம்_parts == 0U || parts != வெற்று)) {
		LOG_ERR("parts array is NULL and num_parts is nonzero");
		ret = -EINVAL;
		செல் out;
	}

	CHECKIF(!(இலக்கம்_parts <= max_partitions)) {
		LOG_ERR("num_parts of %d exceeds maximum allowable partitions (%d)",
			இலக்கம்_parts, max_partitions);
		ret = -EINVAL;
		செல் out;
	}

	சாவி = k_spin_lock(&z_mem_domain_lock);

	domain->இலக்கம்_partitions = 0U;
	(வெறுமை)memset(domain->partitions, 0, அளவு(domain->partitions));

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_init(&domain->thread_mem_domain_list);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#ifdef CONFIG_ARCH_MEM_DOMAIN_DATA
	ret = arch_mem_domain_init(domain);

	எனில் (ret != 0) {
		LOG_ERR("architecture-specific initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		செல் unlock_out;
	}
#endif /* CONFIG_ARCH_MEM_DOMAIN_DATA */
	எனில் (இலக்கம்_parts != 0U) {
		uint32_t i;

		க்கு (i = 0U; i < இலக்கம்_parts; i++) {
			CHECKIF(!check_add_partition(domain, parts[i])) {
				LOG_ERR("invalid partition index %d (%p)",
					i, parts[i]);
				ret = -EINVAL;
				செல் unlock_out;
			}

			domain->partitions[i] = *parts[i];
			domain->இலக்கம்_partitions++;
#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
			முழு ret2 = arch_mem_domain_partition_add(domain, i);

			ARG_UNUSED(ret2);
			CHECKIF(ret2 != 0) {
				ret = ret2;
			}
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */
		}
	}

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, சாவி);

out:
	திரும்பு ret;
}

முழு k_mem_domain_deinit(கட்டமைப்பு k_mem_domain *domain)
{
#எனில் defined(CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT)
	k_spinlock_சாவி_t சாவி;
	முழு ret = 0;

	CHECKIF(domain == வெற்று) {
		ret = -EINVAL;
		செல் out;
	}

	எனில் (domain == &k_mem_domain_default) {
		/* Default memory domain must be there forever. */
		ret = -EINVAL;
		செல் out;
	}

	சாவி = k_spin_lock(&z_mem_domain_lock);

	/* Must make sure there are no threads associated with this memory
	 * domain anymore. Or else these threads will run with an invalid
	 * memory domain.
	 */
	எனில் (!sys_dlist_is_empty(&domain->thread_mem_domain_list)) {
		ret = -EBUSY;
		செல் unlock_out;
	}

	ret = arch_mem_domain_deinit(domain);
	எனில் (ret != 0) {
		LOG_ERR("architecture-specific de-initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		செல் unlock_out;
	}

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, சாவி);

out:
	திரும்பு ret;
#இல்லையெனில்  /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
	ARG_UNUSED(domain);
	திரும்பு -ENOTSUP;
#endif /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
}

முழு k_mem_domain_add_partition(கட்டமைப்பு k_mem_domain *domain,
			       கட்டமைப்பு k_mem_partition *part)
{
	முழு p_idx;
	k_spinlock_சாவி_t சாவி;
	முழு ret = 0;

	CHECKIF(domain == வெற்று) {
		ret = -EINVAL;
		செல் out;
	}

	CHECKIF(!check_add_partition(domain, part)) {
		LOG_ERR("invalid partition %p", part);
		ret = -EINVAL;
		செல் out;
	}

	சாவி = k_spin_lock(&z_mem_domain_lock);

	க்கு (p_idx = 0; p_idx < max_partitions; p_idx++) {
		/* A zero-sized partition denotes it's a free partition */
		எனில் (domain->partitions[p_idx].size == 0U) {
			நிறுத்து;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no free partition slots available");
		ret = -ENOSPC;
		செல் unlock_out;
	}

	LOG_DBG("add partition base %lx size %zu to domain %p",
		part->start, part->size, domain);

	domain->partitions[p_idx].start = part->start;
	domain->partitions[p_idx].size = part->size;
	domain->partitions[p_idx].attr = part->attr;

	domain->இலக்கம்_partitions++;

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_partition_add(domain, p_idx);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, சாவி);

out:
	திரும்பு ret;
}

முழு k_mem_domain_remove_partition(கட்டமைப்பு k_mem_domain *domain,
				  கட்டமைப்பு k_mem_partition *part)
{
	முழு p_idx;
	k_spinlock_சாவி_t சாவி;
	முழு ret = 0;

	CHECKIF((domain == வெற்று) || (part == வெற்று)) {
		ret = -EINVAL;
		செல் out;
	}

	சாவி = k_spin_lock(&z_mem_domain_lock);

	/* find a partition that matches the given start and size */
	க்கு (p_idx = 0; p_idx < max_partitions; p_idx++) {
		எனில் ((domain->partitions[p_idx].start == part->start) &&
		    (domain->partitions[p_idx].size == part->size)) {
			நிறுத்து;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no matching partition found");
		ret = -ENOENT;
		செல் unlock_out;
	}

	LOG_DBG("remove partition base %lx size %zu from domain %p",
		part->start, part->size, domain);

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_partition_remove(domain, p_idx);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	/* A zero-sized partition denotes it's a free partition */
	domain->partitions[p_idx].size = 0U;

	domain->இலக்கம்_partitions--;

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, சாவி);

out:
	திரும்பு ret;
}

நிலையான முழு add_thread_locked(கட்டமைப்பு k_mem_domain *domain,
			     k_tid_t thread)
{
	முழு ret = 0;

	__ASSERT_NO_MSG(domain != வெற்று);
	__ASSERT_NO_MSG(thread != வெற்று);

	LOG_DBG("add thread %p to domain %p", thread, domain);

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_append(&domain->thread_mem_domain_list,
			 &thread->mem_domain_info.thread_mem_domain_முனை);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

	thread->mem_domain_info.mem_domain = domain;

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_add(thread);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	திரும்பு ret;
}

நிலையான முழு remove_thread_locked(கட்டமைப்பு k_thread *thread)
{
	முழு ret = 0;

	__ASSERT_NO_MSG(thread != வெற்று);
	LOG_DBG("remove thread %p from memory domain %p",
		thread, thread->mem_domain_info.mem_domain);

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_remove(&thread->mem_domain_info.thread_mem_domain_முனை);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_remove(thread);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	திரும்பு ret;
}

/* Called from thread object initialization */
வெறுமை z_mem_domain_init_thread(கட்டமைப்பு k_thread *thread)
{
	முழு ret;
	k_spinlock_சாவி_t சாவி = k_spin_lock(&z_mem_domain_lock);

	/* New threads inherit memory domain configuration from parent */
	ret = add_thread_locked(_current->mem_domain_info.mem_domain, thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_UNUSED(ret);

	k_spin_unlock(&z_mem_domain_lock, சாவி);
}

/* Called when thread aborts during teardown tasks. The scheduler's spinlock is held */
வெறுமை z_mem_domain_exit_thread(கட்டமைப்பு k_thread *thread)
{
	முழு ret;

	k_spinlock_சாவி_t சாவி = k_spin_lock(&z_mem_domain_lock);

	ret = remove_thread_locked(thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_UNUSED(ret);

	k_spin_unlock(&z_mem_domain_lock, சாவி);
}

முழு k_mem_domain_add_thread(கட்டமைப்பு k_mem_domain *domain, k_tid_t thread)
{
	முழு ret = 0;
	k_spinlock_சாவி_t சாவி;

	சாவி = k_spin_lock(&z_mem_domain_lock);
	எனில் (thread->mem_domain_info.mem_domain != domain) {
		ret = remove_thread_locked(thread);

		எனில் (ret == 0) {
			ret = add_thread_locked(domain, thread);
		}
	}
	k_spin_unlock(&z_mem_domain_lock, சாவி);

	திரும்பு ret;
}

நிலையான வெறுமை init_mem_domain_module(வெறுமை)
{
	முழு ret;

	ARG_UNUSED(ret);

	max_partitions = arch_mem_domain_max_partitions_get();
	/*
	 * max_partitions must be less than or equal to
	 * CONFIG_MAX_DOMAIN_PARTITIONS, or would encounter array index
	 * out of bounds error.
	 */
	__ASSERT(max_partitions <= CONFIG_MAX_DOMAIN_PARTITIONS, "");

	ret = k_mem_domain_init(&k_mem_domain_default, 0, வெற்று);
	__ASSERT(ret == 0, "failed to init default mem domain");

#ifdef Z_LIBC_PARTITION_EXISTS
	ret = k_mem_domain_add_partition(&k_mem_domain_default,
					 &z_libc_partition);
	__ASSERT(ret == 0, "failed to add default libc mem partition");
#endif /* Z_LIBC_PARTITION_EXISTS */
}

K_KERNEL_INIT_PRE(init_mem_domain_module);
