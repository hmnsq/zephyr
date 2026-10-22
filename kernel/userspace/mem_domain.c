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

ರಚನೆ k_spinlock z_mem_domain_ಬೀಗ;
ಸ್ಥಿರ uint8_t max_partitions;

ರಚನೆ k_mem_domain k_mem_domain_default;

ಸ್ಥಿರ bool check_ಕೂಡಿಸು_ವಿಭಜನೆ(ರಚನೆ k_mem_domain *domain,
				ರಚನೆ k_mem_ವಿಭಜನೆ *part)
{

	ಪೂರ್ಣಾಂಕ i;
	uintptr_t pstart, pend, dstart, dend;

	ಆದರೆ (part == ಖಾಲಿ) {
		LOG_ERR("NULL k_mem_partition provided");
		ಹಿಂತಿರುಗಿ false;
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_EXECUTE_XOR_WRITE
	/* Arches where execution cannot be disabled should always return
	 * false to this check
	 */
	ಆದರೆ (K_MEM_ವಿಭಜನೆ_IS_EXECUTABLE(part->attr) &&
	    K_MEM_ವಿಭಜನೆ_IS_WRITABLE(part->attr)) {
		LOG_ERR("partition is writable and executable <start %lx>",
			part->ಆರಂಭ);
		ಹಿಂತಿರುಗಿ false;
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_EXECUTE_XOR_WRITE */

	ಆದರೆ (part->ಗಾತ್ರ_2 == 0U) {
		LOG_ERR("zero sized partition at %p with base 0x%lx",
			part, part->ಆರಂಭ);
		ಹಿಂತಿರುಗಿ false;
	}

	pstart = part->ಆರಂಭ;
	pend = part->ಆರಂಭ + part->ಗಾತ್ರ_2;

	ಆದರೆ (pend <= pstart) {
		LOG_ERR("invalid partition %p, wraparound detected. base 0x%lx size %zu",
			part, part->ಆರಂಭ, part->ಗಾತ್ರ_2);
		ಹಿಂತಿರುಗಿ false;
	}

	/* Check that this partition doesn't overlap any existing ones already
	 * in the domain
	 */
	ಗಾಗಿ (i = 0; i < domain->ಅಂಕೆ_partitions; i++) {
		ರಚನೆ k_mem_ವಿಭಜನೆ *dpart = &domain->partitions[i];

		ಆದರೆ (dpart->ಗಾತ್ರ_2 == 0U) {
			/* Unused slot */
			ಮುಂದುವರಿ;
		}

		dstart = dpart->ಆರಂಭ;
		dend = dstart + dpart->ಗಾತ್ರ_2;

		ಆದರೆ (pend > dstart && dend > pstart) {
			LOG_ERR("partition %p base %lx (size %zu) overlaps existing base %lx (size %zu)",
				part, part->ಆರಂಭ, part->ಗಾತ್ರ_2,
				dpart->ಆರಂಭ, dpart->ಗಾತ್ರ_2);
			ಹಿಂತಿರುಗಿ false;
		}
	}

	ಹಿಂತಿರುಗಿ true;
}

ಪೂರ್ಣಾಂಕ k_mem_domain_init(ರಚನೆ k_mem_domain *domain, uint8_t ಅಂಕೆ_parts,
		      ರಚನೆ k_mem_ವಿಭಜನೆ *parts[])
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ಪೂರ್ಣಾಂಕ ret = 0;

	CHECKIF(domain == ಖಾಲಿ) {
		ret = -EINVAL;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	CHECKIF(!(ಅಂಕೆ_parts == 0U || parts != ಖಾಲಿ)) {
		LOG_ERR("parts array is NULL and num_parts is nonzero");
		ret = -EINVAL;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	CHECKIF(!(ಅಂಕೆ_parts <= max_partitions)) {
		LOG_ERR("num_parts of %d exceeds maximum allowable partitions (%d)",
			ಅಂಕೆ_parts, max_partitions);
		ret = -EINVAL;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ಕೀಲಿ = k_spin_ಬೀಗ(&z_mem_domain_ಬೀಗ);

	domain->ಅಂಕೆ_partitions = 0U;
	(ಶೂನ್ಯ)memset(domain->partitions, 0, ಗಾತ್ರ(domain->partitions));

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_init(&domain->thread_mem_domain_list);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ARCH_MEM_DOMAIN_ದತ್ತಾಂಶ
	ret = arch_mem_domain_init(domain);

	ಆದರೆ (ret != 0) {
		LOG_ERR("architecture-specific initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		ಹೋಗು unlock_ಹೊರಹೋಗು;
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_ARCH_MEM_DOMAIN_DATA */
	ಆದರೆ (ಅಂಕೆ_parts != 0U) {
		uint32_t i;

		ಗಾಗಿ (i = 0U; i < ಅಂಕೆ_parts; i++) {
			CHECKIF(!check_ಕೂಡಿಸು_ವಿಭಜನೆ(domain, parts[i])) {
				LOG_ERR("invalid partition index %d (%p)",
					i, parts[i]);
				ret = -EINVAL;
				ಹೋಗು unlock_ಹೊರಹೋಗು;
			}

			domain->partitions[i] = *parts[i];
			domain->ಅಂಕೆ_partitions++;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
			ಪೂರ್ಣಾಂಕ ret2 = arch_mem_domain_ವಿಭಜನೆ_ಕೂಡಿಸು(domain, i);

			ARG_ಬಳಸದ(ret2);
			CHECKIF(ret2 != 0) {
				ret = ret2;
			}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */
		}
	}

unlock_ಹೊರಹೋಗು:
	k_spin_unlock(&z_mem_domain_ಬೀಗ, ಕೀಲಿ);

ಹೊರಹೋಗು:
	ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ k_mem_domain_deinit(ರಚನೆ k_mem_domain *domain)
{
#ಆದರೆ defined(CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT)
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ಪೂರ್ಣಾಂಕ ret = 0;

	CHECKIF(domain == ಖಾಲಿ) {
		ret = -EINVAL;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ಆದರೆ (domain == &k_mem_domain_default) {
		/* Default memory domain must be there forever. */
		ret = -EINVAL;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ಕೀಲಿ = k_spin_ಬೀಗ(&z_mem_domain_ಬೀಗ);

	/* Must make sure there are no threads associated with this memory
	 * domain anymore. Or else these threads will run with an invalid
	 * memory domain.
	 */
	ಆದರೆ (!sys_dlist_is_ಖಾಲಿಗಳು(&domain->thread_mem_domain_list)) {
		ret = -EBUSY;
		ಹೋಗು unlock_ಹೊರಹೋಗು;
	}

	ret = arch_mem_domain_deinit(domain);
	ಆದರೆ (ret != 0) {
		LOG_ERR("architecture-specific de-initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		ಹೋಗು unlock_ಹೊರಹೋಗು;
	}

unlock_ಹೊರಹೋಗು:
	k_spin_unlock(&z_mem_domain_ಬೀಗ, ಕೀಲಿ);

ಹೊರಹೋಗು:
	ಹಿಂತಿರುಗಿ ret;
#ಇಲ್ಲದಿದ್ದರೆ  /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
	ARG_ಬಳಸದ(domain);
	ಹಿಂತಿರುಗಿ -ENOTSUP;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
}

ಪೂರ್ಣಾಂಕ k_mem_domain_ಕೂಡಿಸು_ವಿಭಜನೆ(ರಚನೆ k_mem_domain *domain,
			       ರಚನೆ k_mem_ವಿಭಜನೆ *part)
{
	ಪೂರ್ಣಾಂಕ p_idx;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ಪೂರ್ಣಾಂಕ ret = 0;

	CHECKIF(domain == ಖಾಲಿ) {
		ret = -EINVAL;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	CHECKIF(!check_ಕೂಡಿಸು_ವಿಭಜನೆ(domain, part)) {
		LOG_ERR("invalid partition %p", part);
		ret = -EINVAL;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ಕೀಲಿ = k_spin_ಬೀಗ(&z_mem_domain_ಬೀಗ);

	ಗಾಗಿ (p_idx = 0; p_idx < max_partitions; p_idx++) {
		/* A zero-sized partition denotes it's a free partition */
		ಆದರೆ (domain->partitions[p_idx].ಗಾತ್ರ_2 == 0U) {
			ನಿಲ್ಲಿಸು;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no free partition slots available");
		ret = -ENOSPC;
		ಹೋಗು unlock_ಹೊರಹೋಗು;
	}

	LOG_DBG("add partition base %lx size %zu to domain %p",
		part->ಆರಂಭ, part->ಗಾತ್ರ_2, domain);

	domain->partitions[p_idx].ಆರಂಭ = part->ಆರಂಭ;
	domain->partitions[p_idx].ಗಾತ್ರ_2 = part->ಗಾತ್ರ_2;
	domain->partitions[p_idx].attr = part->attr;

	domain->ಅಂಕೆ_partitions++;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_ವಿಭಜನೆ_ಕೂಡಿಸು(domain, p_idx);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

unlock_ಹೊರಹೋಗು:
	k_spin_unlock(&z_mem_domain_ಬೀಗ, ಕೀಲಿ);

ಹೊರಹೋಗು:
	ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ k_mem_domain_remove_ವಿಭಜನೆ(ರಚನೆ k_mem_domain *domain,
				  ರಚನೆ k_mem_ವಿಭಜನೆ *part)
{
	ಪೂರ್ಣಾಂಕ p_idx;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ಪೂರ್ಣಾಂಕ ret = 0;

	CHECKIF((domain == ಖಾಲಿ) || (part == ಖಾಲಿ)) {
		ret = -EINVAL;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ಕೀಲಿ = k_spin_ಬೀಗ(&z_mem_domain_ಬೀಗ);

	/* find a partition that matches the given start and size */
	ಗಾಗಿ (p_idx = 0; p_idx < max_partitions; p_idx++) {
		ಆದರೆ ((domain->partitions[p_idx].ಆರಂಭ == part->ಆರಂಭ) &&
		    (domain->partitions[p_idx].ಗಾತ್ರ_2 == part->ಗಾತ್ರ_2)) {
			ನಿಲ್ಲಿಸು;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no matching partition found");
		ret = -ENOENT;
		ಹೋಗು unlock_ಹೊರಹೋಗು;
	}

	LOG_DBG("remove partition base %lx size %zu from domain %p",
		part->ಆರಂಭ, part->ಗಾತ್ರ_2, domain);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_ವಿಭಜನೆ_remove(domain, p_idx);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	/* A zero-sized partition denotes it's a free partition */
	domain->partitions[p_idx].ಗಾತ್ರ_2 = 0U;

	domain->ಅಂಕೆ_partitions--;

unlock_ಹೊರಹೋಗು:
	k_spin_unlock(&z_mem_domain_ಬೀಗ, ಕೀಲಿ);

ಹೊರಹೋಗು:
	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಕೂಡಿಸು_thread_locked(ರಚನೆ k_mem_domain *domain,
			     k_tid_t thread)
{
	ಪೂರ್ಣಾಂಕ ret = 0;

	__ASSERT_NO_MSG(domain != ಖಾಲಿ);
	__ASSERT_NO_MSG(thread != ಖಾಲಿ);

	LOG_DBG("add thread %p to domain %p", thread, domain);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_append(&domain->thread_mem_domain_list,
			 &thread->mem_domain_info.thread_mem_domain_ಗಂಟು);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

	thread->mem_domain_info.mem_domain = domain;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_ಕೂಡಿಸು(thread);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ remove_thread_locked(ರಚನೆ k_thread *thread)
{
	ಪೂರ್ಣಾಂಕ ret = 0;

	__ASSERT_NO_MSG(thread != ಖಾಲಿ);
	LOG_DBG("remove thread %p from memory domain %p",
		thread, thread->mem_domain_info.mem_domain);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_remove(&thread->mem_domain_info.thread_mem_domain_ಗಂಟು);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_remove(thread);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	ಹಿಂತಿರುಗಿ ret;
}

/* Called from thread object initialization */
ಶೂನ್ಯ z_mem_domain_init_thread(ರಚನೆ k_thread *thread)
{
	ಪೂರ್ಣಾಂಕ ret;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&z_mem_domain_ಬೀಗ);

	/* New threads inherit memory domain configuration from parent */
	ret = ಕೂಡಿಸು_thread_locked(_current->mem_domain_info.mem_domain, thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_ಬಳಸದ(ret);

	k_spin_unlock(&z_mem_domain_ಬೀಗ, ಕೀಲಿ);
}

/* Called when thread aborts during teardown tasks. The scheduler's spinlock is held */
ಶೂನ್ಯ z_mem_domain_exit_thread(ರಚನೆ k_thread *thread)
{
	ಪೂರ್ಣಾಂಕ ret;

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&z_mem_domain_ಬೀಗ);

	ret = remove_thread_locked(thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_ಬಳಸದ(ret);

	k_spin_unlock(&z_mem_domain_ಬೀಗ, ಕೀಲಿ);
}

ಪೂರ್ಣಾಂಕ k_mem_domain_ಕೂಡಿಸು_thread(ರಚನೆ k_mem_domain *domain, k_tid_t thread)
{
	ಪೂರ್ಣಾಂಕ ret = 0;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;

	ಕೀಲಿ = k_spin_ಬೀಗ(&z_mem_domain_ಬೀಗ);
	ಆದರೆ (thread->mem_domain_info.mem_domain != domain) {
		ret = remove_thread_locked(thread);

		ಆದರೆ (ret == 0) {
			ret = ಕೂಡಿಸು_thread_locked(domain, thread);
		}
	}
	k_spin_unlock(&z_mem_domain_ಬೀಗ, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಶೂನ್ಯ init_mem_domain_module(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ ret;

	ARG_ಬಳಸದ(ret);

	max_partitions = arch_mem_domain_max_partitions_get();
	/*
	 * max_partitions must be less than or equal to
	 * CONFIG_MAX_DOMAIN_PARTITIONS, or would encounter array index
	 * out of bounds error.
	 */
	__ASSERT(max_partitions <= CONFIG_MAX_DOMAIN_PARTITIONS, "");

	ret = k_mem_domain_init(&k_mem_domain_default, 0, ಖಾಲಿ);
	__ASSERT(ret == 0, "failed to init default mem domain");

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ Z_LIBC_ವಿಭಜನೆ_EXISTS
	ret = k_mem_domain_ಕೂಡಿಸು_ವಿಭಜನೆ(&k_mem_domain_default,
					 &z_libc_ವಿಭಜನೆ);
	__ASSERT(ret == 0, "failed to add default libc mem partition");
#ಆದರೆ_ಅಂತ್ಯ /* Z_LIBC_PARTITION_EXISTS */
}

K_KERNEL_INIT_PRE(init_mem_domain_module);
