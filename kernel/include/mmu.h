/*
 * Copyright (c) 2020 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#که_نه_تعریف_شوی KERNEL_INCLUDE_MMU_H
#تعریف_کړه KERNEL_INCLUDE_MMU_H

#که_تعریف_شوی CONFIG_MMU

#ضمن <stdint.h>
#ضمن <zephyr/sys/sflist.h>
#ضمن <zephyr/sys/__assert.h>
#ضمن <zephyr/sys/util.h>
#ضمن <zephyr/kernel/mm.h>
#ضمن <zephyr/linker/linker-defs.h>
#ضمن <zephyr/devicetree.h>

/** Start address of physical memory. */
#تعریف_کړه K_MEM_PHYS_RAM_پیل	((uintptr_t)DT_CHOSEN_SRAM_ADDR)

/** Size of physical memory. */
#تعریف_کړه K_MEM_PHYS_RAM_اندازه	DT_CHOSEN_SRAM_اندازه

/** End address (exclusive) of physical memory. */
#تعریف_کړه K_MEM_PHYS_RAM_پای_ټکی	(K_MEM_PHYS_RAM_پیل + K_MEM_PHYS_RAM_اندازه)

/** Start address of virtual memory. */
#تعریف_کړه K_MEM_VIRT_RAM_پیل	((uint8_t *)CONFIG_KERNEL_VM_بنسټ)

/** Size of virtual memory. */
#تعریف_کړه K_MEM_VIRT_RAM_اندازه	((size_t)CONFIG_KERNEL_VM_اندازه)

/** End address (exclusive) of virtual memory. */
#تعریف_کړه K_MEM_VIRT_RAM_پای_ټکی	(K_MEM_VIRT_RAM_پیل + K_MEM_VIRT_RAM_اندازه)

/** Boot-time virtual start address of the kernel image. */
#تعریف_کړه K_MEM_KERNEL_VIRT_پیل	((uint8_t *)&z_mapped_پیل[0])

/** Boot-time virtual end address of the kernel image. */
#تعریف_کړه K_MEM_KERNEL_VIRT_پای_ټکی	((uint8_t *)&z_mapped_پای_ټکی[0])

/** Boot-time virtual address space size of the kernel image. */
#تعریف_کړه K_MEM_KERNEL_VIRT_اندازه	(K_MEM_KERNEL_VIRT_پای_ټکی - K_MEM_KERNEL_VIRT_پیل)

/**
 * @brief Offset for translating between static physical and virtual addresses.
 *
 * @note Do not use directly unless you know exactly what you are going.
 */
#تعریف_کړه K_MEM_VM_دننه_واټن	\
	((CONFIG_KERNEL_VM_بنسټ + CONFIG_KERNEL_VM_دننه_واټن) - \
	 (K_MEM_PHYS_RAM_پیل + CONFIG_SRAM_دننه_واټن))

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
#تعریف_کړه K_MEM_BOOT_VIRT_منزل_PHYS(virt) ((uintptr_t)(((uint8_t *)(virt)) - K_MEM_VM_دننه_واټن))

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
#تعریف_کړه K_MEM_BOOT_PHYS_منزل_VIRT(phys) ((uint8_t *)(((uintptr_t)(phys)) + K_MEM_VM_دننه_واټن))

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
#که_تعریف_شوی CONFIG_ARCH_MAPS_ALL_RAM
#تعریف_کړه K_MEM_VM_FREE_پیل	K_MEM_BOOT_PHYS_منزل_VIRT(K_MEM_PHYS_RAM_پای_ټکی)
#وإلا
#تعریف_کړه K_MEM_VM_FREE_پیل	K_MEM_KERNEL_VIRT_پای_ټکی
#د_که_پای /* CONFIG_ARCH_MAPS_ALL_RAM */

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
#تعریف_کړه K_MEM_عدد_پاڼه_چوکاټونه	(K_MEM_PHYS_RAM_اندازه / (size_t)CONFIG_MMU_پاڼه_اندازه)

/*
 * k_mem_page_frame flags bits
 *
 * Requirements:
 * - K_MEM_PAGE_FRAME_FREE must be one of the possible sfnode flag bits
 * - All bit values must be lower than CONFIG_MMU_PAGE_SIZE
 */

/** This physical page is free and part of the free list */
#تعریف_کړه K_MEM_پاڼه_چوکاټ_FREE		بټ(0)

/** This physical page is reserved by hardware; we will never use it */
#تعریف_کړه K_MEM_پاڼه_چوکاټ_RESERVED	بټ(1)

/** This page contains critical kernel data and will never be swapped */
#تعریف_کړه K_MEM_پاڼه_چوکاټ_PINNED		بټ(2)

