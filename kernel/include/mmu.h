/*
 * Copyright (c) 2020 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#اگر_غیر_تعریف_شدہ KERNEL_INCLUDE_MMU_H
#تعریف_کرو KERNEL_INCLUDE_MMU_H

#اگر_تعریف_شدہ CONFIG_MMU

#شامل <stdint.h>
#شامل <zephyr/sys/sflist.h>
#شامل <zephyr/sys/__assert.h>
#شامل <zephyr/sys/util.h>
#شامل <zephyr/kernel/mm.h>
#شامل <zephyr/linker/linker-defs.h>
#شامل <zephyr/devicetree.h>

/** Start address of physical memory. */
#تعریف_کرو K_MEM_PHYS_RAM_آغاز	((uintptr_t)DT_CHOSEN_SRAM_ADDR)

/** Size of physical memory. */
#تعریف_کرو K_MEM_PHYS_RAM_سائز	DT_CHOSEN_SRAM_سائز

/** End address (exclusive) of physical memory. */
#تعریف_کرو K_MEM_PHYS_RAM_انتہا	(K_MEM_PHYS_RAM_آغاز + K_MEM_PHYS_RAM_سائز)

/** Start address of virtual memory. */
#تعریف_کرو K_MEM_VIRT_RAM_آغاز	((uint8_t *)CONFIG_KERNEL_VM_بنیاد)

/** Size of virtual memory. */
#تعریف_کرو K_MEM_VIRT_RAM_سائز	((size_t)CONFIG_KERNEL_VM_سائز)

/** End address (exclusive) of virtual memory. */
#تعریف_کرو K_MEM_VIRT_RAM_انتہا	(K_MEM_VIRT_RAM_آغاز + K_MEM_VIRT_RAM_سائز)

/** Boot-time virtual start address of the kernel image. */
#تعریف_کرو K_MEM_KERNEL_VIRT_آغاز	((uint8_t *)&z_mapped_آغاز[0])

/** Boot-time virtual end address of the kernel image. */
#تعریف_کرو K_MEM_KERNEL_VIRT_انتہا	((uint8_t *)&z_mapped_انتہا[0])

/** Boot-time virtual address space size of the kernel image. */
#تعریف_کرو K_MEM_KERNEL_VIRT_سائز	(K_MEM_KERNEL_VIRT_انتہا - K_MEM_KERNEL_VIRT_آغاز)

/**
 * @brief Offset for translating between static physical and virtual addresses.
 *
 * @note Do not use directly unless you know exactly what you are going.
 */
#تعریف_کرو K_MEM_VM_انحراف	\
	((CONFIG_KERNEL_VM_بنیاد + CONFIG_KERNEL_VM_انحراف) - \
	 (K_MEM_PHYS_RAM_آغاز + CONFIG_SRAM_انحراف))

/**
 * @brief Get physical address from virtual address for boot RAM mappings.
 *
 * @note Only applies to boot RAM mappings within the Zephyr image that have never
 *       been remapped or paged out. Never use this unless you know exactly what you
 *       are doing.
 *
 * @param virt Virtual address.
 *
 * @return Physical address.
 */
#تعریف_کرو K_MEM_BOOT_VIRT_مقصد_PHYS(virt) ((uintptr_t)(((uint8_t *)(virt)) - K_MEM_VM_انحراف))

/**
 * @brief Get virtual address from physical address for boot RAM mappings.
 *
 * @note Only applies to boot RAM mappings within the Zephyr image that have never
 *       been remapped or paged out. Never use this unless you know exactly what you
 *       are doing.
 *
 * @param phys Physical address.
 *
 * @return Virtual address.
 */
#تعریف_کرو K_MEM_BOOT_PHYS_مقصد_VIRT(phys) ((uint8_t *)(((uintptr_t)(phys)) + K_MEM_VM_انحراف))

/**
 * @def K_MEM_VM_FREE_START
 * @brief Start address of unused, available virtual addresses.
 *
 * This is the start address of the virtual memory region where
 * addresses can be allocated for memory mapping. This depends on whether
 * CONFIG_ARCH_MAPS_ALL_RAM is enabled:
 *
 * - If it is enabled, which means all physical memory are mapped in virtual
 *   memory address space, and it is the same as
 *   (DT_CHOSEN_SRAM_ADDR + DT_CHOSEN_SRAM_SIZE).
 *
 * - If it is disabled, K_MEM_VM_FREE_START is the same K_MEM_KERNEL_VIRT_END which
 *   is the end of the kernel image.
 *
 */
