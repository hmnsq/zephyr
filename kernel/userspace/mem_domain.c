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

संरचना k_spinlock z_mem_domain_तालकम्;
स्थिर uint8_t max_partitions;

संरचना k_mem_domain k_mem_domain_default;

स्थिर bool check_सङ्कलय_partition(संरचना k_mem_domain *domain,
				संरचना k_mem_partition *part)
{

	पूर्णाङ्क i;
	uintptr_t pstart, pend, dstart, dend;

	यदि (part == रिक्त) {
		LOG_ERR("NULL k_mem_partition provided");
		प्रत्यागम false;
	}

#यदि_परिभाषितम् CONFIG_EXECUTE_XOR_WRITE
	/* Arches where execution cannot be disabled should always return
	 * false to this check
	 */
	यदि (K_MEM_PARTITION_IS_EXECUTABLE(part->attr) &&
	    K_MEM_PARTITION_IS_WRITABLE(part->attr)) {
		LOG_ERR("partition is writable and executable <start %lx>",
			part->start);
		प्रत्यागम false;
	}
#यदि_अन्तः /* CONFIG_EXECUTE_XOR_WRITE */

	यदि (part->आकार == 0U) {
		LOG_ERR("zero sized partition at %p with base 0x%lx",
			part, part->start);
		प्रत्यागम false;
	}

	pstart = part->start;
	pend = part->start + part->आकार;

	यदि (pend <= pstart) {
		LOG_ERR("invalid partition %p, wraparound detected. base 0x%lx size %zu",
			part, part->start, part->आकार);
		प्रत्यागम false;
	}

	/* Check that this partition doesn't overlap any existing ones already
	 * in the domain
	 */
	प्रति (i = 0; i < domain->अङ्कः_partitions; i++) {
		संरचना k_mem_partition *dpart = &domain->partitions[i];

		यदि (dpart->आकार == 0U) {
			/* Unused slot */
			अनुवर्त;
		}

		dstart = dpart->start;
		dend = dstart + dpart->आकार;

		यदि (pend > dstart && dend > pstart) {
			LOG_ERR("partition %p base %lx (size %zu) overlaps existing base %lx (size %zu)",
				part, part->start, part->आकार,
				dpart->start, dpart->आकार);
			प्रत्यागम false;
		}
	}

	प्रत्यागम true;
}