/**
 * This physical page is mapped to some virtual memory address
 *
 * Currently, we just support one mapping per page frame. If a page frame
 * is mapped to multiple virtual pages then it must be pinned.
 */
#تعریف_کړه K_MEM_پاڼه_چوکاټ_MAPPED		بټ(3)

/**
 * This page frame is currently involved in a page-in/out operation
 */
#تعریف_کړه K_MEM_پاڼه_چوکاټ_BUSY		بټ(4)

/**
 * This page frame has a clean copy in the backing store
 */
#تعریف_کړه K_MEM_پاڼه_چوکاټ_BACKED		بټ(5)

/**
 * Data structure for physical page frames
 *
 * An array of these is instantiated, one element per physical RAM page.
 * Hence it's necessary to constrain its size as much as possible.
 */
بنية k_mem_پاڼه_چوکاټ {
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
		sys_sfnode_t غوټه;
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
ساكن مضمن bool k_mem_پاڼه_چوکاټ_is_free(بنية k_mem_پاڼه_چوکاټ *pf)
{
	أرجع (pf->va_and_flags & K_MEM_پاڼه_چوکاټ_FREE) != 0U;
}

ساكن مضمن bool k_mem_پاڼه_چوکاټ_is_pinned(بنية k_mem_پاڼه_چوکاټ *pf)
{
	أرجع (pf->va_and_flags & K_MEM_پاڼه_چوکاټ_PINNED) != 0U;
}

ساكن مضمن bool k_mem_پاڼه_چوکاټ_is_reserved(بنية k_mem_پاڼه_چوکاټ *pf)
{
	أرجع (pf->va_and_flags & K_MEM_پاڼه_چوکاټ_RESERVED) != 0U;
}

ساكن مضمن bool k_mem_پاڼه_چوکاټ_is_mapped(بنية k_mem_پاڼه_چوکاټ *pf)
{
	أرجع (pf->va_and_flags & K_MEM_پاڼه_چوکاټ_MAPPED) != 0U;
}

ساكن مضمن bool k_mem_پاڼه_چوکاټ_is_busy(بنية k_mem_پاڼه_چوکاټ *pf)
{
	أرجع (pf->va_and_flags & K_MEM_پاڼه_چوکاټ_BUSY) != 0U;
}

ساكن مضمن bool k_mem_پاڼه_چوکاټ_is_backed(بنية k_mem_پاڼه_چوکاټ *pf)
{
	أرجع (pf->va_and_flags & K_MEM_پاڼه_چوکاټ_BACKED) != 0U;
}

ساكن مضمن bool k_mem_پاڼه_چوکاټ_is_evictable(بنية k_mem_پاڼه_چوکاټ *pf)
{
	أرجع (!k_mem_پاڼه_چوکاټ_is_free(pf) &&
		!k_mem_پاڼه_چوکاټ_is_reserved(pf) &&
		k_mem_پاڼه_چوکاټ_is_mapped(pf) &&
		!k_mem_پاڼه_چوکاټ_is_pinned(pf) &&
		!k_mem_پاڼه_چوکاټ_is_busy(pf));
}

/* If true, page is not being used for anything, is not reserved, is not
 * a member of some free pages list, isn't busy, and is ready to be mapped
 * in memory
 */
ساكن مضمن bool k_mem_پاڼه_چوکاټ_is_شته(بنية k_mem_پاڼه_چوکاټ *پاڼه)
{
	أرجع پاڼه->va_and_flags == 0U;
}

ساكن مضمن فراغ k_mem_پاڼه_چوکاټ_سېټ(بنية k_mem_پاڼه_چوکاټ *pf, uint8_t flags)
{
	pf->va_and_flags |= flags;
}

ساكن مضمن فراغ k_mem_پاڼه_چوکاټ_clear(بنية k_mem_پاڼه_چوکاټ *pf, uint8_t flags)
{
	/* ensure bit inversion to follow is done on the proper type width */
	uintptr_t wide_flags = flags;

	pf->va_and_flags &= ~wide_flags;
}

ساكن مضمن فراغ k_mem_assert_phys_aligned(uintptr_t phys)
{
	__ASSERT(phys % CONFIG_MMU_پاڼه_اندازه == 0U,
		 "physical address 0x%lx is not page-aligned", phys);
	(فراغ)phys;
}

خارجي بنية k_mem_پاڼه_چوکاټ k_mem_پاڼه_چوکاټونه[K_MEM_عدد_پاڼه_چوکاټونه];

ساكن مضمن uintptr_t k_mem_پاڼه_چوکاټ_منزل_phys(بنية k_mem_پاڼه_چوکاټ *pf)
{
	أرجع (uintptr_t)((pf - k_mem_پاڼه_چوکاټونه) * CONFIG_MMU_پاڼه_اندازه) +
			  K_MEM_PHYS_RAM_پیل;
}

/* Presumes there is but one mapping in the virtual address space */
ساكن مضمن فراغ *k_mem_پاڼه_چوکاټ_منزل_virt(بنية k_mem_پاڼه_چوکاټ *pf)
{
	uintptr_t flags_ماسک = CONFIG_MMU_پاڼه_اندازه - 1;

	أرجع (فراغ *)(pf->va_and_flags & ~flags_ماسک);
}

ساكن مضمن bool k_mem_is_پاڼه_چوکاټ(uintptr_t phys)
{
	k_mem_assert_phys_aligned(phys);
	أرجع دننه_RANGE(phys, (uintptr_t)K_MEM_PHYS_RAM_پیل,
			(uintptr_t)(K_MEM_PHYS_RAM_پای_ټکی - 1));
}

ساكن مضمن بنية k_mem_پاڼه_چوکاټ *k_mem_phys_منزل_پاڼه_چوکاټ(uintptr_t phys)
{
	__ASSERT(k_mem_is_پاڼه_چوکاټ(phys),
		 "0x%lx not an SRAM physical address", phys);

	أرجع &k_mem_پاڼه_چوکاټونه[(phys - K_MEM_PHYS_RAM_پیل) /
				  CONFIG_MMU_پاڼه_اندازه];
}

ساكن مضمن فراغ k_mem_assert_virtual_region(uint8_t *addr, size_t اندازه)
{
	__ASSERT((uintptr_t)addr % CONFIG_MMU_پاڼه_اندازه == 0U,
		 "unaligned addr %p", addr);
	__ASSERT(اندازه % CONFIG_MMU_پاڼه_اندازه == 0U,
		 "unaligned size %zu", اندازه);
	__ASSERT(!Z_DETECT_اشاره_ګر_ډکېدنه(addr, اندازه),
		 "region %p size %zu zero or wraps around", addr, اندازه);
	__ASSERT(دننه_RANGE((uintptr_t)addr,
			  (uintptr_t)K_MEM_VIRT_RAM_پیل,
			  ((uintptr_t)K_MEM_VIRT_RAM_پای_ټکی - 1)) &&
		 دننه_RANGE(((uintptr_t)addr + اندازه - 1),
			  (uintptr_t)K_MEM_VIRT_RAM_پیل,
			  ((uintptr_t)K_MEM_VIRT_RAM_پای_ټکی - 1)),
		 "invalid virtual address region %p (%zu)", addr, اندازه);
}

/**
 * @brief Pretty-print page frame information for all page frames.
 *
 * Debug function, pretty-print page frame information for all frames
 * concisely to printk.
 */
فراغ k_mem_پاڼه_چوکاټونه_dump(فراغ);

/* Convenience macro for iterating over all page frames */
#تعریف_کړه K_MEM_پاڼه_چوکاټ_FOREACH(_phys, _pageframe) \
	لكل ((_phys) = K_MEM_PHYS_RAM_پیل, (_pageframe) = k_mem_پاڼه_چوکاټونه; \
	     (_phys) < K_MEM_PHYS_RAM_پای_ټکی; \
	     (_phys) += CONFIG_MMU_پاڼه_اندازه, (_pageframe)++)

/** @} */

/**
 * @def K_MEM_VM_RESERVED
 * @brief Reserve space at the end of virtual memory.
 */
#که_تعریف_شوی CONFIG_DEMAND_PAGING
/* We reserve a virtual page as a scratch area for page-ins/outs at the end
 * of the address space
 */
#تعریف_کړه K_MEM_VM_RESERVED	CONFIG_MMU_پاڼه_اندازه

/**
 * @brief Location of the scratch page used for demand paging.
 */
#تعریف_کړه K_MEM_SCRATCH_پاڼه	((فراغ *)((uintptr_t)CONFIG_KERNEL_VM_بنسټ + \
					  (uintptr_t)CONFIG_KERNEL_VM_اندازه - \
					  CONFIG_MMU_پاڼه_اندازه))
#وإلا
#تعریف_کړه K_MEM_VM_RESERVED	0
#د_که_پای /* CONFIG_DEMAND_PAGING */

#که_تعریف_شوی CONFIG_DEMAND_PAGING
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
بینښه اوږد k_mem_عدد_pagefaults_get(فراغ);

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
صحيح k_mem_پاڼه_چوکاټ_evict(uintptr_t phys);

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
bool k_mem_پاڼه_fault(فراغ *addr);

#د_که_پای /* CONFIG_DEMAND_PAGING */
#د_که_پای /* CONFIG_MMU */
#د_که_پای /* KERNEL_INCLUDE_MMU_H */
