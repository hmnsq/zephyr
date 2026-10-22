/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>
#شامل <kernel_internal.h>
#شامل <zephyr/sys/__assert.h>
#شامل <stdbool.h>
#شامل <zephyr/spinlock.h>
#شامل <zephyr/sys/check.h>
#شامل <zephyr/sys/libc-hooks.h>
#شامل <zephyr/logging/log.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

ساختار k_spinlock z_mem_domain_lock;
ایستا uint8_t max_partitions;

ساختار k_mem_domain k_mem_domain_default;

ایستا bool check_add_partition(ساختار k_mem_domain *domain,
				ساختار k_mem_partition *part)
{

	صحیح i;
	uintptr_t pstart, pend, dstart, dend;

	اگر (part == پوچ) {
		LOG_ERR("NULL k_mem_partition provided");
		بازگشت false;
	}

#ifdef CONFIG_EXECUTE_XOR_WRITE
	/* Arches where execution cannot be disabled should always return
	 * false to this check
	 */
	اگر (K_MEM_PARTITION_IS_EXECUTABLE(part->attr) &&
	    K_MEM_PARTITION_IS_WRITABLE(part->attr)) {
		LOG_ERR("partition is writable and executable <start %lx>",
			part->start);
		بازگشت false;
	}
#endif /* CONFIG_EXECUTE_XOR_WRITE */

	اگر (part->size == 0U) {
		LOG_ERR("zero sized partition at %p with base 0x%lx",
			part, part->start);
		بازگشت false;
	}

	pstart = part->start;
	pend = part->start + part->size;

	اگر (pend <= pstart) {
		LOG_ERR("invalid partition %p, wraparound detected. base 0x%lx size %zu",
			part, part->start, part->size);
		بازگشت false;
	}

	/* Check that this partition doesn't overlap any existing ones already
	 * in the domain
	 */
	برای (i = 0; i < domain->رقم_partitions; i++) {
		ساختار k_mem_partition *dpart = &domain->partitions[i];

		اگر (dpart->size == 0U) {
			/* Unused slot */
			ادامه;
		}

		dstart = dpart->start;
		dend = dstart + dpart->size;

		اگر (pend > dstart && dend > pstart) {
			LOG_ERR("partition %p base %lx (size %zu) overlaps existing base %lx (size %zu)",
				part, part->start, part->size,
				dpart->start, dpart->size);
			بازگشت false;
		}
	}

	بازگشت true;
}

صحیح k_mem_domain_init(ساختار k_mem_domain *domain, uint8_t رقم_parts,
		      ساختار k_mem_partition *parts[])
{
	k_spinlock_کلید_t کلید;
	صحیح ret = 0;

	CHECKIF(domain == پوچ) {
		ret = -EINVAL;
		برو out;
	}

	CHECKIF(!(رقم_parts == 0U || parts != پوچ)) {
		LOG_ERR("parts array is NULL and num_parts is nonzero");
		ret = -EINVAL;
		برو out;
	}

	CHECKIF(!(رقم_parts <= max_partitions)) {
		LOG_ERR("num_parts of %d exceeds maximum allowable partitions (%d)",
			رقم_parts, max_partitions);
		ret = -EINVAL;
		برو out;
	}

	کلید = k_spin_lock(&z_mem_domain_lock);

	domain->رقم_partitions = 0U;
	(تهی)memset(domain->partitions, 0, اندازه(domain->partitions));

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_init(&domain->thread_mem_domain_list);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#ifdef CONFIG_ARCH_MEM_DOMAIN_DATA
	ret = arch_mem_domain_init(domain);

	اگر (ret != 0) {
		LOG_ERR("architecture-specific initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		برو unlock_out;
	}
#endif /* CONFIG_ARCH_MEM_DOMAIN_DATA */
	اگر (رقم_parts != 0U) {
		uint32_t i;

		برای (i = 0U; i < رقم_parts; i++) {
			CHECKIF(!check_add_partition(domain, parts[i])) {
				LOG_ERR("invalid partition index %d (%p)",
					i, parts[i]);
				ret = -EINVAL;
				برو unlock_out;
			}

			domain->partitions[i] = *parts[i];
			domain->رقم_partitions++;
#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
			صحیح ret2 = arch_mem_domain_partition_add(domain, i);

			ARG_UNUSED(ret2);
			CHECKIF(ret2 != 0) {
				ret = ret2;
			}
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */
		}
	}

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, کلید);

out:
	بازگشت ret;
}

صحیح k_mem_domain_deinit(ساختار k_mem_domain *domain)
{
#اگر defined(CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT)
	k_spinlock_کلید_t کلید;
	صحیح ret = 0;

	CHECKIF(domain == پوچ) {
		ret = -EINVAL;
		برو out;
	}

	اگر (domain == &k_mem_domain_default) {
		/* Default memory domain must be there forever. */
		ret = -EINVAL;
		برو out;
	}

	کلید = k_spin_lock(&z_mem_domain_lock);

	/* Must make sure there are no threads associated with this memory
	 * domain anymore. Or else these threads will run with an invalid
	 * memory domain.
	 */
	اگر (!sys_dlist_is_empty(&domain->thread_mem_domain_list)) {
		ret = -EBUSY;
		برو unlock_out;
	}

	ret = arch_mem_domain_deinit(domain);
	اگر (ret != 0) {
		LOG_ERR("architecture-specific de-initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		برو unlock_out;
	}

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, کلید);

out:
	بازگشت ret;
#وگرنه  /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
	ARG_UNUSED(domain);
	بازگشت -ENOTSUP;
#endif /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
}

