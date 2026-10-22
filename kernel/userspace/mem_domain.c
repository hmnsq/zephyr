/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <kernel_internal.h>
#अन्तर्भाव <zephyr/sys/__assert.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <zephyr/spinlock.h>
#अन्तर्भाव <zephyr/sys/check.h>
#अन्तर्भाव <zephyr/sys/libc-hooks.h>
#अन्तर्भाव <zephyr/logging/log.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

संरचना k_spinlock z_mem_domain_lock;
स्थिर uint8_t max_partitions;

संरचना k_mem_domain k_mem_domain_default;

स्थिर bool check_add_partition(संरचना k_mem_domain *domain,
				संरचना k_mem_partition *part)
{

	पूर्णाङ्क i;
	uintptr_t pstart, pend, dstart, dend;

	यदि (part == रिक्त) {
		LOG_ERR("NULL k_mem_partition provided");
		फर्काऊ false;
	}

#ifdef CONFIG_EXECUTE_XOR_WRITE
	/* Arches where execution cannot be disabled should always return
	 * false to this check
	 */
	यदि (K_MEM_PARTITION_IS_EXECUTABLE(part->attr) &&
	    K_MEM_PARTITION_IS_WRITABLE(part->attr)) {
		LOG_ERR("partition is writable and executable <start %lx>",
			part->start);
		फर्काऊ false;
	}
#endif /* CONFIG_EXECUTE_XOR_WRITE */

	यदि (part->size == 0U) {
		LOG_ERR("zero sized partition at %p with base 0x%lx",
			part, part->start);
		फर्काऊ false;
	}

	pstart = part->start;
	pend = part->start + part->size;

	यदि (pend <= pstart) {
		LOG_ERR("invalid partition %p, wraparound detected. base 0x%lx size %zu",
			part, part->start, part->size);
		फर्काऊ false;
	}

	/* Check that this partition doesn't overlap any existing ones already
	 * in the domain
	 */
	लागि (i = 0; i < domain->अङ्क_partitions; i++) {
		संरचना k_mem_partition *dpart = &domain->partitions[i];

		यदि (dpart->size == 0U) {
			/* Unused slot */
			जारी;
		}

		dstart = dpart->start;
		dend = dstart + dpart->size;

		यदि (pend > dstart && dend > pstart) {
			LOG_ERR("partition %p base %lx (size %zu) overlaps existing base %lx (size %zu)",
				part, part->start, part->size,
				dpart->start, dpart->size);
			फर्काऊ false;
		}
	}

	फर्काऊ true;
}

पूर्णाङ्क k_mem_domain_init(संरचना k_mem_domain *domain, uint8_t अङ्क_parts,
		      संरचना k_mem_partition *parts[])
{
	k_spinlock_साँचो_t साँचो;
	पूर्णाङ्क ret = 0;

	CHECKIF(domain == रिक्त) {
		ret = -EINVAL;
		गच्छ out;
	}

	CHECKIF(!(अङ्क_parts == 0U || parts != रिक्त)) {
		LOG_ERR("parts array is NULL and num_parts is nonzero");
		ret = -EINVAL;
		गच्छ out;
	}

	CHECKIF(!(अङ्क_parts <= max_partitions)) {
		LOG_ERR("num_parts of %d exceeds maximum allowable partitions (%d)",
			अङ्क_parts, max_partitions);
		ret = -EINVAL;
		गच्छ out;
	}

	साँचो = k_spin_lock(&z_mem_domain_lock);

	domain->अङ्क_partitions = 0U;
	(शून्य)स_रखो(domain->partitions, 0, परिमाण(domain->partitions));

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_init(&domain->thread_mem_domain_list);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#ifdef CONFIG_ARCH_MEM_DOMAIN_DATA
	ret = arch_mem_domain_init(domain);

	यदि (ret != 0) {
		LOG_ERR("architecture-specific initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		गच्छ unlock_out;
	}
#endif /* CONFIG_ARCH_MEM_DOMAIN_DATA */
	यदि (अङ्क_parts != 0U) {
		uint32_t i;

		लागि (i = 0U; i < अङ्क_parts; i++) {
			CHECKIF(!check_add_partition(domain, parts[i])) {
				LOG_ERR("invalid partition index %d (%p)",
					i, parts[i]);
				ret = -EINVAL;
				गच्छ unlock_out;
			}

			domain->partitions[i] = *parts[i];
			domain->अङ्क_partitions++;
#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
			पूर्णाङ्क ret2 = arch_mem_domain_partition_add(domain, i);

			ARG_UNUSED(ret2);
			CHECKIF(ret2 != 0) {
				ret = ret2;
			}
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */
		}
	}

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, साँचो);

out:
	फर्काऊ ret;
}

