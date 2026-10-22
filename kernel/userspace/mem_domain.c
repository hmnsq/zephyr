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

بنية k_spinlock z_mem_domain_قفل;
ساكن uint8_t max_partitions;

بنية k_mem_domain k_mem_domain_default;

ساكن bool check_جمع_کن_افراز(بنية k_mem_domain *domain,
				بنية k_mem_افراز *part)
{

	صحيح i;
	uintptr_t pstart, pend, dstart, dend;

	إذا (part == عدم) {
		LOG_ERR("NULL k_mem_partition provided");
		أرجع false;
	}

#اگر_تعریف_شده CONFIG_EXECUTE_XOR_WRITE
	/* Arches where execution cannot be disabled should always return
	 * false to this check
	 */
	إذا (K_MEM_افراز_IS_EXECUTABLE(part->attr) &&
	    K_MEM_افراز_IS_WRITABLE(part->attr)) {
		LOG_ERR("partition is writable and executable <start %lx>",
			part->شروع);
		أرجع false;
	}
#پایان_اگر /* CONFIG_EXECUTE_XOR_WRITE */

	إذا (part->اندازه == 0U) {
		LOG_ERR("zero sized partition at %p with base 0x%lx",
			part, part->شروع);
		أرجع false;
	}

	pstart = part->شروع;
	pend = part->شروع + part->اندازه;

	إذا (pend <= pstart) {
		LOG_ERR("invalid partition %p, wraparound detected. base 0x%lx size %zu",
			part, part->شروع, part->اندازه);
		أرجع false;
	}

	/* Check that this partition doesn't overlap any existing ones already
	 * in the domain
	 */
	لكل (i = 0; i < domain->رقم_partitions; i++) {
		بنية k_mem_افراز *dpart = &domain->partitions[i];

		إذا (dpart->اندازه == 0U) {
			/* Unused slot */
			استمر;
		}

		dstart = dpart->شروع;
		dend = dstart + dpart->اندازه;

		إذا (pend > dstart && dend > pstart) {
			LOG_ERR("partition %p base %lx (size %zu) overlaps existing base %lx (size %zu)",
				part, part->شروع, part->اندازه,
				dpart->شروع, dpart->اندازه);
			أرجع false;
		}
	}

	أرجع true;
}

