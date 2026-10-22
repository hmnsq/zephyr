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

بنية k_spinlock z_mem_domain_قلف;
ساكن uint8_t max_partitions;

بنية k_mem_domain k_mem_domain_default;

ساكن bool check_جمع_کړه_برخه_کول(بنية k_mem_domain *domain,
				بنية k_mem_برخه_کول *part)
{

	صحيح i;
	uintptr_t pstart, pend, dstart, dend;

	إذا (part == عدم) {
		LOG_ERR("NULL k_mem_partition provided");
		أرجع false;
	}

#که_تعریف_شوی CONFIG_EXECUTE_XOR_WRITE
	/* Arches where execution cannot be disabled should always return
	 * false to this check
	 */
	إذا (K_MEM_برخه_کول_IS_EXECUTABLE(part->attr) &&
	    K_MEM_برخه_کول_IS_WRITABLE(part->attr)) {
		LOG_ERR("partition is writable and executable <start %lx>",
			part->پیل);
		أرجع false;
	}
#د_که_پای /* CONFIG_EXECUTE_XOR_WRITE */

	إذا (part->اندازه == 0U) {
		LOG_ERR("zero sized partition at %p with base 0x%lx",
			part, part->پیل);
		أرجع false;
	}

	pstart = part->پیل;
	pend = part->پیل + part->اندازه;

	إذا (pend <= pstart) {
		LOG_ERR("invalid partition %p, wraparound detected. base 0x%lx size %zu",
			part, part->پیل, part->اندازه);
		أرجع false;
	}

	/* Check that this partition doesn't overlap any existing ones already
	 * in the domain
	 */
	لكل (i = 0; i < domain->عدد_partitions; i++) {
		بنية k_mem_برخه_کول *dpart = &domain->partitions[i];

		إذا (dpart->اندازه == 0U) {
			/* Unused slot */
			استمر;
		}

		dstart = dpart->پیل;
		dend = dstart + dpart->اندازه;

		إذا (pend > dstart && dend > pstart) {
			LOG_ERR("partition %p base %lx (size %zu) overlaps existing base %lx (size %zu)",
				part, part->پیل, part->اندازه,
				dpart->پیل, dpart->اندازه);
			أرجع false;
		}
	}

	أرجع true;
}