#اگر_تعریف_شدہ CONFIG_ARCH_MAPS_ALL_RAM
#تعریف_کرو K_MEM_VM_FREE_آغاز	K_MEM_BOOT_PHYS_مقصد_VIRT(K_MEM_PHYS_RAM_انتہا)
#ورنہ
#تعریف_کرو K_MEM_VM_FREE_آغاز	K_MEM_KERNEL_VIRT_انتہا
#اگر_ختم /* CONFIG_ARCH_MAPS_ALL_RAM */

/**
 * @defgroup kernel_mm_page_frame_apis Kernel Memory Page Frame Management APIs
 * @ingroup kernel_mm_internal_apis
 * @{
 *
 * Macros and data structures for physical page frame accounting,
 * APIs for use by eviction and backing store algorithms. This code
 * is otherwise not application-facing.
 */

/**
 * @brief Number of page frames.
 *
 * At present, page frame management is only done for main system RAM,
 * and we generate paging structures based on DT_CHOSEN_SRAM_ADDR
 * and DT_CHOSEN_SRAM_SIZE.
 *
 * If we have other RAM regions (DCCM, etc) these typically have special
 * properties and shouldn't be used generically for demand paging or
 * anonymous mappings. We don't currently maintain an ontology of these in the
 * core kernel.
 */
#تعریف_کرو K_MEM_عدد_صفحہ_چوکھٹے	(K_MEM_PHYS_RAM_سائز / (size_t)CONFIG_MMU_صفحہ_سائز)

/*
 * k_mem_page_frame flags bits
 *
 * Requirements:
 * - K_MEM_PAGE_FRAME_FREE must be one of the possible sfnode flag bits
 * - All bit values must be lower than CONFIG_MMU_PAGE_SIZE
 */

/** This physical page is free and part of the free list */
#تعریف_کرو K_MEM_صفحہ_چوکھٹا_FREE		ثنائی_ہندسہ(0)

/** This physical page is reserved by hardware; we will never use it */
#تعریف_کرو K_MEM_صفحہ_چوکھٹا_RESERVED	ثنائی_ہندسہ(1)

/** This page contains critical kernel data and will never be swapped */
#تعریف_کرو K_MEM_صفحہ_چوکھٹا_PINNED		ثنائی_ہندسہ(2)

/**
 * This physical page is mapped to some virtual memory address
 *
 * Currently, we just support one mapping per page frame. If a page frame
 * is mapped to multiple virtual pages then it must be pinned.
 */
#تعریف_کرو K_MEM_صفحہ_چوکھٹا_MAPPED		ثنائی_ہندسہ(3)

/**
 * This page frame is currently involved in a page-in/out operation
 */
#تعریف_کرو K_MEM_صفحہ_چوکھٹا_BUSY		ثنائی_ہندسہ(4)

/**
 * This page frame has a clean copy in the backing store
 */
#تعریف_کرو K_MEM_صفحہ_چوکھٹا_BACKED		ثنائی_ہندسہ(5)

/**
 * Data structure for physical page frames
 *
 * An array of these is instantiated, one element per physical RAM page.
 * Hence it's necessary to constrain its size as much as possible.
 */
ساخت k_mem_صفحہ_چوکھٹا {
	اتحاد {
		/*
		 * If mapped, K_MEM_PAGE_FRAME_* flags and virtual address
		 * this page is mapped to.
		 */
		uintptr_t va_and_flags;

		/*
		 * If unmapped and available, free pages list membership
		 * with the K_MEM_PAGE_FRAME_FREE flag.
		 */
		sys_sfnode_t گرہ;
	};

	/* Backing store and eviction algorithms may both need to
	 * require additional per-frame custom data for accounting purposes.
	 * They should declare their own array with indices matching
	 * k_mem_page_frames[] ones whenever possible.
	 * They may also want additional flags bits that could be stored here
	 * and they shouldn't clobber each other. At all costs the total
	 * size of struct k_mem_page_frame must be minimized.
	 */
};

/* Note: this must be false for the other flag bits to be valid */
ساکن اندرونی bool k_mem_صفحہ_چوکھٹا_is_free(ساخت k_mem_صفحہ_چوکھٹا *pf)
{
	واپس (pf->va_and_flags & K_MEM_صفحہ_چوکھٹا_FREE) != 0U;
}

ساکن اندرونی bool k_mem_صفحہ_چوکھٹا_is_pinned(ساخت k_mem_صفحہ_چوکھٹا *pf)
{
	واپس (pf->va_and_flags & K_MEM_صفحہ_چوکھٹا_PINNED) != 0U;
}

ساکن اندرونی bool k_mem_صفحہ_چوکھٹا_is_reserved(ساخت k_mem_صفحہ_چوکھٹا *pf)
{
	واپس (pf->va_and_flags & K_MEM_صفحہ_چوکھٹا_RESERVED) != 0U;
}

