/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <kernel_internal.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/__assert.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/spinlock.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/check.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/libc-hooks.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/logging/log.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

ସଂରଚନା k_spinlock z_mem_domain_lock;
ସ୍ଥିର uint8_t max_partitions;

ସଂରଚନା k_mem_domain k_mem_domain_default;

ସ୍ଥିର bool check_add_partition(ସଂରଚନା k_mem_domain *domain,
				ସଂରଚନା k_mem_partition *part)
{

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	uintptr_t pstart, pend, dstart, dend;

	ଯଦି (part == ଖାଲି) {
		LOG_ERR("NULL k_mem_partition provided");
		ଫେରନ୍ତୁ false;
	}

#ifdef CONFIG_EXECUTE_XOR_WRITE
	/* Arches where execution cannot be disabled should always return
	 * false to this check
	 */
	ଯଦି (K_MEM_PARTITION_IS_EXECUTABLE(part->attr) &&
	    K_MEM_PARTITION_IS_WRITABLE(part->attr)) {
		LOG_ERR("partition is writable and executable <start %lx>",
			part->start);
		ଫେରନ୍ତୁ false;
	}
#endif /* CONFIG_EXECUTE_XOR_WRITE */

	ଯଦି (part->size == 0U) {
		LOG_ERR("zero sized partition at %p with base 0x%lx",
			part, part->start);
		ଫେରନ୍ତୁ false;
	}

	pstart = part->start;
	pend = part->start + part->size;

	ଯଦି (pend <= pstart) {
		LOG_ERR("invalid partition %p, wraparound detected. base 0x%lx size %zu",
			part, part->start, part->size);
		ଫେରନ୍ତୁ false;
	}

	/* Check that this partition doesn't overlap any existing ones already
	 * in the domain
	 */
	ପାଇଁ (i = 0; i < domain->ଅଙ୍କ_partitions; i++) {
		ସଂରଚନା k_mem_partition *dpart = &domain->partitions[i];

		ଯଦି (dpart->size == 0U) {
			/* Unused slot */
			ଜାରି;
		}

		dstart = dpart->start;
		dend = dstart + dpart->size;

		ଯଦି (pend > dstart && dend > pstart) {
			LOG_ERR("partition %p base %lx (size %zu) overlaps existing base %lx (size %zu)",
				part, part->start, part->size,
				dpart->start, dpart->size);
			ଫେରନ୍ତୁ false;
		}
	}

	ଫେରନ୍ତୁ true;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_mem_domain_init(ସଂରଚନା k_mem_domain *domain, uint8_t ଅଙ୍କ_parts,
		      ସଂରଚନା k_mem_partition *parts[])
{
	k_spinlock_ଚାବି_t ଚାବି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;

	CHECKIF(domain == ଖାଲି) {
		ret = -EINVAL;
		ଯାଅ out;
	}

	CHECKIF(!(ଅଙ୍କ_parts == 0U || parts != ଖାଲି)) {
		LOG_ERR("parts array is NULL and num_parts is nonzero");
		ret = -EINVAL;
		ଯାଅ out;
	}

	CHECKIF(!(ଅଙ୍କ_parts <= max_partitions)) {
		LOG_ERR("num_parts of %d exceeds maximum allowable partitions (%d)",
			ଅଙ୍କ_parts, max_partitions);
		ret = -EINVAL;
		ଯାଅ out;
	}

	ଚାବି = k_spin_lock(&z_mem_domain_lock);

	domain->ଅଙ୍କ_partitions = 0U;
	(ଶୂନ୍ୟ)memset(domain->partitions, 0, ଆକାର(domain->partitions));

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_init(&domain->thread_mem_domain_list);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#ifdef CONFIG_ARCH_MEM_DOMAIN_DATA
	ret = arch_mem_domain_init(domain);

	ଯଦି (ret != 0) {
		LOG_ERR("architecture-specific initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		ଯାଅ unlock_out;
	}
#endif /* CONFIG_ARCH_MEM_DOMAIN_DATA */
	ଯଦି (ଅଙ୍କ_parts != 0U) {
		uint32_t i;

		ପାଇଁ (i = 0U; i < ଅଙ୍କ_parts; i++) {
			CHECKIF(!check_add_partition(domain, parts[i])) {
				LOG_ERR("invalid partition index %d (%p)",
					i, parts[i]);
				ret = -EINVAL;
				ଯାଅ unlock_out;
			}

			domain->partitions[i] = *parts[i];
			domain->ଅଙ୍କ_partitions++;
#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret2 = arch_mem_domain_partition_add(domain, i);

			ARG_UNUSED(ret2);
			CHECKIF(ret2 != 0) {
				ret = ret2;
			}
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */
		}
	}

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, ଚାବି);

out:
	ଫେରନ୍ତୁ ret;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_mem_domain_deinit(ସଂରଚନା k_mem_domain *domain)
{
#ଯଦି defined(CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT)
	k_spinlock_ଚାବି_t ଚାବି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;

	CHECKIF(domain == ଖାଲି) {
		ret = -EINVAL;
		ଯାଅ out;
	}

	ଯଦି (domain == &k_mem_domain_default) {
		/* Default memory domain must be there forever. */
		ret = -EINVAL;
		ଯାଅ out;
	}

	ଚାବି = k_spin_lock(&z_mem_domain_lock);

	/* Must make sure there are no threads associated with this memory
	 * domain anymore. Or else these threads will run with an invalid
	 * memory domain.
	 */
	ଯଦି (!sys_dlist_is_empty(&domain->thread_mem_domain_list)) {
		ret = -EBUSY;
		ଯାଅ unlock_out;
	}

	ret = arch_mem_domain_deinit(domain);
	ଯଦି (ret != 0) {
		LOG_ERR("architecture-specific de-initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		ଯାଅ unlock_out;
	}

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, ଚାବି);

out:
	ଫେରନ୍ତୁ ret;
#ଅନ୍ୟଥା  /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
	ARG_UNUSED(domain);
	ଫେରନ୍ତୁ -ENOTSUP;
#endif /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_mem_domain_add_partition(ସଂରଚନା k_mem_domain *domain,
			       ସଂରଚନା k_mem_partition *part)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା p_idx;
	k_spinlock_ଚାବି_t ଚାବି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;

	CHECKIF(domain == ଖାଲି) {
		ret = -EINVAL;
		ଯାଅ out;
	}

	CHECKIF(!check_add_partition(domain, part)) {
		LOG_ERR("invalid partition %p", part);
		ret = -EINVAL;
		ଯାଅ out;
	}

	ଚାବି = k_spin_lock(&z_mem_domain_lock);

	ପାଇଁ (p_idx = 0; p_idx < max_partitions; p_idx++) {
		/* A zero-sized partition denotes it's a free partition */
		ଯଦି (domain->partitions[p_idx].size == 0U) {
			ଭାଙ୍ଗ;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no free partition slots available");
		ret = -ENOSPC;
		ଯାଅ unlock_out;
	}

	LOG_DBG("add partition base %lx size %zu to domain %p",
		part->start, part->size, domain);

	domain->partitions[p_idx].start = part->start;
	domain->partitions[p_idx].size = part->size;
	domain->partitions[p_idx].attr = part->attr;

	domain->ଅଙ୍କ_partitions++;

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_partition_add(domain, p_idx);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, ଚାବି);

out:
	ଫେରନ୍ତୁ ret;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_mem_domain_remove_partition(ସଂରଚନା k_mem_domain *domain,
				  ସଂରଚନା k_mem_partition *part)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା p_idx;
	k_spinlock_ଚାବି_t ଚାବି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;

	CHECKIF((domain == ଖାଲି) || (part == ଖାଲି)) {
		ret = -EINVAL;
		ଯାଅ out;
	}

	ଚାବି = k_spin_lock(&z_mem_domain_lock);

	/* find a partition that matches the given start and size */
	ପାଇଁ (p_idx = 0; p_idx < max_partitions; p_idx++) {
		ଯଦି ((domain->partitions[p_idx].start == part->start) &&
		    (domain->partitions[p_idx].size == part->size)) {
			ଭାଙ୍ଗ;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no matching partition found");
		ret = -ENOENT;
		ଯାଅ unlock_out;
	}

	LOG_DBG("remove partition base %lx size %zu from domain %p",
		part->start, part->size, domain);

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_partition_remove(domain, p_idx);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	/* A zero-sized partition denotes it's a free partition */
	domain->partitions[p_idx].size = 0U;

	domain->ଅଙ୍କ_partitions--;

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, ଚାବି);

out:
	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା add_thread_locked(ସଂରଚନା k_mem_domain *domain,
			     k_tid_t thread)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;

	__ASSERT_NO_MSG(domain != ଖାଲି);
	__ASSERT_NO_MSG(thread != ଖାଲି);

	LOG_DBG("add thread %p to domain %p", thread, domain);

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_append(&domain->thread_mem_domain_list,
			 &thread->mem_domain_info.thread_mem_domain_ଗଣ୍ଠି);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

	thread->mem_domain_info.mem_domain = domain;

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_add(thread);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା remove_thread_locked(ସଂରଚନା k_thread *thread)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;

	__ASSERT_NO_MSG(thread != ଖାଲି);
	LOG_DBG("remove thread %p from memory domain %p",
		thread, thread->mem_domain_info.mem_domain);

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_remove(&thread->mem_domain_info.thread_mem_domain_ଗଣ୍ଠି);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_remove(thread);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	ଫେରନ୍ତୁ ret;
}

/* Called from thread object initialization */
ଶୂନ୍ୟ z_mem_domain_init_thread(ସଂରଚନା k_thread *thread)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&z_mem_domain_lock);

	/* New threads inherit memory domain configuration from parent */
	ret = add_thread_locked(_current->mem_domain_info.mem_domain, thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_UNUSED(ret);

	k_spin_unlock(&z_mem_domain_lock, ଚାବି);
}

