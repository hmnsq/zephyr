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

بنية k_spinlock z_mem_domain_جندرا;
ساكن uint8_t max_partitions;

بنية k_mem_domain k_mem_domain_default;

ساكن bool check_جوڑو_حصے_کرو(بنية k_mem_domain *domain,
				بنية k_mem_حصے_کرو *part)
{

	صحيح i;
	uintptr_t pstart, pend, dstart, dend;

	إذا (part == عدم) {
		LOG_ERR("NULL k_mem_partition provided");
		أرجع false;
	}

#جے_تعریف_شدہ CONFIG_EXECUTE_XOR_WRITE
	/* Arches where execution cannot be disabled should always return
	 * false to this check
	 */
	إذا (K_MEM_حصے_کرو_IS_EXECUTABLE(part->attr) &&
	    K_MEM_حصے_کرو_IS_WRITABLE(part->attr)) {
		LOG_ERR("partition is writable and executable <start %lx>",
			part->شروع);
		أرجع false;
	}
#جے_انت /* CONFIG_EXECUTE_XOR_WRITE */

	إذا (part->ناپ == 0U) {
		LOG_ERR("zero sized partition at %p with base 0x%lx",
			part, part->شروع);
		أرجع false;
	}

	pstart = part->شروع;
	pend = part->شروع + part->ناپ;

	إذا (pend <= pstart) {
		LOG_ERR("invalid partition %p, wraparound detected. base 0x%lx size %zu",
			part, part->شروع, part->ناپ);
		أرجع false;
	}

	/* Check that this partition doesn't overlap any existing ones already
	 * in the domain
	 */
	لكل (i = 0; i < domain->عدد_partitions; i++) {
		بنية k_mem_حصے_کرو *dpart = &domain->partitions[i];

		إذا (dpart->ناپ == 0U) {
			/* Unused slot */
			استمر;
		}

		dstart = dpart->شروع;
		dend = dstart + dpart->ناپ;

		إذا (pend > dstart && dend > pstart) {
			LOG_ERR("partition %p base %lx (size %zu) overlaps existing base %lx (size %zu)",
				part, part->شروع, part->ناپ,
				dpart->شروع, dpart->ناپ);
			أرجع false;
		}
	}

	أرجع true;
}