ساکن اندرونی bool k_mem_صفحہ_چوکھٹا_is_mapped(ساخت k_mem_صفحہ_چوکھٹا *pf)
{
	واپس (pf->va_and_flags & K_MEM_صفحہ_چوکھٹا_MAPPED) != 0U;
}

ساکن اندرونی bool k_mem_صفحہ_چوکھٹا_is_busy(ساخت k_mem_صفحہ_چوکھٹا *pf)
{
	واپس (pf->va_and_flags & K_MEM_صفحہ_چوکھٹا_BUSY) != 0U;
}

ساکن اندرونی bool k_mem_صفحہ_چوکھٹا_is_backed(ساخت k_mem_صفحہ_چوکھٹا *pf)
{
	واپس (pf->va_and_flags & K_MEM_صفحہ_چوکھٹا_BACKED) != 0U;
}

ساکن اندرونی bool k_mem_صفحہ_چوکھٹا_is_evictable(ساخت k_mem_صفحہ_چوکھٹا *pf)
{
	واپس (!k_mem_صفحہ_چوکھٹا_is_free(pf) &&
		!k_mem_صفحہ_چوکھٹا_is_reserved(pf) &&
		k_mem_صفحہ_چوکھٹا_is_mapped(pf) &&
		!k_mem_صفحہ_چوکھٹا_is_pinned(pf) &&
		!k_mem_صفحہ_چوکھٹا_is_busy(pf));
}

/* If true, page is not being used for anything, is not reserved, is not
 * a member of some free pages list, isn't busy, and is ready to be mapped
 * in memory
 */
ساکن اندرونی bool k_mem_صفحہ_چوکھٹا_is_دستیاب(ساخت k_mem_صفحہ_چوکھٹا *صفحہ)
{
	واپس صفحہ->va_and_flags == 0U;
}

ساکن اندرونی خالی k_mem_صفحہ_چوکھٹا_سیٹ(ساخت k_mem_صفحہ_چوکھٹا *pf, uint8_t flags)
{
	pf->va_and_flags |= flags;
}

ساکن اندرونی خالی k_mem_صفحہ_چوکھٹا_clear(ساخت k_mem_صفحہ_چوکھٹا *pf, uint8_t flags)
{
	/* ensure bit inversion to follow is done on the proper type width */
	uintptr_t wide_flags = flags;

	pf->va_and_flags &= ~wide_flags;
}

ساکن اندرونی خالی k_mem_assert_phys_aligned(uintptr_t phys)
{
	__ASSERT(phys % CONFIG_MMU_صفحہ_سائز == 0U,
		 "physical address 0x%lx is not page-aligned", phys);
	(خالی)phys;
}

خارجی ساخت k_mem_صفحہ_چوکھٹا k_mem_صفحہ_چوکھٹے[K_MEM_عدد_صفحہ_چوکھٹے];

ساکن اندرونی uintptr_t k_mem_صفحہ_چوکھٹا_مقصد_phys(ساخت k_mem_صفحہ_چوکھٹا *pf)
{
	واپس (uintptr_t)((pf - k_mem_صفحہ_چوکھٹے) * CONFIG_MMU_صفحہ_سائز) +
			  K_MEM_PHYS_RAM_آغاز;
}

/* Presumes there is but one mapping in the virtual address space */
ساکن اندرونی خالی *k_mem_صفحہ_چوکھٹا_مقصد_virt(ساخت k_mem_صفحہ_چوکھٹا *pf)
{
	uintptr_t flags_نقاب = CONFIG_MMU_صفحہ_سائز - 1;

	واپس (خالی *)(pf->va_and_flags & ~flags_نقاب);
}

ساکن اندرونی bool k_mem_is_صفحہ_چوکھٹا(uintptr_t phys)
{
	k_mem_assert_phys_aligned(phys);
	واپس اندر_RANGE(phys, (uintptr_t)K_MEM_PHYS_RAM_آغاز,
			(uintptr_t)(K_MEM_PHYS_RAM_انتہا - 1));
}

ساکن اندرونی ساخت k_mem_صفحہ_چوکھٹا *k_mem_phys_مقصد_صفحہ_چوکھٹا(uintptr_t phys)
{
	__ASSERT(k_mem_is_صفحہ_چوکھٹا(phys),
		 "0x%lx not an SRAM physical address", phys);

	واپس &k_mem_صفحہ_چوکھٹے[(phys - K_MEM_PHYS_RAM_آغاز) /
				  CONFIG_MMU_صفحہ_سائز];
}