/* Called when thread aborts during teardown tasks. The scheduler's spinlock is held */
ଶୂନ୍ୟ z_mem_domain_exit_thread(ସଂରଚନା k_thread *thread)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&z_mem_domain_lock);

	ret = remove_thread_locked(thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_UNUSED(ret);

	k_spin_unlock(&z_mem_domain_lock, ଚାବି);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_mem_domain_add_thread(ସଂରଚନା k_mem_domain *domain, k_tid_t thread)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;
	k_spinlock_ଚାବି_t ଚାବି;

	ଚାବି = k_spin_lock(&z_mem_domain_lock);
	ଯଦି (thread->mem_domain_info.mem_domain != domain) {
		ret = remove_thread_locked(thread);

		ଯଦି (ret == 0) {
			ret = add_thread_locked(domain, thread);
		}
	}
	k_spin_unlock(&z_mem_domain_lock, ଚାବି);

	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ଶୂନ୍ୟ init_mem_domain_module(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ARG_UNUSED(ret);

	max_partitions = arch_mem_domain_max_partitions_get();
	/*
	 * max_partitions must be less than or equal to
	 * CONFIG_MAX_DOMAIN_PARTITIONS, or would encounter array index
	 * out of bounds error.
	 */
	__ASSERT(max_partitions <= CONFIG_MAX_DOMAIN_PARTITIONS, "");

	ret = k_mem_domain_init(&k_mem_domain_default, 0, ଖାଲି);
	__ASSERT(ret == 0, "failed to init default mem domain");

#ifdef Z_LIBC_PARTITION_EXISTS
	ret = k_mem_domain_add_partition(&k_mem_domain_default,
					 &z_libc_partition);
	__ASSERT(ret == 0, "failed to add default libc mem partition");
#endif /* Z_LIBC_PARTITION_EXISTS */
}

K_KERNEL_INIT_PRE(init_mem_domain_module);
