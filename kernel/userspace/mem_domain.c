/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাব <zephyr/kernel.h>
#অন্তর্ভাব <kernel_internal.h>
#অন্তর্ভাব <zephyr/sys/__assert.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <zephyr/spinlock.h>
#অন্তর্ভাব <zephyr/sys/check.h>
#অন্তর্ভাব <zephyr/sys/libc-hooks.h>
#অন্তর্ভাব <zephyr/logging/log.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

কাঠামো k_spinlock z_mem_domain_lock;
স্থির uint8_t max_partitions;

কাঠামো k_mem_domain k_mem_domain_default;

স্থির bool check_add_partition(কাঠামো k_mem_domain *domain,
				কাঠামো k_mem_partition *part)
{

	পূর্ণসংখ্যা i;
	uintptr_t pstart, pend, dstart, dend;

	যদি (part == ফাঁকা) {
		LOG_ERR("NULL k_mem_partition provided");
		ফেরত false;
	}

#ifdef CONFIG_EXECUTE_XOR_WRITE
	/* Arches where execution cannot be disabled should always return
	 * false to this check
	 */
	যদি (K_MEM_PARTITION_IS_EXECUTABLE(part->attr) &&
	    K_MEM_PARTITION_IS_WRITABLE(part->attr)) {
		LOG_ERR("partition is writable and executable <start %lx>",
			part->start);
		ফেরত false;
	}
#endif /* CONFIG_EXECUTE_XOR_WRITE */

	যদি (part->আকার_2 == 0U) {
		LOG_ERR("zero sized partition at %p with base 0x%lx",
			part, part->start);
		ফেরত false;
	}

	pstart = part->start;
	pend = part->start + part->আকার_2;

	যদি (pend <= pstart) {
		LOG_ERR("invalid partition %p, wraparound detected. base 0x%lx size %zu",
			part, part->start, part->আকার_2);
		ফেরত false;
	}

	/* Check that this partition doesn't overlap any existing ones already
	 * in the domain
	 */
	জন্য (i = 0; i < domain->অঙ্ক_partitions; i++) {
		কাঠামো k_mem_partition *dpart = &domain->partitions[i];

		যদি (dpart->আকার_2 == 0U) {
			/* Unused slot */
			চালু;
		}

		dstart = dpart->start;
		dend = dstart + dpart->আকার_2;

		যদি (pend > dstart && dend > pstart) {
			LOG_ERR("partition %p base %lx (size %zu) overlaps existing base %lx (size %zu)",
				part, part->start, part->আকার_2,
				dpart->start, dpart->আকার_2);
			ফেরত false;
		}
	}

	ফেরত true;
}

পূর্ণসংখ্যা k_mem_domain_init(কাঠামো k_mem_domain *domain, uint8_t অঙ্ক_parts,
		      কাঠামো k_mem_partition *parts[])
{
	k_spinlock_চাবি_t চাবি;
	পূর্ণসংখ্যা ret = 0;

	CHECKIF(domain == ফাঁকা) {
		ret = -EINVAL;
		যাও out;
	}

	CHECKIF(!(অঙ্ক_parts == 0U || parts != ফাঁকা)) {
		LOG_ERR("parts array is NULL and num_parts is nonzero");
		ret = -EINVAL;
		যাও out;
	}

	CHECKIF(!(অঙ্ক_parts <= max_partitions)) {
		LOG_ERR("num_parts of %d exceeds maximum allowable partitions (%d)",
			অঙ্ক_parts, max_partitions);
		ret = -EINVAL;
		যাও out;
	}

	চাবি = k_spin_lock(&z_mem_domain_lock);

	domain->অঙ্ক_partitions = 0U;
	(শূন্য)memset(domain->partitions, 0, আকার(domain->partitions));

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_init(&domain->thread_mem_domain_list);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#ifdef CONFIG_ARCH_MEM_DOMAIN_তথ্য
	ret = arch_mem_domain_init(domain);

	যদি (ret != 0) {
		LOG_ERR("architecture-specific initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		যাও unlock_out;
	}
#endif /* CONFIG_ARCH_MEM_DOMAIN_DATA */
	যদি (অঙ্ক_parts != 0U) {
		uint32_t i;

		জন্য (i = 0U; i < অঙ্ক_parts; i++) {
			CHECKIF(!check_add_partition(domain, parts[i])) {
				LOG_ERR("invalid partition index %d (%p)",
					i, parts[i]);
				ret = -EINVAL;
				যাও unlock_out;
			}

			domain->partitions[i] = *parts[i];
			domain->অঙ্ক_partitions++;
#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
			পূর্ণসংখ্যা ret2 = arch_mem_domain_partition_add(domain, i);

			ARG_UNUSED(ret2);
			CHECKIF(ret2 != 0) {
				ret = ret2;
			}
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */
		}
	}

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, চাবি);

out:
	ফেরত ret;
}

