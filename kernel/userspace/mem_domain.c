/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <kernel_internal.h>
#అన్తర్భావ <zephyr/sys/__assert.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <zephyr/spinlock.h>
#అన్తర్భావ <zephyr/sys/check.h>
#అన్తర్భావ <zephyr/sys/libc-hooks.h>
#అన్తర్భావ <zephyr/logging/log.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

నిర్మాణం k_spinlock z_mem_domain_తాళం;
స్థిర uint8_t max_partitions;

నిర్మాణం k_mem_domain k_mem_domain_default;

స్థిర bool check_కూడు_విభజన(నిర్మాణం k_mem_domain *domain,
				నిర్మాణం k_mem_విభజన *part)
{

	పూర్ణాంకం i;
	uintptr_t pstart, pend, dstart, dend;

	ఒకవేళ (part == ఖాళీ) {
		LOG_ERR("NULL k_mem_partition provided");
		తిరిగి false;
	}

#నిర్వచితమైతే CONFIG_EXECUTE_XOR_WRITE
	/* Arches where execution cannot be disabled should always return
	 * false to this check
	 */
	ఒకవేళ (K_MEM_విభజన_IS_EXECUTABLE(part->attr) &&
	    K_MEM_విభజన_IS_WRITABLE(part->attr)) {
		LOG_ERR("partition is writable and executable <start %lx>",
			part->ప్రారంభం);
		తిరిగి false;
	}
#అయితే_ముగింపు /* CONFIG_EXECUTE_XOR_WRITE */

	ఒకవేళ (part->కొలత == 0U) {
		LOG_ERR("zero sized partition at %p with base 0x%lx",
			part, part->ప్రారంభం);
		తిరిగి false;
	}

	pstart = part->ప్రారంభం;
	pend = part->ప్రారంభం + part->కొలత;

	ఒకవేళ (pend <= pstart) {
		LOG_ERR("invalid partition %p, wraparound detected. base 0x%lx size %zu",
			part, part->ప్రారంభం, part->కొలత);
		తిరిగి false;
	}

	/* Check that this partition doesn't overlap any existing ones already
	 * in the domain
	 */
	కొరకు (i = 0; i < domain->అంకె_partitions; i++) {
		నిర్మాణం k_mem_విభజన *dpart = &domain->partitions[i];

		ఒకవేళ (dpart->కొలత == 0U) {
			/* Unused slot */
			కొనసాగు;
		}

		dstart = dpart->ప్రారంభం;
		dend = dstart + dpart->కొలత;

		ఒకవేళ (pend > dstart && dend > pstart) {
			LOG_ERR("partition %p base %lx (size %zu) overlaps existing base %lx (size %zu)",
				part, part->ప్రారంభం, part->కొలత,
				dpart->ప్రారంభం, dpart->కొలత);
			తిరిగి false;
		}
	}

	తిరిగి true;
}

