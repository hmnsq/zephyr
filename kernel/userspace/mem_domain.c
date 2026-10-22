/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <kernel_internal.h>
#ضمن <zephyr/sys/__assert.h>
#ضمن <stdbool.h>
#ضمن <zephyr/spinlock.h>
#ضمن <zephyr/sys/check.h>
#ضمن <zephyr/sys/libc-hooks.h>
#ضمن <zephyr/logging/log.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

بنية k_spinlock z_mem_domain_lock;
ساكن uint8_t max_partitions;

بنية k_mem_domain k_mem_domain_default;

ساكن bool check_add_partition(بنية k_mem_domain *domain,
				بنية k_mem_partition *part)
{

	صحيح i;
	uintptr_t pstart, pend, dstart, dend;

	אם (part == عدم) {
		LOG_ERR("NULL k_mem_partition provided");
		החזר false;
	}

#ifdef CONFIG_EXECUTE_XOR_WRITE
	/* Arches where execution cannot be disabled should always return
	 * false to this check
	 */
	אם (K_MEM_PARTITION_IS_EXECUTABLE(part->attr) &&
	    K_MEM_PARTITION_IS_WRITABLE(part->attr)) {
		LOG_ERR("partition is writable and executable <start %lx>",
			part->start);
		החזר false;
	}
#endif /* CONFIG_EXECUTE_XOR_WRITE */

	אם (part->size == 0U) {
		LOG_ERR("zero sized partition at %p with base 0x%lx",
			part, part->start);
		החזר false;
	}

	pstart = part->start;
	pend = part->start + part->size;

	אם (pend <= pstart) {
		LOG_ERR("invalid partition %p, wraparound detected. base 0x%lx size %zu",
			part, part->start, part->size);
		החזר false;
	}

	/* Check that this partition doesn't overlap any existing ones already
	 * in the domain
	 */
	עבור (i = 0; i < domain->num_partitions; i++) {
		بنية k_mem_partition *dpart = &domain->partitions[i];

		אם (dpart->size == 0U) {
			/* Unused slot */
			استمر;
		}

		dstart = dpart->start;
		dend = dstart + dpart->size;

		אם (pend > dstart && dend > pstart) {
			LOG_ERR("partition %p base %lx (size %zu) overlaps existing base %lx (size %zu)",
				part, part->start, part->size,
				dpart->start, dpart->size);
			החזר false;
		}
	}

	החזר true;
}