صحيح k_mem_domain_init(بنية k_mem_domain *domain, uint8_t عدد_parts,
		      بنية k_mem_برخه_کول *parts[])
{
	k_spinlock_کلی_t کلی;
	صحيح ret = 0;

	CHECKIF(domain == عدم) {
		ret = -EINVAL;
		اذهب بهر;
	}

	CHECKIF(!(عدد_parts == 0U || parts != عدم)) {
		LOG_ERR("parts array is NULL and num_parts is nonzero");
		ret = -EINVAL;
		اذهب بهر;
	}

	CHECKIF(!(عدد_parts <= max_partitions)) {
		LOG_ERR("num_parts of %d exceeds maximum allowable partitions (%d)",
			عدد_parts, max_partitions);
		ret = -EINVAL;
		اذهب بهر;
	}

	کلی = k_spin_قلف(&z_mem_domain_قلف);

	domain->عدد_partitions = 0U;
	(فراغ)memset(domain->partitions, 0, حجم(domain->partitions));

#که_تعریف_شوی CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_init(&domain->thread_mem_domain_list);
#د_که_پای /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#که_تعریف_شوی CONFIG_ARCH_MEM_DOMAIN_معلومات
	ret = arch_mem_domain_init(domain);

	إذا (ret != 0) {
		LOG_ERR("architecture-specific initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		اذهب unlock_بهر;
	}
#د_که_پای /* CONFIG_ARCH_MEM_DOMAIN_DATA */
	إذا (عدد_parts != 0U) {
		uint32_t i;

		لكل (i = 0U; i < عدد_parts; i++) {
			CHECKIF(!check_جمع_کړه_برخه_کول(domain, parts[i])) {
				LOG_ERR("invalid partition index %d (%p)",
					i, parts[i]);
				ret = -EINVAL;
				اذهب unlock_بهر;
			}

			domain->partitions[i] = *parts[i];
			domain->عدد_partitions++;
#که_تعریف_شوی CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
			صحيح ret2 = arch_mem_domain_برخه_کول_جمع_کړه(domain, i);

			ARG_ناکارول_شوی(ret2);
			CHECKIF(ret2 != 0) {
				ret = ret2;
			}
#د_که_پای /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */
		}
	}

unlock_بهر:
	k_spin_unlock(&z_mem_domain_قلف, کلی);

بهر:
	أرجع ret;
}

صحيح k_mem_domain_deinit(بنية k_mem_domain *domain)
{
#إذا defined(CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT)
	k_spinlock_کلی_t کلی;
	صحيح ret = 0;

	CHECKIF(domain == عدم) {
		ret = -EINVAL;
		اذهب بهر;
	}

	إذا (domain == &k_mem_domain_default) {
		/* Default memory domain must be there forever. */
		ret = -EINVAL;
		اذهب بهر;
	}

	کلی = k_spin_قلف(&z_mem_domain_قلف);

	/* Must make sure there are no threads associated with this memory
	 * domain anymore. Or else these threads will run with an invalid
	 * memory domain.
	 */
	إذا (!sys_dlist_is_تشه(&domain->thread_mem_domain_list)) {
		ret = -EBUSY;
		اذهب unlock_بهر;
	}

	ret = arch_mem_domain_deinit(domain);
	إذا (ret != 0) {
		LOG_ERR("architecture-specific de-initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		اذهب unlock_بهر;
	}

unlock_بهر:
	k_spin_unlock(&z_mem_domain_قلف, کلی);

بهر:
	أرجع ret;
#وإلا  /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
	ARG_ناکارول_شوی(domain);
	أرجع -ENOTSUP;
#د_که_پای /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
}

صحيح k_mem_domain_جمع_کړه_برخه_کول(بنية k_mem_domain *domain,
			       بنية k_mem_برخه_کول *part)
{
	صحيح p_idx;
	k_spinlock_کلی_t کلی;
	صحيح ret = 0;

	CHECKIF(domain == عدم) {
		ret = -EINVAL;
		اذهب بهر;
	}

	CHECKIF(!check_جمع_کړه_برخه_کول(domain, part)) {
		LOG_ERR("invalid partition %p", part);
		ret = -EINVAL;
		اذهب بهر;
	}

	کلی = k_spin_قلف(&z_mem_domain_قلف);

	لكل (p_idx = 0; p_idx < max_partitions; p_idx++) {
		/* A zero-sized partition denotes it's a free partition */
		إذا (domain->partitions[p_idx].اندازه == 0U) {
			اكسر;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no free partition slots available");
		ret = -ENOSPC;
		اذهب unlock_بهر;
	}

	LOG_DBG("add partition base %lx size %zu to domain %p",
		part->پیل, part->اندازه, domain);

	domain->partitions[p_idx].پیل = part->پیل;
	domain->partitions[p_idx].اندازه = part->اندازه;
	domain->partitions[p_idx].attr = part->attr;

	domain->عدد_partitions++;

#که_تعریف_شوی CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_برخه_کول_جمع_کړه(domain, p_idx);
#د_که_پای /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

unlock_بهر:
	k_spin_unlock(&z_mem_domain_قلف, کلی);

بهر:
	أرجع ret;
}

صحيح k_mem_domain_remove_برخه_کول(بنية k_mem_domain *domain,
				  بنية k_mem_برخه_کول *part)
{
	صحيح p_idx;
	k_spinlock_کلی_t کلی;
	صحيح ret = 0;

	CHECKIF((domain == عدم) || (part == عدم)) {
		ret = -EINVAL;
		اذهب بهر;
	}

	کلی = k_spin_قلف(&z_mem_domain_قلف);

	/* find a partition that matches the given start and size */
	لكل (p_idx = 0; p_idx < max_partitions; p_idx++) {
		إذا ((domain->partitions[p_idx].پیل == part->پیل) &&
		    (domain->partitions[p_idx].اندازه == part->اندازه)) {
			اكسر;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no matching partition found");
		ret = -ENOENT;
		اذهب unlock_بهر;
	}

	LOG_DBG("remove partition base %lx size %zu from domain %p",
		part->پیل, part->اندازه, domain);

#که_تعریف_شوی CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_برخه_کول_remove(domain, p_idx);
#د_که_پای /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	/* A zero-sized partition denotes it's a free partition */
	domain->partitions[p_idx].اندازه = 0U;

	domain->عدد_partitions--;

unlock_بهر:
	k_spin_unlock(&z_mem_domain_قلف, کلی);

بهر:
	أرجع ret;
}

ساكن صحيح جمع_کړه_thread_locked(بنية k_mem_domain *domain,
			     k_tid_t thread)
{
	صحيح ret = 0;

	__ASSERT_NO_MSG(domain != عدم);
	__ASSERT_NO_MSG(thread != عدم);

	LOG_DBG("add thread %p to domain %p", thread, domain);

#که_تعریف_شوی CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_append(&domain->thread_mem_domain_list,
			 &thread->mem_domain_info.thread_mem_domain_غوټه);
#د_که_پای /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

	thread->mem_domain_info.mem_domain = domain;

#که_تعریف_شوی CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_جمع_کړه(thread);
#د_که_پای /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	أرجع ret;
}

ساكن صحيح remove_thread_locked(بنية k_thread *thread)
{
	صحيح ret = 0;

	__ASSERT_NO_MSG(thread != عدم);
	LOG_DBG("remove thread %p from memory domain %p",
		thread, thread->mem_domain_info.mem_domain);

#که_تعریف_شوی CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_remove(&thread->mem_domain_info.thread_mem_domain_غوټه);
#د_که_پای /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#که_تعریف_شوی CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_remove(thread);
#د_که_پای /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	أرجع ret;
}

/* Called from thread object initialization */
فراغ z_mem_domain_init_thread(بنية k_thread *thread)
{
	صحيح ret;
	k_spinlock_کلی_t کلی = k_spin_قلف(&z_mem_domain_قلف);

	/* New threads inherit memory domain configuration from parent */
	ret = جمع_کړه_thread_locked(_current->mem_domain_info.mem_domain, thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_ناکارول_شوی(ret);

	k_spin_unlock(&z_mem_domain_قلف, کلی);
}

/* Called when thread aborts during teardown tasks. The scheduler's spinlock is held */
فراغ z_mem_domain_exit_thread(بنية k_thread *thread)
{
	صحيح ret;

	k_spinlock_کلی_t کلی = k_spin_قلف(&z_mem_domain_قلف);

	ret = remove_thread_locked(thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_ناکارول_شوی(ret);

	k_spin_unlock(&z_mem_domain_قلف, کلی);
}

صحيح k_mem_domain_جمع_کړه_thread(بنية k_mem_domain *domain, k_tid_t thread)
{
	صحيح ret = 0;
	k_spinlock_کلی_t کلی;

	کلی = k_spin_قلف(&z_mem_domain_قلف);
	إذا (thread->mem_domain_info.mem_domain != domain) {
		ret = remove_thread_locked(thread);

		إذا (ret == 0) {
			ret = جمع_کړه_thread_locked(domain, thread);
		}
	}
	k_spin_unlock(&z_mem_domain_قلف, کلی);

	أرجع ret;
}

ساكن فراغ init_mem_domain_module(فراغ)
{
	صحيح ret;

	ARG_ناکارول_شوی(ret);

	max_partitions = arch_mem_domain_max_partitions_get();
	/*
	 * max_partitions must be less than or equal to
	 * CONFIG_MAX_DOMAIN_PARTITIONS, or would encounter array index
	 * out of bounds error.
	 */
	__ASSERT(max_partitions <= CONFIG_MAX_DOMAIN_PARTITIONS, "");

	ret = k_mem_domain_init(&k_mem_domain_default, 0, عدم);
	__ASSERT(ret == 0, "failed to init default mem domain");

#که_تعریف_شوی Z_LIBC_برخه_کول_EXISTS
	ret = k_mem_domain_جمع_کړه_برخه_کول(&k_mem_domain_default,
					 &z_libc_برخه_کول);
	__ASSERT(ret == 0, "failed to add default libc mem partition");
#د_که_پای /* Z_LIBC_PARTITION_EXISTS */
}

K_KERNEL_INIT_PRE(init_mem_domain_module);