صحیح k_mem_domain_add_partition(ساختار k_mem_domain *domain,
			       ساختار k_mem_partition *part)
{
	صحیح p_idx;
	k_spinlock_کلید_t کلید;
	صحیح ret = 0;

	CHECKIF(domain == پوچ) {
		ret = -EINVAL;
		برو out;
	}

	CHECKIF(!check_add_partition(domain, part)) {
		LOG_ERR("invalid partition %p", part);
		ret = -EINVAL;
		برو out;
	}

	کلید = k_spin_lock(&z_mem_domain_lock);

	برای (p_idx = 0; p_idx < max_partitions; p_idx++) {
		/* A zero-sized partition denotes it's a free partition */
		اگر (domain->partitions[p_idx].size == 0U) {
			بشکن;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no free partition slots available");
		ret = -ENOSPC;
		برو unlock_out;
	}

	LOG_DBG("add partition base %lx size %zu to domain %p",
		part->start, part->size, domain);

	domain->partitions[p_idx].start = part->start;
	domain->partitions[p_idx].size = part->size;
	domain->partitions[p_idx].attr = part->attr;

	domain->رقم_partitions++;

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_partition_add(domain, p_idx);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, کلید);

out:
	بازگشت ret;
}

صحیح k_mem_domain_remove_partition(ساختار k_mem_domain *domain,
				  ساختار k_mem_partition *part)
{
	صحیح p_idx;
	k_spinlock_کلید_t کلید;
	صحیح ret = 0;

	CHECKIF((domain == پوچ) || (part == پوچ)) {
		ret = -EINVAL;
		برو out;
	}

	کلید = k_spin_lock(&z_mem_domain_lock);

	/* find a partition that matches the given start and size */
	برای (p_idx = 0; p_idx < max_partitions; p_idx++) {
		اگر ((domain->partitions[p_idx].start == part->start) &&
		    (domain->partitions[p_idx].size == part->size)) {
			بشکن;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no matching partition found");
		ret = -ENOENT;
		برو unlock_out;
	}

	LOG_DBG("remove partition base %lx size %zu from domain %p",
		part->start, part->size, domain);

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_partition_remove(domain, p_idx);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	/* A zero-sized partition denotes it's a free partition */
	domain->partitions[p_idx].size = 0U;

	domain->رقم_partitions--;

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, کلید);

out:
	بازگشت ret;
}

ایستا صحیح add_thread_locked(ساختار k_mem_domain *domain,
			     k_tid_t thread)
{
	صحیح ret = 0;

	__ASSERT_NO_MSG(domain != پوچ);
	__ASSERT_NO_MSG(thread != پوچ);

	LOG_DBG("add thread %p to domain %p", thread, domain);

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_append(&domain->thread_mem_domain_list,
			 &thread->mem_domain_info.thread_mem_domain_گره);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

	thread->mem_domain_info.mem_domain = domain;

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_add(thread);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	بازگشت ret;
}

ایستا صحیح remove_thread_locked(ساختار k_thread *thread)
{
	صحیح ret = 0;

	__ASSERT_NO_MSG(thread != پوچ);
	LOG_DBG("remove thread %p from memory domain %p",
		thread, thread->mem_domain_info.mem_domain);

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_remove(&thread->mem_domain_info.thread_mem_domain_گره);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_remove(thread);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	بازگشت ret;
}

/* Called from thread object initialization */
تهی z_mem_domain_init_thread(ساختار k_thread *thread)
{
	صحیح ret;
	k_spinlock_کلید_t کلید = k_spin_lock(&z_mem_domain_lock);

	/* New threads inherit memory domain configuration from parent */
	ret = add_thread_locked(_current->mem_domain_info.mem_domain, thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_UNUSED(ret);

	k_spin_unlock(&z_mem_domain_lock, کلید);
}

/* Called when thread aborts during teardown tasks. The scheduler's spinlock is held */
تهی z_mem_domain_exit_thread(ساختار k_thread *thread)
{
	صحیح ret;

	k_spinlock_کلید_t کلید = k_spin_lock(&z_mem_domain_lock);

	ret = remove_thread_locked(thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_UNUSED(ret);

	k_spin_unlock(&z_mem_domain_lock, کلید);
}

صحیح k_mem_domain_add_thread(ساختار k_mem_domain *domain, k_tid_t thread)
{
	صحیح ret = 0;
	k_spinlock_کلید_t کلید;

	کلید = k_spin_lock(&z_mem_domain_lock);
	اگر (thread->mem_domain_info.mem_domain != domain) {
		ret = remove_thread_locked(thread);

		اگر (ret == 0) {
			ret = add_thread_locked(domain, thread);
		}
	}
	k_spin_unlock(&z_mem_domain_lock, کلید);

	بازگشت ret;
}

ایستا تهی init_mem_domain_module(تهی)
{
	صحیح ret;

	ARG_UNUSED(ret);

	max_partitions = arch_mem_domain_max_partitions_get();
	/*
	 * max_partitions must be less than or equal to
	 * CONFIG_MAX_DOMAIN_PARTITIONS, or would encounter array index
	 * out of bounds error.
	 */
	__ASSERT(max_partitions <= CONFIG_MAX_DOMAIN_PARTITIONS, "");

	ret = k_mem_domain_init(&k_mem_domain_default, 0, پوچ);
	__ASSERT(ret == 0, "failed to init default mem domain");

#ifdef Z_LIBC_PARTITION_EXISTS
	ret = k_mem_domain_add_partition(&k_mem_domain_default,
					 &z_libc_partition);
	__ASSERT(ret == 0, "failed to add default libc mem partition");
#endif /* Z_LIBC_PARTITION_EXISTS */
}

K_KERNEL_INIT_PRE(init_mem_domain_module);
