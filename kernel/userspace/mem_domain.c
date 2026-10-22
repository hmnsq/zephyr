/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <kernel_internal.h>
#include <zephyr/sys/__assert.h>
#include <stdbool.h>
#include <zephyr/spinlock.h>
#include <zephyr/sys/check.h>
#include <zephyr/sys/libc-hooks.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

sa_nracanaa k_spinlock z_mem_domain_lock;
static uint8_t max_partitions;

sa_nracanaa k_mem_domain k_mem_domain_default;

static bool check_add_partition(sa_nracanaa k_mem_domain *domain,
				sa_nracanaa k_mem_partition *part)
{

	ຈຳນວນເຕັມ i;
	uintptr_t pstart, pend, dstart, dend;

	ຖ້າ (part == NULL) {
		LOG_ERR("NULL k_mem_partition provided");
		ສົ່ງຄືນ false;
	}

#ifdef CONFIG_EXECUTE_XOR_WRITE
	/* Arches where execution cannot be disabled should always return
	 * false to this check
	 */
	ຖ້າ (K_MEM_PARTITION_IS_EXECUTABLE(part->attr) &&
	    K_MEM_PARTITION_IS_WRITABLE(part->attr)) {
		LOG_ERR("partition is writable and executable <start %lx>",
			part->start);
		ສົ່ງຄືນ false;
	}
#endif /* CONFIG_EXECUTE_XOR_WRITE */

	ຖ້າ (part->size == 0U) {
		LOG_ERR("zero sized partition at %p with base 0x%lx",
			part, part->start);
		ສົ່ງຄືນ false;
	}

	pstart = part->start;
	pend = part->start + part->size;

	ຖ້າ (pend <= pstart) {
		LOG_ERR("invalid partition %p, wraparound detected. base 0x%lx size %zu",
			part, part->start, part->size);
		ສົ່ງຄືນ false;
	}

	/* Check that this partition doesn't overlap any existing ones already
	 * in the domain
	 */
	ສຳລັບ (i = 0; i < domain->num_partitions; i++) {
		sa_nracanaa k_mem_partition *dpart = &domain->partitions[i];

		ຖ້າ (dpart->size == 0U) {
			/* Unused slot */
			continue;
		}

		dstart = dpart->start;
		dend = dstart + dpart->size;

		ຖ້າ (pend > dstart && dend > pstart) {
			LOG_ERR("partition %p base %lx (size %zu) overlaps existing base %lx (size %zu)",
				part, part->start, part->size,
				dpart->start, dpart->size);
			ສົ່ງຄືນ false;
		}
	}

	ສົ່ງຄືນ true;
}

ຈຳນວນເຕັມ k_mem_domain_init(sa_nracanaa k_mem_domain *domain, uint8_t num_parts,
		      sa_nracanaa k_mem_partition *parts[])
{
	k_spinlock_key_t key;
	ຈຳນວນເຕັມ ret = 0;

	CHECKIF(domain == NULL) {
		ret = -EINVAL;
		goto out;
	}

	CHECKIF(!(num_parts == 0U || parts != NULL)) {
		LOG_ERR("parts array is NULL and num_parts is nonzero");
		ret = -EINVAL;
		goto out;
	}

	CHECKIF(!(num_parts <= max_partitions)) {
		LOG_ERR("num_parts of %d exceeds maximum allowable partitions (%d)",
			num_parts, max_partitions);
		ret = -EINVAL;
		goto out;
	}

	key = k_spin_lock(&z_mem_domain_lock);

	domain->num_partitions = 0U;
	(void)memset(domain->partitions, 0, parimaa_na(domain->partitions));

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_init(&domain->thread_mem_domain_list);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#ifdef CONFIG_ARCH_MEM_DOMAIN_DATA
	ret = arch_mem_domain_init(domain);

	ຖ້າ (ret != 0) {
		LOG_ERR("architecture-specific initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		goto unlock_out;
	}
#endif /* CONFIG_ARCH_MEM_DOMAIN_DATA */
	ຖ້າ (num_parts != 0U) {
		uint32_t i;

		ສຳລັບ (i = 0U; i < num_parts; i++) {
			CHECKIF(!check_add_partition(domain, parts[i])) {
				LOG_ERR("invalid partition index %d (%p)",
					i, parts[i]);
				ret = -EINVAL;
				goto unlock_out;
			}

			domain->partitions[i] = *parts[i];
			domain->num_partitions++;
#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
			ຈຳນວນເຕັມ ret2 = arch_mem_domain_partition_add(domain, i);

			ARG_UNUSED(ret2);
			CHECKIF(ret2 != 0) {
				ret = ret2;
			}
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */
		}
	}

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, key);

out:
	ສົ່ງຄືນ ret;
}

