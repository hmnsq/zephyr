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

بنية k_spinlock z_mem_domain_تالو;
ساكن uint8_t max_partitions;

بنية k_mem_domain k_mem_domain_default;

ساكن bool check_جوڙيو_حصا_ڪريو(بنية k_mem_domain *domain,
				بنية k_mem_حصا_ڪريو *part)
{

	صحيح i;
	uintptr_t pstart, pend, dstart, dend;

	إذا (part == عدم) {
		LOG_ERR("NULL k_mem_partition provided");
		أرجع false;
	}

#جيڪڏهن_وصف_ٿيل CONFIG_EXECUTE_XOR_WRITE
	/* Arches where execution cannot be disabled should always return
	 * false to this check
	 */
	إذا (K_MEM_حصا_ڪريو_IS_EXECUTABLE(part->attr) &&
	    K_MEM_حصا_ڪريو_IS_WRITABLE(part->attr)) {
		LOG_ERR("partition is writable and executable <start %lx>",
			part->شروعات);
		أرجع false;
	}
#جيڪڏهن_پورو /* CONFIG_EXECUTE_XOR_WRITE */

	إذا (part->سائيز == 0U) {
		LOG_ERR("zero sized partition at %p with base 0x%lx",
			part, part->شروعات);
		أرجع false;
	}

	pstart = part->شروعات;
	pend = part->شروعات + part->سائيز;

	إذا (pend <= pstart) {
		LOG_ERR("invalid partition %p, wraparound detected. base 0x%lx size %zu",
			part, part->شروعات, part->سائيز);
		أرجع false;
	}

	/* Check that this partition doesn't overlap any existing ones already
	 * in the domain
	 */
	لكل (i = 0; i < domain->انگ_partitions; i++) {
		بنية k_mem_حصا_ڪريو *dpart = &domain->partitions[i];

		إذا (dpart->سائيز == 0U) {
			/* Unused slot */
			استمر;
		}

		dstart = dpart->شروعات;
		dend = dstart + dpart->سائيز;

		إذا (pend > dstart && dend > pstart) {
			LOG_ERR("partition %p base %lx (size %zu) overlaps existing base %lx (size %zu)",
				part, part->شروعات, part->سائيز,
				dpart->شروعات, dpart->سائيز);
			أرجع false;
		}
	}

	أرجع true;
}

