/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <kernel_internal.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/__assert.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <zephyr/spinlock.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/check.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/libc-hooks.h>
#ಅನ್ತರ್ಭಾವ <zephyr/logging/log.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

ಸಂರಚನಾ k_spinlock z_mem_domain_lock;
ಸ್ಥಿರ uint8_t max_partitions;

ಸಂರಚನಾ k_mem_domain k_mem_domain_default;

ಸ್ಥಿರ bool check_add_partition(ಸಂರಚನಾ k_mem_domain *domain,
				ಸಂರಚನಾ k_mem_partition *part)
{

	ಪೂರ್ಣಾಙ್ಕ i;
	uintptr_t pstart, pend, dstart, dend;

	ಯದಿ (part == ರಿಕ್ತ) {
		LOG_ERR("NULL k_mem_partition provided");
		ಪ್ರತ್ಯಾಗಮ false;
	}

#ifdef CONFIG_EXECUTE_XOR_WRITE
	/* Arches where execution cannot be disabled should always return
	 * false to this check
	 */
	ಯದಿ (K_MEM_PARTITION_IS_EXECUTABLE(part->attr) &&
	    K_MEM_PARTITION_IS_WRITABLE(part->attr)) {
		LOG_ERR("partition is writable and executable <start %lx>",
			part->start);
		ಪ್ರತ್ಯಾಗಮ false;
	}
#endif /* CONFIG_EXECUTE_XOR_WRITE */

	ಯದಿ (part->size == 0U) {
		LOG_ERR("zero sized partition at %p with base 0x%lx",
			part, part->start);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	pstart = part->start;
	pend = part->start + part->size;

	ಯದಿ (pend <= pstart) {
		LOG_ERR("invalid partition %p, wraparound detected. base 0x%lx size %zu",
			part, part->start, part->size);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	/* Check that this partition doesn't overlap any existing ones already
	 * in the domain
	 */
	ಪ್ರತಿ (i = 0; i < domain->ಅಂಕೆ_partitions; i++) {
		ಸಂರಚನಾ k_mem_partition *dpart = &domain->partitions[i];

		ಯದಿ (dpart->size == 0U) {
			/* Unused slot */
			ಅನುವರ್ತ;
		}

		dstart = dpart->start;
		dend = dstart + dpart->size;

		ಯದಿ (pend > dstart && dend > pstart) {
			LOG_ERR("partition %p base %lx (size %zu) overlaps existing base %lx (size %zu)",
				part, part->start, part->size,
				dpart->start, dpart->size);
			ಪ್ರತ್ಯಾಗಮ false;
		}
	}

	ಪ್ರತ್ಯಾಗಮ true;
}

ಪೂರ್ಣಾಙ್ಕ k_mem_domain_init(ಸಂರಚನಾ k_mem_domain *domain, uint8_t ಅಂಕೆ_parts,
		      ಸಂರಚನಾ k_mem_partition *parts[])
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ಪೂರ್ಣಾಙ್ಕ ret = 0;

	CHECKIF(domain == ರಿಕ್ತ) {
		ret = -EINVAL;
		ಗಚ್ಛ out;
	}

	CHECKIF(!(ಅಂಕೆ_parts == 0U || parts != ರಿಕ್ತ)) {
		LOG_ERR("parts array is NULL and num_parts is nonzero");
		ret = -EINVAL;
		ಗಚ್ಛ out;
	}

	CHECKIF(!(ಅಂಕೆ_parts <= max_partitions)) {
		LOG_ERR("num_parts of %d exceeds maximum allowable partitions (%d)",
			ಅಂಕೆ_parts, max_partitions);
		ret = -EINVAL;
		ಗಚ್ಛ out;
	}

	ಕೀಲಿ = k_spin_lock(&z_mem_domain_lock);

	domain->ಅಂಕೆ_partitions = 0U;
	(ಶೂನ್ಯ)memset(domain->partitions, 0, ಪರಿಮಾಣ(domain->partitions));

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_init(&domain->thread_mem_domain_list);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#ifdef CONFIG_ARCH_MEM_DOMAIN_DATA
	ret = arch_mem_domain_init(domain);

	ಯದಿ (ret != 0) {
		LOG_ERR("architecture-specific initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		ಗಚ್ಛ unlock_out;
	}
#endif /* CONFIG_ARCH_MEM_DOMAIN_DATA */
	ಯದಿ (ಅಂಕೆ_parts != 0U) {
		uint32_t i;

		ಪ್ರತಿ (i = 0U; i < ಅಂಕೆ_parts; i++) {
			CHECKIF(!check_add_partition(domain, parts[i])) {
				LOG_ERR("invalid partition index %d (%p)",
					i, parts[i]);
				ret = -EINVAL;
				ಗಚ್ಛ unlock_out;
			}

			domain->partitions[i] = *parts[i];
			domain->ಅಂಕೆ_partitions++;
#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
			ಪೂರ್ಣಾಙ್ಕ ret2 = arch_mem_domain_partition_add(domain, i);

			ARG_UNUSED(ret2);
			CHECKIF(ret2 != 0) {
				ret = ret2;
			}
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */
		}
	}

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, ಕೀಲಿ);

out:
	ಪ್ರತ್ಯಾಗಮ ret;
}

ಪೂರ್ಣಾಙ್ಕ k_mem_domain_deinit(ಸಂರಚನಾ k_mem_domain *domain)
{
#ಯದಿ defined(CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT)
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ಪೂರ್ಣಾಙ್ಕ ret = 0;

	CHECKIF(domain == ರಿಕ್ತ) {
		ret = -EINVAL;
		ಗಚ್ಛ out;
	}

	ಯದಿ (domain == &k_mem_domain_default) {
		/* Default memory domain must be there forever. */
		ret = -EINVAL;
		ಗಚ್ಛ out;
	}

	ಕೀಲಿ = k_spin_lock(&z_mem_domain_lock);

	/* Must make sure there are no threads associated with this memory
	 * domain anymore. Or else these threads will run with an invalid
	 * memory domain.
	 */
	ಯದಿ (!sys_dlist_is_empty(&domain->thread_mem_domain_list)) {
		ret = -EBUSY;
		ಗಚ್ಛ unlock_out;
	}

	ret = arch_mem_domain_deinit(domain);
	ಯದಿ (ret != 0) {
		LOG_ERR("architecture-specific de-initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		ಗಚ್ಛ unlock_out;
	}

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, ಕೀಲಿ);

out:
	ಪ್ರತ್ಯಾಗಮ ret;
#ಅನ್ಯಥಾ  /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
	ARG_UNUSED(domain);
	ಪ್ರತ್ಯಾಗಮ -ENOTSUP;
#endif /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
}

ಪೂರ್ಣಾಙ್ಕ k_mem_domain_add_partition(ಸಂರಚನಾ k_mem_domain *domain,
			       ಸಂರಚನಾ k_mem_partition *part)
{
	ಪೂರ್ಣಾಙ್ಕ p_idx;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ಪೂರ್ಣಾಙ್ಕ ret = 0;

	CHECKIF(domain == ರಿಕ್ತ) {
		ret = -EINVAL;
		ಗಚ್ಛ out;
	}

	CHECKIF(!check_add_partition(domain, part)) {
		LOG_ERR("invalid partition %p", part);
		ret = -EINVAL;
		ಗಚ್ಛ out;
	}

	ಕೀಲಿ = k_spin_lock(&z_mem_domain_lock);

	ಪ್ರತಿ (p_idx = 0; p_idx < max_partitions; p_idx++) {
		/* A zero-sized partition denotes it's a free partition */
		ಯದಿ (domain->partitions[p_idx].size == 0U) {
			ಭಙ್ಗ;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no free partition slots available");
		ret = -ENOSPC;
		ಗಚ್ಛ unlock_out;
	}

	LOG_DBG("add partition base %lx size %zu to domain %p",
		part->start, part->size, domain);

	domain->partitions[p_idx].start = part->start;
	domain->partitions[p_idx].size = part->size;
	domain->partitions[p_idx].attr = part->attr;

	domain->ಅಂಕೆ_partitions++;

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_partition_add(domain, p_idx);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, ಕೀಲಿ);

out:
	ಪ್ರತ್ಯಾಗಮ ret;
}

ಪೂರ್ಣಾಙ್ಕ k_mem_domain_remove_partition(ಸಂರಚನಾ k_mem_domain *domain,
				  ಸಂರಚನಾ k_mem_partition *part)
{
	ಪೂರ್ಣಾಙ್ಕ p_idx;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ಪೂರ್ಣಾಙ್ಕ ret = 0;

	CHECKIF((domain == ರಿಕ್ತ) || (part == ರಿಕ್ತ)) {
		ret = -EINVAL;
		ಗಚ್ಛ out;
	}

	ಕೀಲಿ = k_spin_lock(&z_mem_domain_lock);

	/* find a partition that matches the given start and size */
	ಪ್ರತಿ (p_idx = 0; p_idx < max_partitions; p_idx++) {
		ಯದಿ ((domain->partitions[p_idx].start == part->start) &&
		    (domain->partitions[p_idx].size == part->size)) {
			ಭಙ್ಗ;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no matching partition found");
		ret = -ENOENT;
		ಗಚ್ಛ unlock_out;
	}

	LOG_DBG("remove partition base %lx size %zu from domain %p",
		part->start, part->size, domain);

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_partition_remove(domain, p_idx);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	/* A zero-sized partition denotes it's a free partition */
	domain->partitions[p_idx].size = 0U;

	domain->ಅಂಕೆ_partitions--;

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, ಕೀಲಿ);

out:
	ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ add_thread_locked(ಸಂರಚನಾ k_mem_domain *domain,
			     k_tid_t thread)
{
	ಪೂರ್ಣಾಙ್ಕ ret = 0;

	__ASSERT_NO_MSG(domain != ರಿಕ್ತ);
	__ASSERT_NO_MSG(thread != ರಿಕ್ತ);

	LOG_DBG("add thread %p to domain %p", thread, domain);

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_append(&domain->thread_mem_domain_list,
			 &thread->mem_domain_info.thread_mem_domain_ಗಂಟು);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

	thread->mem_domain_info.mem_domain = domain;

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_add(thread);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ remove_thread_locked(ಸಂರಚನಾ k_thread *thread)
{
	ಪೂರ್ಣಾಙ್ಕ ret = 0;

	__ASSERT_NO_MSG(thread != ರಿಕ್ತ);
	LOG_DBG("remove thread %p from memory domain %p",
		thread, thread->mem_domain_info.mem_domain);

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_remove(&thread->mem_domain_info.thread_mem_domain_ಗಂಟು);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_remove(thread);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	ಪ್ರತ್ಯಾಗಮ ret;
}

/* Called from thread object initialization */
ಶೂನ್ಯ z_mem_domain_init_thread(ಸಂರಚನಾ k_thread *thread)
{
	ಪೂರ್ಣಾಙ್ಕ ret;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&z_mem_domain_lock);

	/* New threads inherit memory domain configuration from parent */
	ret = add_thread_locked(_current->mem_domain_info.mem_domain, thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_UNUSED(ret);

	k_spin_unlock(&z_mem_domain_lock, ಕೀಲಿ);
}

/* Called when thread aborts during teardown tasks. The scheduler's spinlock is held */
ಶೂನ್ಯ z_mem_domain_exit_thread(ಸಂರಚನಾ k_thread *thread)
{
	ಪೂರ್ಣಾಙ್ಕ ret;

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&z_mem_domain_lock);

	ret = remove_thread_locked(thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_UNUSED(ret);

	k_spin_unlock(&z_mem_domain_lock, ಕೀಲಿ);
}

ಪೂರ್ಣಾಙ್ಕ k_mem_domain_add_thread(ಸಂರಚನಾ k_mem_domain *domain, k_tid_t thread)
{
	ಪೂರ್ಣಾಙ್ಕ ret = 0;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;

	ಕೀಲಿ = k_spin_lock(&z_mem_domain_lock);
	ಯದಿ (thread->mem_domain_info.mem_domain != domain) {
		ret = remove_thread_locked(thread);

		ಯದಿ (ret == 0) {
			ret = add_thread_locked(domain, thread);
		}
	}
	k_spin_unlock(&z_mem_domain_lock, ಕೀಲಿ);

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಶೂನ್ಯ init_mem_domain_module(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ ret;

	ARG_UNUSED(ret);

	max_partitions = arch_mem_domain_max_partitions_get();
	/*
	 * max_partitions must be less than or equal to
	 * CONFIG_MAX_DOMAIN_PARTITIONS, or would encounter array index
	 * out of bounds error.
	 */
	__ASSERT(max_partitions <= CONFIG_MAX_DOMAIN_PARTITIONS, "");

	ret = k_mem_domain_init(&k_mem_domain_default, 0, ರಿಕ್ತ);
	__ASSERT(ret == 0, "failed to init default mem domain");

#ifdef Z_LIBC_PARTITION_EXISTS
	ret = k_mem_domain_add_partition(&k_mem_domain_default,
					 &z_libc_partition);
	__ASSERT(ret == 0, "failed to add default libc mem partition");
#endif /* Z_LIBC_PARTITION_EXISTS */
}

K_KERNEL_INIT_PRE(init_mem_domain_module);