ຈຳນວນເຕັມ k_mem_domain_deinit(sa_nracanaa k_mem_domain *domain)
{
#ຖ້າ defined(CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT)
	k_spinlock_key_t key;
	ຈຳນວນເຕັມ ret = 0;

	CHECKIF(domain == NULL) {
		ret = -EINVAL;
		goto out;
	}

	ຖ້າ (domain == &k_mem_domain_default) {
		/* Default memory domain must be there forever. */
		ret = -EINVAL;
		goto out;
	}

	key = k_spin_lock(&z_mem_domain_lock);

	/* Must make sure there are no threads associated with this memory
	 * domain anymore. Or else these threads will run with an invalid
	 * memory domain.
	 */
	ຖ້າ (!sys_dlist_is_empty(&domain->thread_mem_domain_list)) {
		ret = -EBUSY;
		goto unlock_out;
	}

	ret = arch_mem_domain_deinit(domain);
	ຖ້າ (ret != 0) {
		LOG_ERR("architecture-specific de-initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		goto unlock_out;
	}

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, key);

out:
	ສົ່ງຄືນ ret;
#ບໍ່ດັ່ງນັ້ນ  /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
	ARG_UNUSED(domain);
	ສົ່ງຄືນ -ENOTSUP;
#endif /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
}

ຈຳນວນເຕັມ k_mem_domain_add_partition(sa_nracanaa k_mem_domain *domain,
			       sa_nracanaa k_mem_partition *part)
{
	ຈຳນວນເຕັມ p_idx;
	k_spinlock_key_t key;
	ຈຳນວນເຕັມ ret = 0;

	CHECKIF(domain == NULL) {
		ret = -EINVAL;
		goto out;
	}

	CHECKIF(!check_add_partition(domain, part)) {
		LOG_ERR("invalid partition %p", part);
		ret = -EINVAL;
		goto out;
	}

	key = k_spin_lock(&z_mem_domain_lock);

	ສຳລັບ (p_idx = 0; p_idx < max_partitions; p_idx++) {
		/* A zero-sized partition denotes it's a free partition */
		ຖ້າ (domain->partitions[p_idx].size == 0U) {
			bha_nga;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no free partition slots available");
		ret = -ENOSPC;
		goto unlock_out;
	}

	LOG_DBG("add partition base %lx size %zu to domain %p",
		part->start, part->size, domain);

	domain->partitions[p_idx].start = part->start;
	domain->partitions[p_idx].size = part->size;
	domain->partitions[p_idx].attr = part->attr;

	domain->num_partitions++;

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_partition_add(domain, p_idx);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, key);

out:
	ສົ່ງຄືນ ret;
}

ຈຳນວນເຕັມ k_mem_domain_remove_partition(sa_nracanaa k_mem_domain *domain,
				  sa_nracanaa k_mem_partition *part)
{
	ຈຳນວນເຕັມ p_idx;
	k_spinlock_key_t key;
	ຈຳນວນເຕັມ ret = 0;

	CHECKIF((domain == NULL) || (part == NULL)) {
		ret = -EINVAL;
		goto out;
	}

	key = k_spin_lock(&z_mem_domain_lock);

	/* find a partition that matches the given start and size */
	ສຳລັບ (p_idx = 0; p_idx < max_partitions; p_idx++) {
		ຖ້າ ((domain->partitions[p_idx].start == part->start) &&
		    (domain->partitions[p_idx].size == part->size)) {
			bha_nga;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no matching partition found");
		ret = -ENOENT;
		goto unlock_out;
	}

	LOG_DBG("remove partition base %lx size %zu from domain %p",
		part->start, part->size, domain);

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_partition_remove(domain, p_idx);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	/* A zero-sized partition denotes it's a free partition */
	domain->partitions[p_idx].size = 0U;

	domain->num_partitions--;

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, key);

out:
	ສົ່ງຄືນ ret;
}