पूर्णाङ्क k_mem_domain_deinit(संरचना k_mem_domain *domain)
{
#यदि defined(CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT)
	k_spinlock_साँचो_t साँचो;
	पूर्णाङ्क ret = 0;

	CHECKIF(domain == रिक्त) {
		ret = -EINVAL;
		गच्छ out;
	}

	यदि (domain == &k_mem_domain_default) {
		/* Default memory domain must be there forever. */
		ret = -EINVAL;
		गच्छ out;
	}

	साँचो = k_spin_lock(&z_mem_domain_lock);

	/* Must make sure there are no threads associated with this memory
	 * domain anymore. Or else these threads will run with an invalid
	 * memory domain.
	 */
	यदि (!sys_dlist_is_empty(&domain->thread_mem_domain_list)) {
		ret = -EBUSY;
		गच्छ unlock_out;
	}

	ret = arch_mem_domain_deinit(domain);
	यदि (ret != 0) {
		LOG_ERR("architecture-specific de-initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		गच्छ unlock_out;
	}

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, साँचो);

out:
	फर्काऊ ret;
#अन्यथा  /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
	ARG_UNUSED(domain);
	फर्काऊ -ENOTSUP;
#endif /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
}

पूर्णाङ्क k_mem_domain_add_partition(संरचना k_mem_domain *domain,
			       संरचना k_mem_partition *part)
{
	पूर्णाङ्क p_idx;
	k_spinlock_साँचो_t साँचो;
	पूर्णाङ्क ret = 0;

	CHECKIF(domain == रिक्त) {
		ret = -EINVAL;
		गच्छ out;
	}

	CHECKIF(!check_add_partition(domain, part)) {
		LOG_ERR("invalid partition %p", part);
		ret = -EINVAL;
		गच्छ out;
	}

	साँचो = k_spin_lock(&z_mem_domain_lock);

	लागि (p_idx = 0; p_idx < max_partitions; p_idx++) {
		/* A zero-sized partition denotes it's a free partition */
		यदि (domain->partitions[p_idx].size == 0U) {
			रोक;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no free partition slots available");
		ret = -ENOSPC;
		गच्छ unlock_out;
	}

	LOG_DBG("add partition base %lx size %zu to domain %p",
		part->start, part->size, domain);

	domain->partitions[p_idx].start = part->start;
	domain->partitions[p_idx].size = part->size;
	domain->partitions[p_idx].attr = part->attr;

	domain->अङ्क_partitions++;

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_partition_add(domain, p_idx);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, साँचो);

out:
	फर्काऊ ret;
}

पूर्णाङ्क k_mem_domain_remove_partition(संरचना k_mem_domain *domain,
				  संरचना k_mem_partition *part)
{
	पूर्णाङ्क p_idx;
	k_spinlock_साँचो_t साँचो;
	पूर्णाङ्क ret = 0;

	CHECKIF((domain == रिक्त) || (part == रिक्त)) {
		ret = -EINVAL;
		गच्छ out;
	}

	साँचो = k_spin_lock(&z_mem_domain_lock);

	/* find a partition that matches the given start and size */
	लागि (p_idx = 0; p_idx < max_partitions; p_idx++) {
		यदि ((domain->partitions[p_idx].start == part->start) &&
		    (domain->partitions[p_idx].size == part->size)) {
			रोक;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no matching partition found");
		ret = -ENOENT;
		गच्छ unlock_out;
	}

	LOG_DBG("remove partition base %lx size %zu from domain %p",
		part->start, part->size, domain);

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_partition_remove(domain, p_idx);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	/* A zero-sized partition denotes it's a free partition */
	domain->partitions[p_idx].size = 0U;

	domain->अङ्क_partitions--;

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, साँचो);

out:
	फर्काऊ ret;
}