صحيح k_mem_domain_init(بنية k_mem_domain *domain, uint8_t عدد_parts,
		      بنية k_mem_حصے_کرو *parts[])
{
	k_spinlock_کلید_t کلید;
	صحيح ret = 0;

	CHECKIF(domain == عدم) {
		ret = -EINVAL;
		اذهب باہرلا;
	}

	CHECKIF(!(عدد_parts == 0U || parts != عدم)) {
		LOG_ERR("parts array is NULL and num_parts is nonzero");
		ret = -EINVAL;
		اذهب باہرلا;
	}

	CHECKIF(!(عدد_parts <= max_partitions)) {
		LOG_ERR("num_parts of %d exceeds maximum allowable partitions (%d)",
			عدد_parts, max_partitions);
		ret = -EINVAL;
		اذهب باہرلا;
	}

	کلید = k_spin_جندرا(&z_mem_domain_جندرا);

	domain->عدد_partitions = 0U;
	(فراغ)memset(domain->partitions, 0, حجم(domain->partitions));

#جے_تعریف_شدہ CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_init(&domain->thread_mem_domain_list);
#جے_انت /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#جے_تعریف_شدہ CONFIG_ARCH_MEM_DOMAIN_انکڑے
	ret = arch_mem_domain_init(domain);

	إذا (ret != 0) {
		LOG_ERR("architecture-specific initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		اذهب unlock_باہرلا;
	}
#جے_انت /* CONFIG_ARCH_MEM_DOMAIN_DATA */
	إذا (عدد_parts != 0U) {
		uint32_t i;

		لكل (i = 0U; i < عدد_parts; i++) {
			CHECKIF(!check_جوڑو_حصے_کرو(domain, parts[i])) {
				LOG_ERR("invalid partition index %d (%p)",
					i, parts[i]);
				ret = -EINVAL;
				اذهب unlock_باہرلا;
			}

			domain->partitions[i] = *parts[i];
			domain->عدد_partitions++;
#جے_تعریف_شدہ CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
			صحيح ret2 = arch_mem_domain_حصے_کرو_جوڑو(domain, i);

			ARG_بن_ورتیا(ret2);
			CHECKIF(ret2 != 0) {
				ret = ret2;
			}
#جے_انت /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */
		}
	}

unlock_باہرلا:
	k_spin_unlock(&z_mem_domain_جندرا, کلید);

باہرلا:
	أرجع ret;
}

صحيح k_mem_domain_deinit(بنية k_mem_domain *domain)
{
#إذا defined(CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT)
	k_spinlock_کلید_t کلید;
	صحيح ret = 0;

	CHECKIF(domain == عدم) {
		ret = -EINVAL;
		اذهب باہرلا;
	}

	إذا (domain == &k_mem_domain_default) {
		/* Default memory domain must be there forever. */
		ret = -EINVAL;
		اذهب باہرلا;
	}

	کلید = k_spin_جندرا(&z_mem_domain_جندرا);

	/* Must make sure there are no threads associated with this memory
	 * domain anymore. Or else these threads will run with an invalid
	 * memory domain.
	 */
	إذا (!sys_dlist_is_خالی_تھاواں(&domain->thread_mem_domain_list)) {
		ret = -EBUSY;
		اذهب unlock_باہرلا;
	}

	ret = arch_mem_domain_deinit(domain);
	إذا (ret != 0) {
		LOG_ERR("architecture-specific de-initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		اذهب unlock_باہرلا;
	}

unlock_باہرلا:
	k_spin_unlock(&z_mem_domain_جندرا, کلید);

باہرلا:
	أرجع ret;
#وإلا  /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
	ARG_بن_ورتیا(domain);
	أرجع -ENOTSUP;
#جے_انت /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
}

صحيح k_mem_domain_جوڑو_حصے_کرو(بنية k_mem_domain *domain,
			       بنية k_mem_حصے_کرو *part)
{
	صحيح p_idx;
	k_spinlock_کلید_t کلید;
	صحيح ret = 0;

	CHECKIF(domain == عدم) {
		ret = -EINVAL;
		اذهب باہرلا;
	}

	CHECKIF(!check_جوڑو_حصے_کرو(domain, part)) {
		LOG_ERR("invalid partition %p", part);
		ret = -EINVAL;
		اذهب باہرلا;
	}

	کلید = k_spin_جندرا(&z_mem_domain_جندرا);

	لكل (p_idx = 0; p_idx < max_partitions; p_idx++) {
		/* A zero-sized partition denotes it's a free partition */
		إذا (domain->partitions[p_idx].ناپ == 0U) {
			اكسر;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no free partition slots available");
		ret = -ENOSPC;
		اذهب unlock_باہرلا;
	}

	LOG_DBG("add partition base %lx size %zu to domain %p",
		part->شروع, part->ناپ, domain);

	domain->partitions[p_idx].شروع = part->شروع;
	domain->partitions[p_idx].ناپ = part->ناپ;
	domain->partitions[p_idx].attr = part->attr;

	domain->عدد_partitions++;

#جے_تعریف_شدہ CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_حصے_کرو_جوڑو(domain, p_idx);
#جے_انت /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

unlock_باہرلا:
	k_spin_unlock(&z_mem_domain_جندرا, کلید);

باہرلا:
	أرجع ret;
}

صحيح k_mem_domain_remove_حصے_کرو(بنية k_mem_domain *domain,
				  بنية k_mem_حصے_کرو *part)
{
	صحيح p_idx;
	k_spinlock_کلید_t کلید;
	صحيح ret = 0;

	CHECKIF((domain == عدم) || (part == عدم)) {
		ret = -EINVAL;
		اذهب باہرلا;
	}

	کلید = k_spin_جندرا(&z_mem_domain_جندرا);

	/* find a partition that matches the given start and size */
	لكل (p_idx = 0; p_idx < max_partitions; p_idx++) {
		إذا ((domain->partitions[p_idx].شروع == part->شروع) &&
		    (domain->partitions[p_idx].ناپ == part->ناپ)) {
			اكسر;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no matching partition found");
		ret = -ENOENT;
		اذهب unlock_باہرلا;
	}

	LOG_DBG("remove partition base %lx size %zu from domain %p",
		part->شروع, part->ناپ, domain);

#جے_تعریف_شدہ CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_حصے_کرو_remove(domain, p_idx);
#جے_انت /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	/* A zero-sized partition denotes it's a free partition */
	domain->partitions[p_idx].ناپ = 0U;

	domain->عدد_partitions--;

unlock_باہرلا:
	k_spin_unlock(&z_mem_domain_جندرا, کلید);

باہرلا:
	أرجع ret;
}

ساكن صحيح جوڑو_thread_locked(بنية k_mem_domain *domain,
			     k_tid_t thread)
{
	صحيح ret = 0;

	__ASSERT_NO_MSG(domain != عدم);
	__ASSERT_NO_MSG(thread != عدم);

	LOG_DBG("add thread %p to domain %p", thread, domain);

#جے_تعریف_شدہ CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_append(&domain->thread_mem_domain_list,
			 &thread->mem_domain_info.thread_mem_domain_گرہ);
#جے_انت /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

	thread->mem_domain_info.mem_domain = domain;

#جے_تعریف_شدہ CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_جوڑو(thread);
#جے_انت /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	أرجع ret;
}

ساكن صحيح remove_thread_locked(بنية k_thread *thread)
{
	صحيح ret = 0;

	__ASSERT_NO_MSG(thread != عدم);
	LOG_DBG("remove thread %p from memory domain %p",
		thread, thread->mem_domain_info.mem_domain);

#جے_تعریف_شدہ CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_remove(&thread->mem_domain_info.thread_mem_domain_گرہ);
#جے_انت /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#جے_تعریف_شدہ CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_remove(thread);
#جے_انت /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	أرجع ret;
}

/* Called from thread object initialization */
فراغ z_mem_domain_init_thread(بنية k_thread *thread)
{
	صحيح ret;
	k_spinlock_کلید_t کلید = k_spin_جندرا(&z_mem_domain_جندرا);

	/* New threads inherit memory domain configuration from parent */
	ret = جوڑو_thread_locked(_current->mem_domain_info.mem_domain, thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_بن_ورتیا(ret);

	k_spin_unlock(&z_mem_domain_جندرا, کلید);
}

/* Called when thread aborts during teardown tasks. The scheduler's spinlock is held */
فراغ z_mem_domain_exit_thread(بنية k_thread *thread)
{
	صحيح ret;

	k_spinlock_کلید_t کلید = k_spin_جندرا(&z_mem_domain_جندرا);

	ret = remove_thread_locked(thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_بن_ورتیا(ret);

	k_spin_unlock(&z_mem_domain_جندرا, کلید);
}

صحيح k_mem_domain_جوڑو_thread(بنية k_mem_domain *domain, k_tid_t thread)
{
	صحيح ret = 0;
	k_spinlock_کلید_t کلید;

	کلید = k_spin_جندرا(&z_mem_domain_جندرا);
	إذا (thread->mem_domain_info.mem_domain != domain) {
		ret = remove_thread_locked(thread);

		إذا (ret == 0) {
			ret = جوڑو_thread_locked(domain, thread);
		}
	}
	k_spin_unlock(&z_mem_domain_جندرا, کلید);

	أرجع ret;
}

ساكن فراغ init_mem_domain_module(فراغ)
{
	صحيح ret;

	ARG_بن_ورتیا(ret);

	max_partitions = arch_mem_domain_max_partitions_get();
	/*
	 * max_partitions must be less than or equal to
	 * CONFIG_MAX_DOMAIN_PARTITIONS, or would encounter array index
	 * out of bounds error.
	 */
	__ASSERT(max_partitions <= CONFIG_MAX_DOMAIN_PARTITIONS, "");

	ret = k_mem_domain_init(&k_mem_domain_default, 0, عدم);
	__ASSERT(ret == 0, "failed to init default mem domain");

#جے_تعریف_شدہ Z_LIBC_حصے_کرو_EXISTS
	ret = k_mem_domain_جوڑو_حصے_کرو(&k_mem_domain_default,
					 &z_libc_حصے_کرو);
	__ASSERT(ret == 0, "failed to add default libc mem partition");
#جے_انت /* Z_LIBC_PARTITION_EXISTS */
}

K_KERNEL_INIT_PRE(init_mem_domain_module);