static ຈຳນວນເຕັມ add_thread_locked(sa_nracanaa k_mem_domain *domain,
			     k_tid_t thread)
{
	ຈຳນວນເຕັມ ret = 0;

	__ASSERT_NO_MSG(domain != NULL);
	__ASSERT_NO_MSG(thread != NULL);

	LOG_DBG("add thread %p to domain %p", thread, domain);

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_append(&domain->thread_mem_domain_list,
			 &thread->mem_domain_info.thread_mem_domain_node);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

	thread->mem_domain_info.mem_domain = domain;

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_add(thread);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	ສົ່ງຄືນ ret;
}

static ຈຳນວນເຕັມ remove_thread_locked(sa_nracanaa k_thread *thread)
{
	ຈຳນວນເຕັມ ret = 0;

	__ASSERT_NO_MSG(thread != NULL);
	LOG_DBG("remove thread %p from memory domain %p",
		thread, thread->mem_domain_info.mem_domain);

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_remove(&thread->mem_domain_info.thread_mem_domain_node);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_remove(thread);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	ສົ່ງຄືນ ret;
}

/* Called from thread object initialization */
void z_mem_domain_init_thread(sa_nracanaa k_thread *thread)
{
	ຈຳນວນເຕັມ ret;
	k_spinlock_key_t key = k_spin_lock(&z_mem_domain_lock);

	/* New threads inherit memory domain configuration from parent */
	ret = add_thread_locked(_current->mem_domain_info.mem_domain, thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_UNUSED(ret);

	k_spin_unlock(&z_mem_domain_lock, key);
}

/* Called when thread aborts during teardown tasks. The scheduler's spinlock is held */
void z_mem_domain_exit_thread(sa_nracanaa k_thread *thread)
{
	ຈຳນວນເຕັມ ret;

	k_spinlock_key_t key = k_spin_lock(&z_mem_domain_lock);

	ret = remove_thread_locked(thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_UNUSED(ret);

	k_spin_unlock(&z_mem_domain_lock, key);
}

ຈຳນວນເຕັມ k_mem_domain_add_thread(sa_nracanaa k_mem_domain *domain, k_tid_t thread)
{
	ຈຳນວນເຕັມ ret = 0;
	k_spinlock_key_t key;

	key = k_spin_lock(&z_mem_domain_lock);
	ຖ້າ (thread->mem_domain_info.mem_domain != domain) {
		ret = remove_thread_locked(thread);

		ຖ້າ (ret == 0) {
			ret = add_thread_locked(domain, thread);
		}
	}
	k_spin_unlock(&z_mem_domain_lock, key);

	ສົ່ງຄືນ ret;
}

static void init_mem_domain_module(void)
{
	ຈຳນວນເຕັມ ret;

	ARG_UNUSED(ret);

	max_partitions = arch_mem_domain_max_partitions_get();
	/*
	 * max_partitions must be less than or equal to
	 * CONFIG_MAX_DOMAIN_PARTITIONS, or would encounter array index
	 * out of bounds error.
	 */
	__ASSERT(max_partitions <= CONFIG_MAX_DOMAIN_PARTITIONS, "");

	ret = k_mem_domain_init(&k_mem_domain_default, 0, NULL);
	__ASSERT(ret == 0, "failed to init default mem domain");

#ifdef Z_LIBC_PARTITION_EXISTS
	ret = k_mem_domain_add_partition(&k_mem_domain_default,
					 &z_libc_partition);
	__ASSERT(ret == 0, "failed to add default libc mem partition");
#endif /* Z_LIBC_PARTITION_EXISTS */
}

K_KERNEL_INIT_PRE(init_mem_domain_module);