पूर्णाङ्क k_mem_domain_init(संरचना k_mem_domain *domain, uint8_t अङ्कः_parts,
		      संरचना k_mem_partition *parts[])
{
	k_spinlock_कुञ्चिका_t कुञ्चिका;
	पूर्णाङ्क ret = 0;

	CHECKIF(domain == रिक्त) {
		ret = -EINVAL;
		गच्छ बहिर्गमनम्;
	}

	CHECKIF(!(अङ्कः_parts == 0U || parts != रिक्त)) {
		LOG_ERR("parts array is NULL and num_parts is nonzero");
		ret = -EINVAL;
		गच्छ बहिर्गमनम्;
	}

	CHECKIF(!(अङ्कः_parts <= max_partitions)) {
		LOG_ERR("num_parts of %d exceeds maximum allowable partitions (%d)",
			अङ्कः_parts, max_partitions);
		ret = -EINVAL;
		गच्छ बहिर्गमनम्;
	}

	कुञ्चिका = k_spin_तालकम्(&z_mem_domain_तालकम्);

	domain->अङ्कः_partitions = 0U;
	(शून्य)स्मृतिं_पूरय(domain->partitions, 0, परिमाण(domain->partitions));

#यदि_परिभाषितम् CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_init(&domain->thread_mem_domain_list);
#यदि_अन्तः /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#यदि_परिभाषितम् CONFIG_ARCH_MEM_DOMAIN_दत्त
	ret = arch_mem_domain_init(domain);

	यदि (ret != 0) {
		LOG_ERR("architecture-specific initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		गच्छ unlock_बहिर्गमनम्;
	}
#यदि_अन्तः /* CONFIG_ARCH_MEM_DOMAIN_DATA */
	यदि (अङ्कः_parts != 0U) {
		uint32_t i;

		प्रति (i = 0U; i < अङ्कः_parts; i++) {
			CHECKIF(!check_सङ्कलय_partition(domain, parts[i])) {
				LOG_ERR("invalid partition index %d (%p)",
					i, parts[i]);
				ret = -EINVAL;
				गच्छ unlock_बहिर्गमनम्;
			}

			domain->partitions[i] = *parts[i];
			domain->अङ्कः_partitions++;
#यदि_परिभाषितम् CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
			पूर्णाङ्क ret2 = arch_mem_domain_partition_सङ्कलय(domain, i);

			ARG_अप्रयुक्तम्(ret2);
			CHECKIF(ret2 != 0) {
				ret = ret2;
			}
#यदि_अन्तः /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */
		}
	}

unlock_बहिर्गमनम्:
	k_spin_unlock(&z_mem_domain_तालकम्, कुञ्चिका);

बहिर्गमनम्:
	प्रत्यागम ret;
}

पूर्णाङ्क k_mem_domain_deinit(संरचना k_mem_domain *domain)
{
#यदि defined(CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT)
	k_spinlock_कुञ्चिका_t कुञ्चिका;
	पूर्णाङ्क ret = 0;

	CHECKIF(domain == रिक्त) {
		ret = -EINVAL;
		गच्छ बहिर्गमनम्;
	}

	यदि (domain == &k_mem_domain_default) {
		/* Default memory domain must be there forever. */
		ret = -EINVAL;
		गच्छ बहिर्गमनम्;
	}

	कुञ्चिका = k_spin_तालकम्(&z_mem_domain_तालकम्);

	/* Must make sure there are no threads associated with this memory
	 * domain anymore. Or else these threads will run with an invalid
	 * memory domain.
	 */
	यदि (!sys_dlist_is_रिक्तानि(&domain->thread_mem_domain_list)) {
		ret = -EBUSY;
		गच्छ unlock_बहिर्गमनम्;
	}

	ret = arch_mem_domain_deinit(domain);
	यदि (ret != 0) {
		LOG_ERR("architecture-specific de-initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		गच्छ unlock_बहिर्गमनम्;
	}

unlock_बहिर्गमनम्:
	k_spin_unlock(&z_mem_domain_तालकम्, कुञ्चिका);

बहिर्गमनम्:
	प्रत्यागम ret;
#अन्यथा  /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
	ARG_अप्रयुक्तम्(domain);
	प्रत्यागम -ENOTSUP;
#यदि_अन्तः /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
}

पूर्णाङ्क k_mem_domain_सङ्कलय_partition(संरचना k_mem_domain *domain,
			       संरचना k_mem_partition *part)
{
	पूर्णाङ्क p_idx;
	k_spinlock_कुञ्चिका_t कुञ्चिका;
	पूर्णाङ्क ret = 0;

	CHECKIF(domain == रिक्त) {
		ret = -EINVAL;
		गच्छ बहिर्गमनम्;
	}

	CHECKIF(!check_सङ्कलय_partition(domain, part)) {
		LOG_ERR("invalid partition %p", part);
		ret = -EINVAL;
		गच्छ बहिर्गमनम्;
	}

	कुञ्चिका = k_spin_तालकम्(&z_mem_domain_तालकम्);

	प्रति (p_idx = 0; p_idx < max_partitions; p_idx++) {
		/* A zero-sized partition denotes it's a free partition */
		यदि (domain->partitions[p_idx].आकार == 0U) {
			भङ्ग;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no free partition slots available");
		ret = -ENOSPC;
		गच्छ unlock_बहिर्गमनम्;
	}

	LOG_DBG("add partition base %lx size %zu to domain %p",
		part->start, part->आकार, domain);

	domain->partitions[p_idx].start = part->start;
	domain->partitions[p_idx].आकार = part->आकार;
	domain->partitions[p_idx].attr = part->attr;

	domain->अङ्कः_partitions++;

#यदि_परिभाषितम् CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_partition_सङ्कलय(domain, p_idx);
#यदि_अन्तः /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

unlock_बहिर्गमनम्:
	k_spin_unlock(&z_mem_domain_तालकम्, कुञ्चिका);

बहिर्गमनम्:
	प्रत्यागम ret;
}

पूर्णाङ्क k_mem_domain_remove_partition(संरचना k_mem_domain *domain,
				  संरचना k_mem_partition *part)
{
	पूर्णाङ्क p_idx;
	k_spinlock_कुञ्चिका_t कुञ्चिका;
	पूर्णाङ्क ret = 0;

	CHECKIF((domain == रिक्त) || (part == रिक्त)) {
		ret = -EINVAL;
		गच्छ बहिर्गमनम्;
	}

	कुञ्चिका = k_spin_तालकम्(&z_mem_domain_तालकम्);

	/* find a partition that matches the given start and size */
	प्रति (p_idx = 0; p_idx < max_partitions; p_idx++) {
		यदि ((domain->partitions[p_idx].start == part->start) &&
		    (domain->partitions[p_idx].आकार == part->आकार)) {
			भङ्ग;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no matching partition found");
		ret = -ENOENT;
		गच्छ unlock_बहिर्गमनम्;
	}

	LOG_DBG("remove partition base %lx size %zu from domain %p",
		part->start, part->आकार, domain);

#यदि_परिभाषितम् CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_partition_remove(domain, p_idx);
#यदि_अन्तः /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	/* A zero-sized partition denotes it's a free partition */
	domain->partitions[p_idx].आकार = 0U;

	domain->अङ्कः_partitions--;

unlock_बहिर्गमनम्:
	k_spin_unlock(&z_mem_domain_तालकम्, कुञ्चिका);

बहिर्गमनम्:
	प्रत्यागम ret;
}

स्थिर पूर्णाङ्क सङ्कलय_thread_locked(संरचना k_mem_domain *domain,
			     k_tid_t thread)
{
	पूर्णाङ्क ret = 0;

	__ASSERT_NO_MSG(domain != रिक्त);
	__ASSERT_NO_MSG(thread != रिक्त);

	LOG_DBG("add thread %p to domain %p", thread, domain);

#यदि_परिभाषितम् CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_append(&domain->thread_mem_domain_list,
			 &thread->mem_domain_info.thread_mem_domain_ग्रन्थिः);
#यदि_अन्तः /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

	thread->mem_domain_info.mem_domain = domain;

#यदि_परिभाषितम् CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_सङ्कलय(thread);
#यदि_अन्तः /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	प्रत्यागम ret;
}

स्थिर पूर्णाङ्क remove_thread_locked(संरचना k_thread *thread)
{
	पूर्णाङ्क ret = 0;

	__ASSERT_NO_MSG(thread != रिक्त);
	LOG_DBG("remove thread %p from memory domain %p",
		thread, thread->mem_domain_info.mem_domain);

#यदि_परिभाषितम् CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_remove(&thread->mem_domain_info.thread_mem_domain_ग्रन्थिः);
#यदि_अन्तः /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#यदि_परिभाषितम् CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_remove(thread);
#यदि_अन्तः /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	प्रत्यागम ret;
}

/* Called from thread object initialization */
शून्य z_mem_domain_init_thread(संरचना k_thread *thread)
{
	पूर्णाङ्क ret;
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&z_mem_domain_तालकम्);

	/* New threads inherit memory domain configuration from parent */
	ret = सङ्कलय_thread_locked(_current->mem_domain_info.mem_domain, thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_अप्रयुक्तम्(ret);

	k_spin_unlock(&z_mem_domain_तालकम्, कुञ्चिका);
}

/* Called when thread aborts during teardown tasks. The scheduler's spinlock is held */
शून्य z_mem_domain_exit_thread(संरचना k_thread *thread)
{
	पूर्णाङ्क ret;

	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&z_mem_domain_तालकम्);

	ret = remove_thread_locked(thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_अप्रयुक्तम्(ret);

	k_spin_unlock(&z_mem_domain_तालकम्, कुञ्चिका);
}

पूर्णाङ्क k_mem_domain_सङ्कलय_thread(संरचना k_mem_domain *domain, k_tid_t thread)
{
	पूर्णाङ्क ret = 0;
	k_spinlock_कुञ्चिका_t कुञ्चिका;

	कुञ्चिका = k_spin_तालकम्(&z_mem_domain_तालकम्);
	यदि (thread->mem_domain_info.mem_domain != domain) {
		ret = remove_thread_locked(thread);

		यदि (ret == 0) {
			ret = सङ्कलय_thread_locked(domain, thread);
		}
	}
	k_spin_unlock(&z_mem_domain_तालकम्, कुञ्चिका);

	प्रत्यागम ret;
}

स्थिर शून्य init_mem_domain_module(शून्य)
{
	पूर्णाङ्क ret;

	ARG_अप्रयुक्तम्(ret);

	max_partitions = arch_mem_domain_max_partitions_get();
	/*
	 * max_partitions must be less than or equal to
	 * CONFIG_MAX_DOMAIN_PARTITIONS, or would encounter array index
	 * out of bounds error.
	 */
	__ASSERT(max_partitions <= CONFIG_MAX_DOMAIN_PARTITIONS, "");

	ret = k_mem_domain_init(&k_mem_domain_default, 0, रिक्त);
	__ASSERT(ret == 0, "failed to init default mem domain");

#यदि_परिभाषितम् Z_LIBC_PARTITION_EXISTS
	ret = k_mem_domain_सङ्कलय_partition(&k_mem_domain_default,
					 &z_libc_partition);
	__ASSERT(ret == 0, "failed to add default libc mem partition");
#यदि_अन्तः /* Z_LIBC_PARTITION_EXISTS */
}

K_KERNEL_INIT_PRE(init_mem_domain_module);