صحيح k_mem_domain_init(بنية k_mem_domain *domain, uint8_t رقم_parts,
		      بنية k_mem_افراز *parts[])
{
	k_spinlock_کلید_t کلید;
	صحيح ret = 0;

	CHECKIF(domain == عدم) {
		ret = -EINVAL;
		اذهب بیرون;
	}

	CHECKIF(!(رقم_parts == 0U || parts != عدم)) {
		LOG_ERR("parts array is NULL and num_parts is nonzero");
		ret = -EINVAL;
		اذهب بیرون;
	}

	CHECKIF(!(رقم_parts <= max_partitions)) {
		LOG_ERR("num_parts of %d exceeds maximum allowable partitions (%d)",
			رقم_parts, max_partitions);
		ret = -EINVAL;
		اذهب بیرون;
	}

	کلید = k_spin_قفل(&z_mem_domain_قفل);

	domain->رقم_partitions = 0U;
	(فراغ)memset(domain->partitions, 0, حجم(domain->partitions));

#اگر_تعریف_شده CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_init(&domain->thread_mem_domain_list);
#پایان_اگر /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#اگر_تعریف_شده CONFIG_ARCH_MEM_DOMAIN_داده
	ret = arch_mem_domain_init(domain);

	إذا (ret != 0) {
		LOG_ERR("architecture-specific initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		اذهب unlock_بیرون;
	}
#پایان_اگر /* CONFIG_ARCH_MEM_DOMAIN_DATA */
	إذا (رقم_parts != 0U) {
		uint32_t i;

		لكل (i = 0U; i < رقم_parts; i++) {
			CHECKIF(!check_جمع_کن_افراز(domain, parts[i])) {
				LOG_ERR("invalid partition index %d (%p)",
					i, parts[i]);
				ret = -EINVAL;
				اذهب unlock_بیرون;
			}

			domain->partitions[i] = *parts[i];
			domain->رقم_partitions++;
#اگر_تعریف_شده CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
			صحيح ret2 = arch_mem_domain_افراز_جمع_کن(domain, i);

			ARG_استفادهنشده(ret2);
			CHECKIF(ret2 != 0) {
				ret = ret2;
			}
#پایان_اگر /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */
		}
	}

unlock_بیرون:
	k_spin_unlock(&z_mem_domain_قفل, کلید);

بیرون:
	أرجع ret;
}

صحيح k_mem_domain_deinit(بنية k_mem_domain *domain)
{
#إذا defined(CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT)
	k_spinlock_کلید_t کلید;
	صحيح ret = 0;

	CHECKIF(domain == عدم) {
		ret = -EINVAL;
		اذهب بیرون;
	}

	إذا (domain == &k_mem_domain_default) {
		/* Default memory domain must be there forever. */
		ret = -EINVAL;
		اذهب بیرون;
	}

	کلید = k_spin_قفل(&z_mem_domain_قفل);

	/* Must make sure there are no threads associated with this memory
	 * domain anymore. Or else these threads will run with an invalid
	 * memory domain.
	 */
	إذا (!sys_dlist_is_خالی(&domain->thread_mem_domain_list)) {
		ret = -EBUSY;
		اذهب unlock_بیرون;
	}

	ret = arch_mem_domain_deinit(domain);
	إذا (ret != 0) {
		LOG_ERR("architecture-specific de-initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		اذهب unlock_بیرون;
	}

unlock_بیرون:
	k_spin_unlock(&z_mem_domain_قفل, کلید);

بیرون:
	أرجع ret;
#وإلا  /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
	ARG_استفادهنشده(domain);
	أرجع -ENOTSUP;
#پایان_اگر /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
}

صحيح k_mem_domain_جمع_کن_افراز(بنية k_mem_domain *domain,
			       بنية k_mem_افراز *part)
{
	صحيح p_idx;
	k_spinlock_کلید_t کلید;
	صحيح ret = 0;

	CHECKIF(domain == عدم) {
		ret = -EINVAL;
		اذهب بیرون;
	}

	CHECKIF(!check_جمع_کن_افراز(domain, part)) {
		LOG_ERR("invalid partition %p", part);
		ret = -EINVAL;
		اذهب بیرون;
	}

	کلید = k_spin_قفل(&z_mem_domain_قفل);

	لكل (p_idx = 0; p_idx < max_partitions; p_idx++) {
		/* A zero-sized partition denotes it's a free partition */
		إذا (domain->partitions[p_idx].اندازه == 0U) {
			اكسر;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no free partition slots available");
		ret = -ENOSPC;
		اذهب unlock_بیرون;
	}

	LOG_DBG("add partition base %lx size %zu to domain %p",
		part->شروع, part->اندازه, domain);

	domain->partitions[p_idx].شروع = part->شروع;
	domain->partitions[p_idx].اندازه = part->اندازه;
	domain->partitions[p_idx].attr = part->attr;

	domain->رقم_partitions++;

#اگر_تعریف_شده CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_افراز_جمع_کن(domain, p_idx);
#پایان_اگر /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

unlock_بیرون:
	k_spin_unlock(&z_mem_domain_قفل, کلید);

بیرون:
	أرجع ret;
}

صحيح k_mem_domain_remove_افراز(بنية k_mem_domain *domain,
				  بنية k_mem_افراز *part)
{
	صحيح p_idx;
	k_spinlock_کلید_t کلید;
	صحيح ret = 0;

	CHECKIF((domain == عدم) || (part == عدم)) {
		ret = -EINVAL;
		اذهب بیرون;
	}

	کلید = k_spin_قفل(&z_mem_domain_قفل);

	/* find a partition that matches the given start and size */
	لكل (p_idx = 0; p_idx < max_partitions; p_idx++) {
		إذا ((domain->partitions[p_idx].شروع == part->شروع) &&
		    (domain->partitions[p_idx].اندازه == part->اندازه)) {
			اكسر;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no matching partition found");
		ret = -ENOENT;
		اذهب unlock_بیرون;
	}

	LOG_DBG("remove partition base %lx size %zu from domain %p",
		part->شروع, part->اندازه, domain);

#اگر_تعریف_شده CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_افراز_remove(domain, p_idx);
#پایان_اگر /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	/* A zero-sized partition denotes it's a free partition */
	domain->partitions[p_idx].اندازه = 0U;

	domain->رقم_partitions--;

unlock_بیرون:
	k_spin_unlock(&z_mem_domain_قفل, کلید);

بیرون:
	أرجع ret;
}

ساكن صحيح جمع_کن_thread_locked(بنية k_mem_domain *domain,
			     k_tid_t thread)
{
	صحيح ret = 0;

	__ASSERT_NO_MSG(domain != عدم);
	__ASSERT_NO_MSG(thread != عدم);

	LOG_DBG("add thread %p to domain %p", thread, domain);

#اگر_تعریف_شده CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_append(&domain->thread_mem_domain_list,
			 &thread->mem_domain_info.thread_mem_domain_گره);
#پایان_اگر /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

	thread->mem_domain_info.mem_domain = domain;

#اگر_تعریف_شده CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_جمع_کن(thread);
#پایان_اگر /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	أرجع ret;
}

ساكن صحيح remove_thread_locked(بنية k_thread *thread)
{
	صحيح ret = 0;

	__ASSERT_NO_MSG(thread != عدم);
	LOG_DBG("remove thread %p from memory domain %p",
		thread, thread->mem_domain_info.mem_domain);

#اگر_تعریف_شده CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_remove(&thread->mem_domain_info.thread_mem_domain_گره);
#پایان_اگر /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#اگر_تعریف_شده CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_remove(thread);
#پایان_اگر /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	أرجع ret;
}

/* Called from thread object initialization */
فراغ z_mem_domain_init_thread(بنية k_thread *thread)
{
	صحيح ret;
	k_spinlock_کلید_t کلید = k_spin_قفل(&z_mem_domain_قفل);

	/* New threads inherit memory domain configuration from parent */
	ret = جمع_کن_thread_locked(_current->mem_domain_info.mem_domain, thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_استفادهنشده(ret);

	k_spin_unlock(&z_mem_domain_قفل, کلید);
}

/* Called when thread aborts during teardown tasks. The scheduler's spinlock is held */
فراغ z_mem_domain_exit_thread(بنية k_thread *thread)
{
	صحيح ret;

	k_spinlock_کلید_t کلید = k_spin_قفل(&z_mem_domain_قفل);

	ret = remove_thread_locked(thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_استفادهنشده(ret);

	k_spin_unlock(&z_mem_domain_قفل, کلید);
}

صحيح k_mem_domain_جمع_کن_thread(بنية k_mem_domain *domain, k_tid_t thread)
{
	صحيح ret = 0;
	k_spinlock_کلید_t کلید;

	کلید = k_spin_قفل(&z_mem_domain_قفل);
	إذا (thread->mem_domain_info.mem_domain != domain) {
		ret = remove_thread_locked(thread);

		إذا (ret == 0) {
			ret = جمع_کن_thread_locked(domain, thread);
		}
	}
	k_spin_unlock(&z_mem_domain_قفل, کلید);

	أرجع ret;
}

ساكن فراغ init_mem_domain_module(فراغ)
{
	صحيح ret;

	ARG_استفادهنشده(ret);

	max_partitions = arch_mem_domain_max_partitions_get();
	/*
	 * max_partitions must be less than or equal to
	 * CONFIG_MAX_DOMAIN_PARTITIONS, or would encounter array index
	 * out of bounds error.
	 */
	__ASSERT(max_partitions <= CONFIG_MAX_DOMAIN_PARTITIONS, "");

	ret = k_mem_domain_init(&k_mem_domain_default, 0, عدم);
	__ASSERT(ret == 0, "failed to init default mem domain");

#اگر_تعریف_شده Z_LIBC_افراز_EXISTS
	ret = k_mem_domain_جمع_کن_افراز(&k_mem_domain_default,
					 &z_libc_افراز);
	__ASSERT(ret == 0, "failed to add default libc mem partition");
#پایان_اگر /* Z_LIBC_PARTITION_EXISTS */
}

K_KERNEL_INIT_PRE(init_mem_domain_module);