পূর্ণসংখ্যা k_mem_domain_deinit(কাঠামো k_mem_domain *domain)
{
#যদি defined(CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT)
	k_spinlock_চাবি_t চাবি;
	পূর্ণসংখ্যা ret = 0;

	CHECKIF(domain == ফাঁকা) {
		ret = -EINVAL;
		যাও out;
	}

	যদি (domain == &k_mem_domain_default) {
		/* Default memory domain must be there forever. */
		ret = -EINVAL;
		যাও out;
	}

	চাবি = k_spin_lock(&z_mem_domain_lock);

	/* Must make sure there are no threads associated with this memory
	 * domain anymore. Or else these threads will run with an invalid
	 * memory domain.
	 */
	যদি (!sys_dlist_is_empty(&domain->thread_mem_domain_list)) {
		ret = -EBUSY;
		যাও unlock_out;
	}

	ret = arch_mem_domain_deinit(domain);
	যদি (ret != 0) {
		LOG_ERR("architecture-specific de-initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		যাও unlock_out;
	}

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, চাবি);

out:
	ফেরত ret;
#অন্যথা  /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
	ARG_UNUSED(domain);
	ফেরত -ENOTSUP;
#endif /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
}

পূর্ণসংখ্যা k_mem_domain_add_partition(কাঠামো k_mem_domain *domain,
			       কাঠামো k_mem_partition *part)
{
	পূর্ণসংখ্যা p_idx;
	k_spinlock_চাবি_t চাবি;
	পূর্ণসংখ্যা ret = 0;

	CHECKIF(domain == ফাঁকা) {
		ret = -EINVAL;
		যাও out;
	}

	CHECKIF(!check_add_partition(domain, part)) {
		LOG_ERR("invalid partition %p", part);
		ret = -EINVAL;
		যাও out;
	}

	চাবি = k_spin_lock(&z_mem_domain_lock);

	জন্য (p_idx = 0; p_idx < max_partitions; p_idx++) {
		/* A zero-sized partition denotes it's a free partition */
		যদি (domain->partitions[p_idx].আকার_2 == 0U) {
			বিরতি;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no free partition slots available");
		ret = -ENOSPC;
		যাও unlock_out;
	}

	LOG_DBG("add partition base %lx size %zu to domain %p",
		part->start, part->আকার_2, domain);

	domain->partitions[p_idx].start = part->start;
	domain->partitions[p_idx].আকার_2 = part->আকার_2;
	domain->partitions[p_idx].attr = part->attr;

	domain->অঙ্ক_partitions++;

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_partition_add(domain, p_idx);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, চাবি);

out:
	ফেরত ret;
}

পূর্ণসংখ্যা k_mem_domain_remove_partition(কাঠামো k_mem_domain *domain,
				  কাঠামো k_mem_partition *part)
{
	পূর্ণসংখ্যা p_idx;
	k_spinlock_চাবি_t চাবি;
	পূর্ণসংখ্যা ret = 0;

	CHECKIF((domain == ফাঁকা) || (part == ফাঁকা)) {
		ret = -EINVAL;
		যাও out;
	}

	চাবি = k_spin_lock(&z_mem_domain_lock);

	/* find a partition that matches the given start and size */
	জন্য (p_idx = 0; p_idx < max_partitions; p_idx++) {
		যদি ((domain->partitions[p_idx].start == part->start) &&
		    (domain->partitions[p_idx].আকার_2 == part->আকার_2)) {
			বিরতি;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no matching partition found");
		ret = -ENOENT;
		যাও unlock_out;
	}

	LOG_DBG("remove partition base %lx size %zu from domain %p",
		part->start, part->আকার_2, domain);

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_partition_remove(domain, p_idx);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	/* A zero-sized partition denotes it's a free partition */
	domain->partitions[p_idx].আকার_2 = 0U;

	domain->অঙ্ক_partitions--;

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, চাবি);

out:
	ফেরত ret;
}