పూర్ణాంకం k_mem_domain_init(నిర్మాణం k_mem_domain *domain, uint8_t అంకె_parts,
		      నిర్మాణం k_mem_విభజన *parts[])
{
	k_spinlock_కీలకం_t కీలకం;
	పూర్ణాంకం ret = 0;

	CHECKIF(domain == ఖాళీ) {
		ret = -EINVAL;
		వెళ్ళు బహిర్గతం;
	}

	CHECKIF(!(అంకె_parts == 0U || parts != ఖాళీ)) {
		LOG_ERR("parts array is NULL and num_parts is nonzero");
		ret = -EINVAL;
		వెళ్ళు బహిర్గతం;
	}

	CHECKIF(!(అంకె_parts <= max_partitions)) {
		LOG_ERR("num_parts of %d exceeds maximum allowable partitions (%d)",
			అంకె_parts, max_partitions);
		ret = -EINVAL;
		వెళ్ళు బహిర్గతం;
	}

	కీలకం = k_spin_తాళం(&z_mem_domain_తాళం);

	domain->అంకె_partitions = 0U;
	(శూన్య)memset(domain->partitions, 0, పరిమాణం(domain->partitions));

#నిర్వచితమైతే CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_init(&domain->thread_mem_domain_list);
#అయితే_ముగింపు /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#నిర్వచితమైతే CONFIG_ARCH_MEM_DOMAIN_దత్తాంశం
	ret = arch_mem_domain_init(domain);

	ఒకవేళ (ret != 0) {
		LOG_ERR("architecture-specific initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		వెళ్ళు unlock_బహిర్గతం;
	}
#అయితే_ముగింపు /* CONFIG_ARCH_MEM_DOMAIN_DATA */
	ఒకవేళ (అంకె_parts != 0U) {
		uint32_t i;

		కొరకు (i = 0U; i < అంకె_parts; i++) {
			CHECKIF(!check_కూడు_విభజన(domain, parts[i])) {
				LOG_ERR("invalid partition index %d (%p)",
					i, parts[i]);
				ret = -EINVAL;
				వెళ్ళు unlock_బహిర్గతం;
			}

			domain->partitions[i] = *parts[i];
			domain->అంకె_partitions++;
#నిర్వచితమైతే CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
			పూర్ణాంకం ret2 = arch_mem_domain_విభజన_కూడు(domain, i);

			ARG_వాడని(ret2);
			CHECKIF(ret2 != 0) {
				ret = ret2;
			}
#అయితే_ముగింపు /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */
		}
	}

unlock_బహిర్గతం:
	k_spin_unlock(&z_mem_domain_తాళం, కీలకం);

బహిర్గతం:
	తిరిగి ret;
}

పూర్ణాంకం k_mem_domain_deinit(నిర్మాణం k_mem_domain *domain)
{
#ఒకవేళ defined(CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT)
	k_spinlock_కీలకం_t కీలకం;
	పూర్ణాంకం ret = 0;

	CHECKIF(domain == ఖాళీ) {
		ret = -EINVAL;
		వెళ్ళు బహిర్గతం;
	}

	ఒకవేళ (domain == &k_mem_domain_default) {
		/* Default memory domain must be there forever. */
		ret = -EINVAL;
		వెళ్ళు బహిర్గతం;
	}

	కీలకం = k_spin_తాళం(&z_mem_domain_తాళం);

	/* Must make sure there are no threads associated with this memory
	 * domain anymore. Or else these threads will run with an invalid
	 * memory domain.
	 */
	ఒకవేళ (!sys_dlist_is_ఖాళీలు(&domain->thread_mem_domain_list)) {
		ret = -EBUSY;
		వెళ్ళు unlock_బహిర్గతం;
	}

	ret = arch_mem_domain_deinit(domain);
	ఒకవేళ (ret != 0) {
		LOG_ERR("architecture-specific de-initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		వెళ్ళు unlock_బహిర్గతం;
	}

unlock_బహిర్గతం:
	k_spin_unlock(&z_mem_domain_తాళం, కీలకం);

బహిర్గతం:
	తిరిగి ret;
#లేకపోతే  /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
	ARG_వాడని(domain);
	తిరిగి -ENOTSUP;
#అయితే_ముగింపు /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
}

పూర్ణాంకం k_mem_domain_కూడు_విభజన(నిర్మాణం k_mem_domain *domain,
			       నిర్మాణం k_mem_విభజన *part)
{
	పూర్ణాంకం p_idx;
	k_spinlock_కీలకం_t కీలకం;
	పూర్ణాంకం ret = 0;

	CHECKIF(domain == ఖాళీ) {
		ret = -EINVAL;
		వెళ్ళు బహిర్గతం;
	}

	CHECKIF(!check_కూడు_విభజన(domain, part)) {
		LOG_ERR("invalid partition %p", part);
		ret = -EINVAL;
		వెళ్ళు బహిర్గతం;
	}

	కీలకం = k_spin_తాళం(&z_mem_domain_తాళం);

	కొరకు (p_idx = 0; p_idx < max_partitions; p_idx++) {
		/* A zero-sized partition denotes it's a free partition */
		ఒకవేళ (domain->partitions[p_idx].కొలత == 0U) {
			ఆపు;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no free partition slots available");
		ret = -ENOSPC;
		వెళ్ళు unlock_బహిర్గతం;
	}

	LOG_DBG("add partition base %lx size %zu to domain %p",
		part->ప్రారంభం, part->కొలత, domain);

	domain->partitions[p_idx].ప్రారంభం = part->ప్రారంభం;
	domain->partitions[p_idx].కొలత = part->కొలత;
	domain->partitions[p_idx].attr = part->attr;

	domain->అంకె_partitions++;

#నిర్వచితమైతే CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_విభజన_కూడు(domain, p_idx);
#అయితే_ముగింపు /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

unlock_బహిర్గతం:
	k_spin_unlock(&z_mem_domain_తాళం, కీలకం);

బహిర్గతం:
	తిరిగి ret;
}

పూర్ణాంకం k_mem_domain_remove_విభజన(నిర్మాణం k_mem_domain *domain,
				  నిర్మాణం k_mem_విభజన *part)
{
	పూర్ణాంకం p_idx;
	k_spinlock_కీలకం_t కీలకం;
	పూర్ణాంకం ret = 0;

	CHECKIF((domain == ఖాళీ) || (part == ఖాళీ)) {
		ret = -EINVAL;
		వెళ్ళు బహిర్గతం;
	}

	కీలకం = k_spin_తాళం(&z_mem_domain_తాళం);

	/* find a partition that matches the given start and size */
	కొరకు (p_idx = 0; p_idx < max_partitions; p_idx++) {
		ఒకవేళ ((domain->partitions[p_idx].ప్రారంభం == part->ప్రారంభం) &&
		    (domain->partitions[p_idx].కొలత == part->కొలత)) {
			ఆపు;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no matching partition found");
		ret = -ENOENT;
		వెళ్ళు unlock_బహిర్గతం;
	}

	LOG_DBG("remove partition base %lx size %zu from domain %p",
		part->ప్రారంభం, part->కొలత, domain);

#నిర్వచితమైతే CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_విభజన_remove(domain, p_idx);
#అయితే_ముగింపు /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	/* A zero-sized partition denotes it's a free partition */
	domain->partitions[p_idx].కొలత = 0U;

	domain->అంకె_partitions--;

unlock_బహిర్గతం:
	k_spin_unlock(&z_mem_domain_తాళం, కీలకం);

బహిర్గతం:
	తిరిగి ret;
}

స్థిర పూర్ణాంకం కూడు_thread_locked(నిర్మాణం k_mem_domain *domain,
			     k_tid_t thread)
{
	పూర్ణాంకం ret = 0;

	__ASSERT_NO_MSG(domain != ఖాళీ);
	__ASSERT_NO_MSG(thread != ఖాళీ);

	LOG_DBG("add thread %p to domain %p", thread, domain);

#నిర్వచితమైతే CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_append(&domain->thread_mem_domain_list,
			 &thread->mem_domain_info.thread_mem_domain_కణుపు);
#అయితే_ముగింపు /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

	thread->mem_domain_info.mem_domain = domain;

#నిర్వచితమైతే CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_కూడు(thread);
#అయితే_ముగింపు /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	తిరిగి ret;
}

స్థిర పూర్ణాంకం remove_thread_locked(నిర్మాణం k_thread *thread)
{
	పూర్ణాంకం ret = 0;

	__ASSERT_NO_MSG(thread != ఖాళీ);
	LOG_DBG("remove thread %p from memory domain %p",
		thread, thread->mem_domain_info.mem_domain);

#నిర్వచితమైతే CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_remove(&thread->mem_domain_info.thread_mem_domain_కణుపు);
#అయితే_ముగింపు /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#నిర్వచితమైతే CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_remove(thread);
#అయితే_ముగింపు /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	తిరిగి ret;
}

/* Called from thread object initialization */
శూన్య z_mem_domain_init_thread(నిర్మాణం k_thread *thread)
{
	పూర్ణాంకం ret;
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&z_mem_domain_తాళం);

	/* New threads inherit memory domain configuration from parent */
	ret = కూడు_thread_locked(_current->mem_domain_info.mem_domain, thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_వాడని(ret);

	k_spin_unlock(&z_mem_domain_తాళం, కీలకం);
}

/* Called when thread aborts during teardown tasks. The scheduler's spinlock is held */
శూన్య z_mem_domain_exit_thread(నిర్మాణం k_thread *thread)
{
	పూర్ణాంకం ret;

	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&z_mem_domain_తాళం);

	ret = remove_thread_locked(thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_వాడని(ret);

	k_spin_unlock(&z_mem_domain_తాళం, కీలకం);
}

పూర్ణాంకం k_mem_domain_కూడు_thread(నిర్మాణం k_mem_domain *domain, k_tid_t thread)
{
	పూర్ణాంకం ret = 0;
	k_spinlock_కీలకం_t కీలకం;

	కీలకం = k_spin_తాళం(&z_mem_domain_తాళం);
	ఒకవేళ (thread->mem_domain_info.mem_domain != domain) {
		ret = remove_thread_locked(thread);

		ఒకవేళ (ret == 0) {
			ret = కూడు_thread_locked(domain, thread);
		}
	}
	k_spin_unlock(&z_mem_domain_తాళం, కీలకం);

	తిరిగి ret;
}

స్థిర శూన్య init_mem_domain_module(శూన్య)
{
	పూర్ణాంకం ret;

	ARG_వాడని(ret);

	max_partitions = arch_mem_domain_max_partitions_get();
	/*
	 * max_partitions must be less than or equal to
	 * CONFIG_MAX_DOMAIN_PARTITIONS, or would encounter array index
	 * out of bounds error.
	 */
	__ASSERT(max_partitions <= CONFIG_MAX_DOMAIN_PARTITIONS, "");

	ret = k_mem_domain_init(&k_mem_domain_default, 0, ఖాళీ);
	__ASSERT(ret == 0, "failed to init default mem domain");

#నిర్వచితమైతే Z_LIBC_విభజన_EXISTS
	ret = k_mem_domain_కూడు_విభజన(&k_mem_domain_default,
					 &z_libc_విభజన);
	__ASSERT(ret == 0, "failed to add default libc mem partition");
#అయితే_ముగింపు /* Z_LIBC_PARTITION_EXISTS */
}

K_KERNEL_INIT_PRE(init_mem_domain_module);