صحيح k_mem_domain_init(بنية k_mem_domain *domain, uint8_t num_parts,
		      بنية k_mem_partition *parts[])
{
	k_spinlock_key_t key;
	صحيح ret = 0;

	CHECKIF(domain == عدم) {
		ret = -EINVAL;
		اذهب out;
	}

	CHECKIF(!(num_parts == 0U || parts != عدم)) {
		LOG_ERR("parts array is NULL and num_parts is nonzero");
		ret = -EINVAL;
		اذهب out;
	}

	CHECKIF(!(num_parts <= max_partitions)) {
		LOG_ERR("num_parts of %d exceeds maximum allowable partitions (%d)",
			num_parts, max_partitions);
		ret = -EINVAL;
		اذهب out;
	}

	key = k_spin_lock(&z_mem_domain_lock);

	domain->num_partitions = 0U;
	(فراغ)memset(domain->partitions, 0, حجم(domain->partitions));

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_init(&domain->thread_mem_domain_list);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#ifdef CONFIG_ARCH_MEM_DOMAIN_DATA
	ret = arch_mem_domain_init(domain);

	אם (ret != 0) {
		LOG_ERR("architecture-specific initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		اذهب unlock_out;
	}
#endif /* CONFIG_ARCH_MEM_DOMAIN_DATA */
	אם (num_parts != 0U) {
		uint32_t i;

		עבור (i = 0U; i < num_parts; i++) {
			CHECKIF(!check_add_partition(domain, parts[i])) {
				LOG_ERR("invalid partition index %d (%p)",
					i, parts[i]);
				ret = -EINVAL;
				اذهب unlock_out;
			}

			domain->partitions[i] = *parts[i];
			domain->num_partitions++;
#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
			صحيح ret2 = arch_mem_domain_partition_add(domain, i);

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
	החזר ret;
}

صحيح k_mem_domain_deinit(بنية k_mem_domain *domain)
{
#אם defined(CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT)
	k_spinlock_key_t key;
	صحيح ret = 0;

	CHECKIF(domain == عدم) {
		ret = -EINVAL;
		اذهب out;
	}

	אם (domain == &k_mem_domain_default) {
		/* Default memory domain must be there forever. */
		ret = -EINVAL;
		اذهب out;
	}

	key = k_spin_lock(&z_mem_domain_lock);

	/* Must make sure there are no threads associated with this memory
	 * domain anymore. Or else these threads will run with an invalid
	 * memory domain.
	 */
	אם (!sys_dlist_is_empty(&domain->thread_mem_domain_list)) {
		ret = -EBUSY;
		اذهب unlock_out;
	}

	ret = arch_mem_domain_deinit(domain);
	אם (ret != 0) {
		LOG_ERR("architecture-specific de-initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		اذهب unlock_out;
	}

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, key);

out:
	החזר ret;
#אחרת  /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
	ARG_UNUSED(domain);
	החזר -ENOTSUP;
#endif /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
}

صحيح k_mem_domain_add_partition(بنية k_mem_domain *domain,
			       بنية k_mem_partition *part)
{
	صحيح p_idx;
	k_spinlock_key_t key;
	صحيح ret = 0;

	CHECKIF(domain == عدم) {
		ret = -EINVAL;
		اذهب out;
	}

	CHECKIF(!check_add_partition(domain, part)) {
		LOG_ERR("invalid partition %p", part);
		ret = -EINVAL;
		اذهب out;
	}

	key = k_spin_lock(&z_mem_domain_lock);

	עבור (p_idx = 0; p_idx < max_partitions; p_idx++) {
		/* A zero-sized partition denotes it's a free partition */
		אם (domain->partitions[p_idx].size == 0U) {
			اكسر;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no free partition slots available");
		ret = -ENOSPC;
		اذهب unlock_out;
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
	החזר ret;
}

صحيح k_mem_domain_remove_partition(بنية k_mem_domain *domain,
				  بنية k_mem_partition *part)
{
	صحيح p_idx;
	k_spinlock_key_t key;
	صحيح ret = 0;

	CHECKIF((domain == عدم) || (part == عدم)) {
		ret = -EINVAL;
		اذهب out;
	}

	key = k_spin_lock(&z_mem_domain_lock);

	/* find a partition that matches the given start and size */
	עבור (p_idx = 0; p_idx < max_partitions; p_idx++) {
		אם ((domain->partitions[p_idx].start == part->start) &&
		    (domain->partitions[p_idx].size == part->size)) {
			اكسر;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no matching partition found");
		ret = -ENOENT;
		اذهب unlock_out;
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
	החזר ret;
}

ساكن صحيح add_thread_locked(بنية k_mem_domain *domain,
			     k_tid_t thread)
{
	صحيح ret = 0;

	__ASSERT_NO_MSG(domain != عدم);
	__ASSERT_NO_MSG(thread != عدم);

	LOG_DBG("add thread %p to domain %p", thread, domain);

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_append(&domain->thread_mem_domain_list,
			 &thread->mem_domain_info.thread_mem_domain_node);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

	thread->mem_domain_info.mem_domain = domain;

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_add(thread);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	החזר ret;
}

ساكن صحيح remove_thread_locked(بنية k_thread *thread)
{
	صحيح ret = 0;

	__ASSERT_NO_MSG(thread != عدم);
	LOG_DBG("remove thread %p from memory domain %p",
		thread, thread->mem_domain_info.mem_domain);

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_remove(&thread->mem_domain_info.thread_mem_domain_node);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_remove(thread);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	החזר ret;
}

/* Called from thread object initialization */
فراغ z_mem_domain_init_thread(بنية k_thread *thread)
{
	صحيح ret;
	k_spinlock_key_t key = k_spin_lock(&z_mem_domain_lock);

	/* New threads inherit memory domain configuration from parent */
	ret = add_thread_locked(_current->mem_domain_info.mem_domain, thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_UNUSED(ret);

	k_spin_unlock(&z_mem_domain_lock, key);
}

/* Called when thread aborts during teardown tasks. The scheduler's spinlock is held */
فراغ z_mem_domain_exit_thread(بنية k_thread *thread)
{
	صحيح ret;

	k_spinlock_key_t key = k_spin_lock(&z_mem_domain_lock);

	ret = remove_thread_locked(thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_UNUSED(ret);

	k_spin_unlock(&z_mem_domain_lock, key);
}

صحيح k_mem_domain_add_thread(بنية k_mem_domain *domain, k_tid_t thread)
{
	صحيح ret = 0;
	k_spinlock_key_t key;

	key = k_spin_lock(&z_mem_domain_lock);
	אם (thread->mem_domain_info.mem_domain != domain) {
		ret = remove_thread_locked(thread);

		אם (ret == 0) {
			ret = add_thread_locked(domain, thread);
		}
	}
	k_spin_unlock(&z_mem_domain_lock, key);

	החזר ret;
}

ساكن فراغ init_mem_domain_module(فراغ)
{
	صحيح ret;

	ARG_UNUSED(ret);

	max_partitions = arch_mem_domain_max_partitions_get();
	/*
	 * max_partitions must be less than or equal to
	 * CONFIG_MAX_DOMAIN_PARTITIONS, or would encounter array index
	 * out of bounds error.
	 */
	__ASSERT(max_partitions <= CONFIG_MAX_DOMAIN_PARTITIONS, "");

	ret = k_mem_domain_init(&k_mem_domain_default, 0, عدم);
	__ASSERT(ret == 0, "failed to init default mem domain");

#ifdef Z_LIBC_PARTITION_EXISTS
	ret = k_mem_domain_add_partition(&k_mem_domain_default,
					 &z_libc_partition);
	__ASSERT(ret == 0, "failed to add default libc mem partition");
#endif /* Z_LIBC_PARTITION_EXISTS */
}

K_KERNEL_INIT_PRE(init_mem_domain_module);