स्थिर पूर्णाङ्क add_thread_locked(संरचना k_mem_domain *domain,
			     k_tid_t thread)
{
	पूर्णाङ्क ret = 0;

	__ASSERT_NO_MSG(domain != रिक्त);
	__ASSERT_NO_MSG(thread != रिक्त);

	LOG_DBG("add thread %p to domain %p", thread, domain);

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_append(&domain->thread_mem_domain_list,
			 &thread->mem_domain_info.thread_mem_domain_गाँठो);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

	thread->mem_domain_info.mem_domain = domain;

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_add(thread);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	फर्काऊ ret;
}

स्थिर पूर्णाङ्क remove_thread_locked(संरचना k_thread *thread)
{
	पूर्णाङ्क ret = 0;

	__ASSERT_NO_MSG(thread != रिक्त);
	LOG_DBG("remove thread %p from memory domain %p",
		thread, thread->mem_domain_info.mem_domain);

#ifdef CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_remove(&thread->mem_domain_info.thread_mem_domain_गाँठो);
#endif /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#ifdef CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_remove(thread);
#endif /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	फर्काऊ ret;
}

/* Called from thread object initialization */
शून्य z_mem_domain_init_thread(संरचना k_thread *thread)
{
	पूर्णाङ्क ret;
	k_spinlock_साँचो_t साँचो = k_spin_lock(&z_mem_domain_lock);

	/* New threads inherit memory domain configuration from parent */
	ret = add_thread_locked(_current->mem_domain_info.mem_domain, thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_UNUSED(ret);

	k_spin_unlock(&z_mem_domain_lock, साँचो);
}

/* Called when thread aborts during teardown tasks. The scheduler's spinlock is held */
शून्य z_mem_domain_exit_thread(संरचना k_thread *thread)
{
	पूर्णाङ्क ret;

	k_spinlock_साँचो_t साँचो = k_spin_lock(&z_mem_domain_lock);

	ret = remove_thread_locked(thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_UNUSED(ret);

	k_spin_unlock(&z_mem_domain_lock, साँचो);
}

पूर्णाङ्क k_mem_domain_add_thread(संरचना k_mem_domain *domain, k_tid_t thread)
{
	पूर्णाङ्क ret = 0;
	k_spinlock_साँचो_t साँचो;

	साँचो = k_spin_lock(&z_mem_domain_lock);
	यदि (thread->mem_domain_info.mem_domain != domain) {
		ret = remove_thread_locked(thread);

		यदि (ret == 0) {
			ret = add_thread_locked(domain, thread);
		}
	}
	k_spin_unlock(&z_mem_domain_lock, साँचो);

	फर्काऊ ret;
}

स्थिर शून्य init_mem_domain_module(शून्य)
{
	पूर्णाङ्क ret;

	ARG_UNUSED(ret);

	max_partitions = arch_mem_domain_max_partitions_get();
	/*
	 * max_partitions must be less than or equal to
	 * CONFIG_MAX_DOMAIN_PARTITIONS, or would encounter array index
	 * out of bounds error.
	 */
	__ASSERT(max_partitions <= CONFIG_MAX_DOMAIN_PARTITIONS, "");

	ret = k_mem_domain_init(&k_mem_domain_default, 0, रिक्त);
	__ASSERT(ret == 0, "failed to init default mem domain");

#ifdef Z_LIBC_PARTITION_EXISTS
	ret = k_mem_domain_add_partition(&k_mem_domain_default,
					 &z_libc_partition);
	__ASSERT(ret == 0, "failed to add default libc mem partition");
#endif /* Z_LIBC_PARTITION_EXISTS */
}

K_KERNEL_INIT_PRE(init_mem_domain_module);