صحيح k_mem_domain_init(بنية k_mem_domain *domain, uint8_t انگ_parts,
		      بنية k_mem_حصا_ڪريو *parts[])
{
	k_spinlock_ڪنجي_t ڪنجي;
	صحيح ret = 0;

	CHECKIF(domain == عدم) {
		ret = -EINVAL;
		اذهب ٻاهريون;
	}

	CHECKIF(!(انگ_parts == 0U || parts != عدم)) {
		LOG_ERR("parts array is NULL and num_parts is nonzero");
		ret = -EINVAL;
		اذهب ٻاهريون;
	}

	CHECKIF(!(انگ_parts <= max_partitions)) {
		LOG_ERR("num_parts of %d exceeds maximum allowable partitions (%d)",
			انگ_parts, max_partitions);
		ret = -EINVAL;
		اذهب ٻاهريون;
	}

	ڪنجي = k_spin_تالو(&z_mem_domain_تالو);

	domain->انگ_partitions = 0U;
	(فراغ)memset(domain->partitions, 0, حجم(domain->partitions));

#جيڪڏهن_وصف_ٿيل CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_init(&domain->thread_mem_domain_list);
#جيڪڏهن_پورو /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#جيڪڏهن_وصف_ٿيل CONFIG_ARCH_MEM_DOMAIN_معلومات
	ret = arch_mem_domain_init(domain);

	إذا (ret != 0) {
		LOG_ERR("architecture-specific initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		اذهب unlock_ٻاهريون;
	}
#جيڪڏهن_پورو /* CONFIG_ARCH_MEM_DOMAIN_DATA */
	إذا (انگ_parts != 0U) {
		uint32_t i;

		لكل (i = 0U; i < انگ_parts; i++) {
			CHECKIF(!check_جوڙيو_حصا_ڪريو(domain, parts[i])) {
				LOG_ERR("invalid partition index %d (%p)",
					i, parts[i]);
				ret = -EINVAL;
				اذهب unlock_ٻاهريون;
			}

			domain->partitions[i] = *parts[i];
			domain->انگ_partitions++;
#جيڪڏهن_وصف_ٿيل CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
			صحيح ret2 = arch_mem_domain_حصا_ڪريو_جوڙيو(domain, i);

			ARG_اڻ_استعمال_ٿيل(ret2);
			CHECKIF(ret2 != 0) {
				ret = ret2;
			}
#جيڪڏهن_پورو /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */
		}
	}

unlock_ٻاهريون:
	k_spin_unlock(&z_mem_domain_تالو, ڪنجي);

ٻاهريون:
	أرجع ret;
}

صحيح k_mem_domain_deinit(بنية k_mem_domain *domain)
{
#إذا defined(CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT)
	k_spinlock_ڪنجي_t ڪنجي;
	صحيح ret = 0;

	CHECKIF(domain == عدم) {
		ret = -EINVAL;
		اذهب ٻاهريون;
	}

	إذا (domain == &k_mem_domain_default) {
		/* Default memory domain must be there forever. */
		ret = -EINVAL;
		اذهب ٻاهريون;
	}

	ڪنجي = k_spin_تالو(&z_mem_domain_تالو);

	/* Must make sure there are no threads associated with this memory
	 * domain anymore. Or else these threads will run with an invalid
	 * memory domain.
	 */
	إذا (!sys_dlist_is_خالي_جايون(&domain->thread_mem_domain_list)) {
		ret = -EBUSY;
		اذهب unlock_ٻاهريون;
	}

	ret = arch_mem_domain_deinit(domain);
	إذا (ret != 0) {
		LOG_ERR("architecture-specific de-initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		اذهب unlock_ٻاهريون;
	}

unlock_ٻاهريون:
	k_spin_unlock(&z_mem_domain_تالو, ڪنجي);

ٻاهريون:
	أرجع ret;
#وإلا  /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
	ARG_اڻ_استعمال_ٿيل(domain);
	أرجع -ENOTSUP;
#جيڪڏهن_پورو /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
}

صحيح k_mem_domain_جوڙيو_حصا_ڪريو(بنية k_mem_domain *domain,
			       بنية k_mem_حصا_ڪريو *part)
{
	صحيح p_idx;
	k_spinlock_ڪنجي_t ڪنجي;
	صحيح ret = 0;

	CHECKIF(domain == عدم) {
		ret = -EINVAL;
		اذهب ٻاهريون;
	}

	CHECKIF(!check_جوڙيو_حصا_ڪريو(domain, part)) {
		LOG_ERR("invalid partition %p", part);
		ret = -EINVAL;
		اذهب ٻاهريون;
	}

	ڪنجي = k_spin_تالو(&z_mem_domain_تالو);

	لكل (p_idx = 0; p_idx < max_partitions; p_idx++) {
		/* A zero-sized partition denotes it's a free partition */
		إذا (domain->partitions[p_idx].سائيز == 0U) {
			اكسر;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no free partition slots available");
		ret = -ENOSPC;
		اذهب unlock_ٻاهريون;
	}

	LOG_DBG("add partition base %lx size %zu to domain %p",
		part->شروعات, part->سائيز, domain);

	domain->partitions[p_idx].شروعات = part->شروعات;
	domain->partitions[p_idx].سائيز = part->سائيز;
	domain->partitions[p_idx].attr = part->attr;

	domain->انگ_partitions++;

#جيڪڏهن_وصف_ٿيل CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_حصا_ڪريو_جوڙيو(domain, p_idx);
#جيڪڏهن_پورو /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

unlock_ٻاهريون:
	k_spin_unlock(&z_mem_domain_تالو, ڪنجي);

ٻاهريون:
	أرجع ret;
}

صحيح k_mem_domain_remove_حصا_ڪريو(بنية k_mem_domain *domain,
				  بنية k_mem_حصا_ڪريو *part)
{
	صحيح p_idx;
	k_spinlock_ڪنجي_t ڪنجي;
	صحيح ret = 0;

	CHECKIF((domain == عدم) || (part == عدم)) {
		ret = -EINVAL;
		اذهب ٻاهريون;
	}

	ڪنجي = k_spin_تالو(&z_mem_domain_تالو);

	/* find a partition that matches the given start and size */
	لكل (p_idx = 0; p_idx < max_partitions; p_idx++) {
		إذا ((domain->partitions[p_idx].شروعات == part->شروعات) &&
		    (domain->partitions[p_idx].سائيز == part->سائيز)) {
			اكسر;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no matching partition found");
		ret = -ENOENT;
		اذهب unlock_ٻاهريون;
	}

	LOG_DBG("remove partition base %lx size %zu from domain %p",
		part->شروعات, part->سائيز, domain);

#جيڪڏهن_وصف_ٿيل CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_حصا_ڪريو_remove(domain, p_idx);
#جيڪڏهن_پورو /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	/* A zero-sized partition denotes it's a free partition */
	domain->partitions[p_idx].سائيز = 0U;

	domain->انگ_partitions--;

unlock_ٻاهريون:
	k_spin_unlock(&z_mem_domain_تالو, ڪنجي);

ٻاهريون:
	أرجع ret;
}

ساكن صحيح جوڙيو_thread_locked(بنية k_mem_domain *domain,
			     k_tid_t thread)
{
	صحيح ret = 0;

	__ASSERT_NO_MSG(domain != عدم);
	__ASSERT_NO_MSG(thread != عدم);

	LOG_DBG("add thread %p to domain %p", thread, domain);

#جيڪڏهن_وصف_ٿيل CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_append(&domain->thread_mem_domain_list,
			 &thread->mem_domain_info.thread_mem_domain_ڳنڍ);
#جيڪڏهن_پورو /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

	thread->mem_domain_info.mem_domain = domain;

#جيڪڏهن_وصف_ٿيل CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_جوڙيو(thread);
#جيڪڏهن_پورو /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	أرجع ret;
}

ساكن صحيح remove_thread_locked(بنية k_thread *thread)
{
	صحيح ret = 0;

	__ASSERT_NO_MSG(thread != عدم);
	LOG_DBG("remove thread %p from memory domain %p",
		thread, thread->mem_domain_info.mem_domain);

#جيڪڏهن_وصف_ٿيل CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_remove(&thread->mem_domain_info.thread_mem_domain_ڳنڍ);
#جيڪڏهن_پورو /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#جيڪڏهن_وصف_ٿيل CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_remove(thread);
#جيڪڏهن_پورو /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	أرجع ret;
}

/* Called from thread object initialization */
فراغ z_mem_domain_init_thread(بنية k_thread *thread)
{
	صحيح ret;
	k_spinlock_ڪنجي_t ڪنجي = k_spin_تالو(&z_mem_domain_تالو);

	/* New threads inherit memory domain configuration from parent */
	ret = جوڙيو_thread_locked(_current->mem_domain_info.mem_domain, thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_اڻ_استعمال_ٿيل(ret);

	k_spin_unlock(&z_mem_domain_تالو, ڪنجي);
}

/* Called when thread aborts during teardown tasks. The scheduler's spinlock is held */
فراغ z_mem_domain_exit_thread(بنية k_thread *thread)
{
	صحيح ret;

	k_spinlock_ڪنجي_t ڪنجي = k_spin_تالو(&z_mem_domain_تالو);

	ret = remove_thread_locked(thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_اڻ_استعمال_ٿيل(ret);

	k_spin_unlock(&z_mem_domain_تالو, ڪنجي);
}

صحيح k_mem_domain_جوڙيو_thread(بنية k_mem_domain *domain, k_tid_t thread)
{
	صحيح ret = 0;
	k_spinlock_ڪنجي_t ڪنجي;

	ڪنجي = k_spin_تالو(&z_mem_domain_تالو);
	إذا (thread->mem_domain_info.mem_domain != domain) {
		ret = remove_thread_locked(thread);

		إذا (ret == 0) {
			ret = جوڙيو_thread_locked(domain, thread);
		}
	}
	k_spin_unlock(&z_mem_domain_تالو, ڪنجي);

	أرجع ret;
}

ساكن فراغ init_mem_domain_module(فراغ)
{
	صحيح ret;

	ARG_اڻ_استعمال_ٿيل(ret);

	max_partitions = arch_mem_domain_max_partitions_get();
	/*
	 * max_partitions must be less than or equal to
	 * CONFIG_MAX_DOMAIN_PARTITIONS, or would encounter array index
	 * out of bounds error.
	 */
	__ASSERT(max_partitions <= CONFIG_MAX_DOMAIN_PARTITIONS, "");

	ret = k_mem_domain_init(&k_mem_domain_default, 0, عدم);
	__ASSERT(ret == 0, "failed to init default mem domain");

#جيڪڏهن_وصف_ٿيل Z_LIBC_حصا_ڪريو_EXISTS
	ret = k_mem_domain_جوڙيو_حصا_ڪريو(&k_mem_domain_default,
					 &z_libc_حصا_ڪريو);
	__ASSERT(ret == 0, "failed to add default libc mem partition");
#جيڪڏهن_پورو /* Z_LIBC_PARTITION_EXISTS */
}

K_KERNEL_INIT_PRE(init_mem_domain_module);
