/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kernel_internal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/__assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/spinlock.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/check.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/libc-hooks.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/logging/log.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

ਬਣਤਰ k_spinlock z_mem_domain_ਤਾਲਾ;
ਸਥਿਰ uint8_t max_partitions;

ਬਣਤਰ k_mem_domain k_mem_domain_default;

ਸਥਿਰ bool check_ਜੋੜੋ_ਹਿੱਸੇ_ਕਰੋ(ਬਣਤਰ k_mem_domain *domain,
				ਬਣਤਰ k_mem_ਹਿੱਸੇ_ਕਰੋ *part)
{

	ਪੂਰਨ i;
	uintptr_t pstart, pend, dstart, dend;

	ਜੇ (part == ਨਿਰਾ) {
		LOG_ERR("NULL k_mem_partition provided");
		ਵਾਪਸ false;
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_EXECUTE_XOR_WRITE
	/* Arches where execution cannot be disabled should always return
	 * false to this check
	 */
	ਜੇ (K_MEM_ਹਿੱਸੇ_ਕਰੋ_IS_EXECUTABLE(part->attr) &&
	    K_MEM_ਹਿੱਸੇ_ਕਰੋ_IS_WRITABLE(part->attr)) {
		LOG_ERR("partition is writable and executable <start %lx>",
			part->ਸ਼ੁਰੂ);
		ਵਾਪਸ false;
	}
#ਜੇ_ਅੰਤ /* CONFIG_EXECUTE_XOR_WRITE */

	ਜੇ (part->ਮਾਪ == 0U) {
		LOG_ERR("zero sized partition at %p with base 0x%lx",
			part, part->ਸ਼ੁਰੂ);
		ਵਾਪਸ false;
	}

	pstart = part->ਸ਼ੁਰੂ;
	pend = part->ਸ਼ੁਰੂ + part->ਮਾਪ;

	ਜੇ (pend <= pstart) {
		LOG_ERR("invalid partition %p, wraparound detected. base 0x%lx size %zu",
			part, part->ਸ਼ੁਰੂ, part->ਮਾਪ);
		ਵਾਪਸ false;
	}

	/* Check that this partition doesn't overlap any existing ones already
	 * in the domain
	 */
	ਲਈ (i = 0; i < domain->ਅੰਕ_partitions; i++) {
		ਬਣਤਰ k_mem_ਹਿੱਸੇ_ਕਰੋ *dpart = &domain->partitions[i];

		ਜੇ (dpart->ਮਾਪ == 0U) {
			/* Unused slot */
			ਜਾਰੀ;
		}

		dstart = dpart->ਸ਼ੁਰੂ;
		dend = dstart + dpart->ਮਾਪ;

		ਜੇ (pend > dstart && dend > pstart) {
			LOG_ERR("partition %p base %lx (size %zu) overlaps existing base %lx (size %zu)",
				part, part->ਸ਼ੁਰੂ, part->ਮਾਪ,
				dpart->ਸ਼ੁਰੂ, dpart->ਮਾਪ);
			ਵਾਪਸ false;
		}
	}

	ਵਾਪਸ true;
}

ਪੂਰਨ k_mem_domain_init(ਬਣਤਰ k_mem_domain *domain, uint8_t ਅੰਕ_parts,
		      ਬਣਤਰ k_mem_ਹਿੱਸੇ_ਕਰੋ *parts[])
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;
	ਪੂਰਨ ret = 0;

	CHECKIF(domain == ਨਿਰਾ) {
		ret = -EINVAL;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	CHECKIF(!(ਅੰਕ_parts == 0U || parts != ਨਿਰਾ)) {
		LOG_ERR("parts array is NULL and num_parts is nonzero");
		ret = -EINVAL;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	CHECKIF(!(ਅੰਕ_parts <= max_partitions)) {
		LOG_ERR("num_parts of %d exceeds maximum allowable partitions (%d)",
			ਅੰਕ_parts, max_partitions);
		ret = -EINVAL;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&z_mem_domain_ਤਾਲਾ);

	domain->ਅੰਕ_partitions = 0U;
	(ਖਾਲੀ)memset(domain->partitions, 0, ਆਕਾਰ(domain->partitions));

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_init(&domain->thread_mem_domain_list);
#ਜੇ_ਅੰਤ /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ARCH_MEM_DOMAIN_ਅੰਕੜੇ
	ret = arch_mem_domain_init(domain);

	ਜੇ (ret != 0) {
		LOG_ERR("architecture-specific initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		ਜਾਓ unlock_ਬਾਹਰਲਾ;
	}
#ਜੇ_ਅੰਤ /* CONFIG_ARCH_MEM_DOMAIN_DATA */
	ਜੇ (ਅੰਕ_parts != 0U) {
		uint32_t i;

		ਲਈ (i = 0U; i < ਅੰਕ_parts; i++) {
			CHECKIF(!check_ਜੋੜੋ_ਹਿੱਸੇ_ਕਰੋ(domain, parts[i])) {
				LOG_ERR("invalid partition index %d (%p)",
					i, parts[i]);
				ret = -EINVAL;
				ਜਾਓ unlock_ਬਾਹਰਲਾ;
			}

			domain->partitions[i] = *parts[i];
			domain->ਅੰਕ_partitions++;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
			ਪੂਰਨ ret2 = arch_mem_domain_ਹਿੱਸੇ_ਕਰੋ_ਜੋੜੋ(domain, i);

			ARG_ਅਣਵਰਤਿਆ(ret2);
			CHECKIF(ret2 != 0) {
				ret = ret2;
			}
#ਜੇ_ਅੰਤ /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */
		}
	}

unlock_ਬਾਹਰਲਾ:
	k_spin_unlock(&z_mem_domain_ਤਾਲਾ, ਕੁੰਜੀ);

ਬਾਹਰਲਾ:
	ਵਾਪਸ ret;
}

ਪੂਰਨ k_mem_domain_deinit(ਬਣਤਰ k_mem_domain *domain)
{
#ਜੇ defined(CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT)
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;
	ਪੂਰਨ ret = 0;

	CHECKIF(domain == ਨਿਰਾ) {
		ret = -EINVAL;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ਜੇ (domain == &k_mem_domain_default) {
		/* Default memory domain must be there forever. */
		ret = -EINVAL;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&z_mem_domain_ਤਾਲਾ);

	/* Must make sure there are no threads associated with this memory
	 * domain anymore. Or else these threads will run with an invalid
	 * memory domain.
	 */
	ਜੇ (!sys_dlist_is_ਖਾਲੀ_ਥਾਵਾਂ(&domain->thread_mem_domain_list)) {
		ret = -EBUSY;
		ਜਾਓ unlock_ਬਾਹਰਲਾ;
	}

	ret = arch_mem_domain_deinit(domain);
	ਜੇ (ret != 0) {
		LOG_ERR("architecture-specific de-initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		ਜਾਓ unlock_ਬਾਹਰਲਾ;
	}

unlock_ਬਾਹਰਲਾ:
	k_spin_unlock(&z_mem_domain_ਤਾਲਾ, ਕੁੰਜੀ);

ਬਾਹਰਲਾ:
	ਵਾਪਸ ret;
#ਨਹੀਂਤਾਂ  /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
	ARG_ਅਣਵਰਤਿਆ(domain);
	ਵਾਪਸ -ENOTSUP;
#ਜੇ_ਅੰਤ /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
}

ਪੂਰਨ k_mem_domain_ਜੋੜੋ_ਹਿੱਸੇ_ਕਰੋ(ਬਣਤਰ k_mem_domain *domain,
			       ਬਣਤਰ k_mem_ਹਿੱਸੇ_ਕਰੋ *part)
{
	ਪੂਰਨ p_idx;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;
	ਪੂਰਨ ret = 0;

	CHECKIF(domain == ਨਿਰਾ) {
		ret = -EINVAL;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	CHECKIF(!check_ਜੋੜੋ_ਹਿੱਸੇ_ਕਰੋ(domain, part)) {
		LOG_ERR("invalid partition %p", part);
		ret = -EINVAL;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&z_mem_domain_ਤਾਲਾ);

	ਲਈ (p_idx = 0; p_idx < max_partitions; p_idx++) {
		/* A zero-sized partition denotes it's a free partition */
		ਜੇ (domain->partitions[p_idx].ਮਾਪ == 0U) {
			ਰੋਕੋ;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no free partition slots available");
		ret = -ENOSPC;
		ਜਾਓ unlock_ਬਾਹਰਲਾ;
	}

	LOG_DBG("add partition base %lx size %zu to domain %p",
		part->ਸ਼ੁਰੂ, part->ਮਾਪ, domain);

	domain->partitions[p_idx].ਸ਼ੁਰੂ = part->ਸ਼ੁਰੂ;
	domain->partitions[p_idx].ਮਾਪ = part->ਮਾਪ;
	domain->partitions[p_idx].attr = part->attr;

	domain->ਅੰਕ_partitions++;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_ਹਿੱਸੇ_ਕਰੋ_ਜੋੜੋ(domain, p_idx);
#ਜੇ_ਅੰਤ /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

unlock_ਬਾਹਰਲਾ:
	k_spin_unlock(&z_mem_domain_ਤਾਲਾ, ਕੁੰਜੀ);

ਬਾਹਰਲਾ:
	ਵਾਪਸ ret;
}

ਪੂਰਨ k_mem_domain_remove_ਹਿੱਸੇ_ਕਰੋ(ਬਣਤਰ k_mem_domain *domain,
				  ਬਣਤਰ k_mem_ਹਿੱਸੇ_ਕਰੋ *part)
{
	ਪੂਰਨ p_idx;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;
	ਪੂਰਨ ret = 0;

	CHECKIF((domain == ਨਿਰਾ) || (part == ਨਿਰਾ)) {
		ret = -EINVAL;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&z_mem_domain_ਤਾਲਾ);

	/* find a partition that matches the given start and size */
	ਲਈ (p_idx = 0; p_idx < max_partitions; p_idx++) {
		ਜੇ ((domain->partitions[p_idx].ਸ਼ੁਰੂ == part->ਸ਼ੁਰੂ) &&
		    (domain->partitions[p_idx].ਮਾਪ == part->ਮਾਪ)) {
			ਰੋਕੋ;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no matching partition found");
		ret = -ENOENT;
		ਜਾਓ unlock_ਬਾਹਰਲਾ;
	}

	LOG_DBG("remove partition base %lx size %zu from domain %p",
		part->ਸ਼ੁਰੂ, part->ਮਾਪ, domain);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_ਹਿੱਸੇ_ਕਰੋ_remove(domain, p_idx);
#ਜੇ_ਅੰਤ /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	/* A zero-sized partition denotes it's a free partition */
	domain->partitions[p_idx].ਮਾਪ = 0U;

	domain->ਅੰਕ_partitions--;

unlock_ਬਾਹਰਲਾ:
	k_spin_unlock(&z_mem_domain_ਤਾਲਾ, ਕੁੰਜੀ);

ਬਾਹਰਲਾ:
	ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ ਜੋੜੋ_thread_locked(ਬਣਤਰ k_mem_domain *domain,
			     k_tid_t thread)
{
	ਪੂਰਨ ret = 0;

	__ASSERT_NO_MSG(domain != ਨਿਰਾ);
	__ASSERT_NO_MSG(thread != ਨਿਰਾ);

	LOG_DBG("add thread %p to domain %p", thread, domain);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_append(&domain->thread_mem_domain_list,
			 &thread->mem_domain_info.thread_mem_domain_ਗੰਢ);
#ਜੇ_ਅੰਤ /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

	thread->mem_domain_info.mem_domain = domain;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_ਜੋੜੋ(thread);
#ਜੇ_ਅੰਤ /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ remove_thread_locked(ਬਣਤਰ k_thread *thread)
{
	ਪੂਰਨ ret = 0;

	__ASSERT_NO_MSG(thread != ਨਿਰਾ);
	LOG_DBG("remove thread %p from memory domain %p",
		thread, thread->mem_domain_info.mem_domain);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_remove(&thread->mem_domain_info.thread_mem_domain_ਗੰਢ);
#ਜੇ_ਅੰਤ /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_remove(thread);
#ਜੇ_ਅੰਤ /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	ਵਾਪਸ ret;
}

/* Called from thread object initialization */
ਖਾਲੀ z_mem_domain_init_thread(ਬਣਤਰ k_thread *thread)
{
	ਪੂਰਨ ret;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&z_mem_domain_ਤਾਲਾ);

	/* New threads inherit memory domain configuration from parent */
	ret = ਜੋੜੋ_thread_locked(_current->mem_domain_info.mem_domain, thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_ਅਣਵਰਤਿਆ(ret);

	k_spin_unlock(&z_mem_domain_ਤਾਲਾ, ਕੁੰਜੀ);
}

/* Called when thread aborts during teardown tasks. The scheduler's spinlock is held */
ਖਾਲੀ z_mem_domain_exit_thread(ਬਣਤਰ k_thread *thread)
{
	ਪੂਰਨ ret;

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&z_mem_domain_ਤਾਲਾ);

	ret = remove_thread_locked(thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_ਅਣਵਰਤਿਆ(ret);

	k_spin_unlock(&z_mem_domain_ਤਾਲਾ, ਕੁੰਜੀ);
}

ਪੂਰਨ k_mem_domain_ਜੋੜੋ_thread(ਬਣਤਰ k_mem_domain *domain, k_tid_t thread)
{
	ਪੂਰਨ ret = 0;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&z_mem_domain_ਤਾਲਾ);
	ਜੇ (thread->mem_domain_info.mem_domain != domain) {
		ret = remove_thread_locked(thread);

		ਜੇ (ret == 0) {
			ret = ਜੋੜੋ_thread_locked(domain, thread);
		}
	}
	k_spin_unlock(&z_mem_domain_ਤਾਲਾ, ਕੁੰਜੀ);

	ਵਾਪਸ ret;
}

ਸਥਿਰ ਖਾਲੀ init_mem_domain_module(ਖਾਲੀ)
{
	ਪੂਰਨ ret;

	ARG_ਅਣਵਰਤਿਆ(ret);

	max_partitions = arch_mem_domain_max_partitions_get();
	/*
	 * max_partitions must be less than or equal to
	 * CONFIG_MAX_DOMAIN_PARTITIONS, or would encounter array index
	 * out of bounds error.
	 */
	__ASSERT(max_partitions <= CONFIG_MAX_DOMAIN_PARTITIONS, "");

	ret = k_mem_domain_init(&k_mem_domain_default, 0, ਨਿਰਾ);
	__ASSERT(ret == 0, "failed to init default mem domain");

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ Z_LIBC_ਹਿੱਸੇ_ਕਰੋ_EXISTS
	ret = k_mem_domain_ਜੋੜੋ_ਹਿੱਸੇ_ਕਰੋ(&k_mem_domain_default,
					 &z_libc_ਹਿੱਸੇ_ਕਰੋ);
	__ASSERT(ret == 0, "failed to add default libc mem partition");
#ਜੇ_ਅੰਤ /* Z_LIBC_PARTITION_EXISTS */
}

K_KERNEL_INIT_PRE(init_mem_domain_module);