স্থির পূর্ণসংখ্যা add_thread_locked(কাঠামো k_mem_domain *domain,
			     k_tid_t thread)
{
	পূর্ণসংখ্যা ret = 0;

	__ASSERT_NO_MSG(domain != ফাঁকা);
	__ASSERT_NO_MSG(thread != ফাঁকা);

	LOG_DBG("add thread %p to domain %p", thread, domain);

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_append(&domain->thread_mem_domain_list,
			 &thread->mem_domain_info.thread_mem_domain_গ্রন্থি);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

	thread->mem_domain_info.mem_domain = domain;

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_add(thread);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	ফেরত ret;
}

স্থির পূর্ণসংখ্যা remove_thread_locked(কাঠামো k_thread *thread)
{
	পূর্ণসংখ্যা ret = 0;

	__ASSERT_NO_MSG(thread != ফাঁকা);
	LOG_DBG("remove thread %p from memory domain %p",
		thread, thread->mem_domain_info.mem_domain);

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_remove(&thread->mem_domain_info.thread_mem_domain_গ্রন্থি);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_remove(thread);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	ফেরত ret;
}

/* Called from thread object initialization */
শূন্য z_mem_domain_init_thread(কাঠামো k_thread *thread)
{
	পূর্ণসংখ্যা ret;
	k_spinlock_চাবি_t চাবি = k_spin_lock(&z_mem_domain_lock);

	/* New threads inherit memory domain configuration from parent */
	ret = add_thread_locked(_current->mem_domain_info.mem_domain, thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_UNUSED(ret);

	k_spin_unlock(&z_mem_domain_lock, চাবি);
}

/* Called when thread aborts during teardown tasks. The scheduler's spinlock is held */
শূন্য z_mem_domain_exit_thread(কাঠামো k_thread *thread)
{
	পূর্ণসংখ্যা ret;

	k_spinlock_চাবি_t চাবি = k_spin_lock(&z_mem_domain_lock);

	ret = remove_thread_locked(thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_UNUSED(ret);

	k_spin_unlock(&z_mem_domain_lock, চাবি);
}

পূর্ণসংখ্যা k_mem_domain_add_thread(কাঠামো k_mem_domain *domain, k_tid_t thread)
{
	পূর্ণসংখ্যা ret = 0;
	k_spinlock_চাবি_t চাবি;

	চাবি = k_spin_lock(&z_mem_domain_lock);
	যদি (thread->mem_domain_info.mem_domain != domain) {
		ret = remove_thread_locked(thread);

		যদি (ret == 0) {
			ret = add_thread_locked(domain, thread);
		}
	}
	k_spin_unlock(&z_mem_domain_lock, চাবি);

	ফেরত ret;
}

স্থির শূন্য init_mem_domain_module(শূন্য)
{
	পূর্ণসংখ্যা ret;

	ARG_UNUSED(ret);

	max_partitions = arch_mem_domain_max_partitions_get();
	/*
	 * max_partitions must be less than or equal to
	 * CONFIG_MAX_DOMAIN_PARTITIONS, or would encounter array index
	 * out of bounds error.
	 */
	__ASSERT(max_partitions <= CONFIG_MAX_DOMAIN_PARTITIONS, "");

	ret = k_mem_domain_init(&k_mem_domain_default, 0, ফাঁকা);
	__ASSERT(ret == 0, "failed to init default mem domain");

#ifdef Z_LIBC_PARTITION_EXISTS
	ret = k_mem_domain_add_partition(&k_mem_domain_default,
					 &z_libc_partition);
	__ASSERT(ret == 0, "failed to add default libc mem partition");
#endif /* Z_LIBC_PARTITION_EXISTS */
}

K_KERNEL_INIT_PRE(init_mem_domain_module);