ساکن اندرونی خالی k_mem_assert_virtual_region(uint8_t *addr, size_t سائز)
{
	__ASSERT((uintptr_t)addr % CONFIG_MMU_صفحہ_سائز == 0U,
		 "unaligned addr %p", addr);
	__ASSERT(سائز % CONFIG_MMU_صفحہ_سائز == 0U,
		 "unaligned size %zu", سائز);
	__ASSERT(!Z_DETECT_اشارہ_گر_حد_سے_تجاوز(addr, سائز),
		 "region %p size %zu zero or wraps around", addr, سائز);
	__ASSERT(اندر_RANGE((uintptr_t)addr,
			  (uintptr_t)K_MEM_VIRT_RAM_آغاز,
			  ((uintptr_t)K_MEM_VIRT_RAM_انتہا - 1)) &&
		 اندر_RANGE(((uintptr_t)addr + سائز - 1),
			  (uintptr_t)K_MEM_VIRT_RAM_آغاز,
			  ((uintptr_t)K_MEM_VIRT_RAM_انتہا - 1)),
		 "invalid virtual address region %p (%zu)", addr, سائز);
}

/**
 * @brief Pretty-print page frame information for all page frames.
 *
 * Debug function, pretty-print page frame information for all frames
 * concisely to printk.
 */
خالی k_mem_صفحہ_چوکھٹے_dump(خالی);

/* Convenience macro for iterating over all page frames */
#تعریف_کرو K_MEM_صفحہ_چوکھٹا_FOREACH(_phys, _pageframe) \
	برائے ((_phys) = K_MEM_PHYS_RAM_آغاز, (_pageframe) = k_mem_صفحہ_چوکھٹے; \
	     (_phys) < K_MEM_PHYS_RAM_انتہا; \
	     (_phys) += CONFIG_MMU_صفحہ_سائز, (_pageframe)++)

/** @} */

/**
 * @def K_MEM_VM_RESERVED
 * @brief Reserve space at the end of virtual memory.
 */
#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING
/* We reserve a virtual page as a scratch area for page-ins/outs at the end
 * of the address space
 */
#تعریف_کرو K_MEM_VM_RESERVED	CONFIG_MMU_صفحہ_سائز

/**
 * @brief Location of the scratch page used for demand paging.
 */
#تعریف_کرو K_MEM_SCRATCH_صفحہ	((خالی *)((uintptr_t)CONFIG_KERNEL_VM_بنیاد + \
					  (uintptr_t)CONFIG_KERNEL_VM_سائز - \
					  CONFIG_MMU_صفحہ_سائز))
#ورنہ
#تعریف_کرو K_MEM_VM_RESERVED	0
#اگر_ختم /* CONFIG_DEMAND_PAGING */

#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING
/*
 * Core kernel demand paging APIs
 */

/**
 * Number of page faults since system startup
 *
 * Counts only those page faults that were handled successfully by the demand
 * paging mechanism and were not errors.
 *
 * @return Number of successful page faults
 */
غیرعلامتی طویل k_mem_عدد_pagefaults_get(خالی);

/**
 * Free a page frame physical address by evicting its contents
 *
 * The indicated page frame, if it contains a data page, will have that
 * data page evicted to the backing store. The page frame will then be
 * marked as available for mappings or page-ins.
 *
 * This is useful for freeing up entire memory banks so that they may be
 * deactivated to save power.
 *
 * If CONFIG_DEMAND_PAGING_ALLOW_IRQ is enabled, this function may not be
 * called by ISRs as the backing store may be in-use.
 *
 * @param phys Page frame physical address
 * @retval 0 Success
 * @retval -ENOMEM Insufficient backing store space
 */
صحیح k_mem_صفحہ_چوکھٹا_evict(uintptr_t phys);

/**
 * Handle a page fault for a virtual data page
 *
 * This is invoked from the architecture page fault handler.
 *
 * If a valid page fault, the core kernel will obtain a page frame,
 * populate it with the data page that was evicted to the backing store,
 * update page tables, and return so that the faulting instruction may be
 * re-tried.
 *
 * The architecture must not call this function if the page was mapped and
 * not paged out at the time the exception was triggered (i.e. a protection
 * violation for a mapped page).
 *
 * If the faulting context had interrupts disabled when the page fault was
 * triggered, the entire page fault handling path must have interrupts
 * disabled, including the invocation of this function.
 *
 * Otherwise, interrupts may be enabled and the page fault handler may be
 * preemptible. Races to page-in will be appropriately handled by the kernel.
 *
 * @param addr Faulting virtual address
 * @retval true Page fault successfully handled, or nothing needed to be done.
 *              The arch layer should retry the faulting instruction.
 * @retval false This page fault was from an un-mapped page, should
 *               be treated as an error, and not re-tried.
 */
bool k_mem_صفحہ_fault(خالی *addr);

#اگر_ختم /* CONFIG_DEMAND_PAGING */
#اگر_ختم /* CONFIG_MMU */
#اگر_ختم /* KERNEL_INCLUDE_MMU_H */
