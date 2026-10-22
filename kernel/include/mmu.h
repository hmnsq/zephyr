/*
 * Copyright (c) 2020 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#నిర్వచితం_కాకపోతే KERNEL_INCLUDE_MMU_H
#నిర్వచించు KERNEL_INCLUDE_MMU_H

#నిర్వచితమైతే CONFIG_MMU

#అన్తర్భావ <stdint.h>
#అన్తర్భావ <zephyr/sys/sflist.h>
#అన్తర్భావ <zephyr/sys/__assert.h>
#అన్తర్భావ <zephyr/sys/util.h>
#అన్తర్భావ <zephyr/kernel/mm.h>
#అన్తర్భావ <zephyr/linker/linker-defs.h>
#అన్తర్భావ <zephyr/devicetree.h>

/** Start address of physical memory. */
#నిర్వచించు K_MEM_PHYS_RAM_ప్రారంభం	((uintptr_t)DT_CHOSEN_SRAM_ADDR)

/** Size of physical memory. */
#నిర్వచించు K_MEM_PHYS_RAM_కొలత	DT_CHOSEN_SRAM_కొలత

/** End address (exclusive) of physical memory. */
#నిర్వచించు K_MEM_PHYS_RAM_చివర	(K_MEM_PHYS_RAM_ప్రారంభం + K_MEM_PHYS_RAM_కొలత)

/** Start address of virtual memory. */
#నిర్వచించు K_MEM_VIRT_RAM_ప్రారంభం	((uint8_t *)CONFIG_KERNEL_VM_పునాది)

/** Size of virtual memory. */
#నిర్వచించు K_MEM_VIRT_RAM_కొలత	((size_t)CONFIG_KERNEL_VM_కొలత)

/** End address (exclusive) of virtual memory. */
#నిర్వచించు K_MEM_VIRT_RAM_చివర	(K_MEM_VIRT_RAM_ప్రారంభం + K_MEM_VIRT_RAM_కొలత)

/** Boot-time virtual start address of the kernel image. */
#నిర్వచించు K_MEM_KERNEL_VIRT_ప్రారంభం	((uint8_t *)&z_mapped_ప్రారంభం[0])

/** Boot-time virtual end address of the kernel image. */
#నిర్వచించు K_MEM_KERNEL_VIRT_చివర	((uint8_t *)&z_mapped_చివర[0])

/** Boot-time virtual address space size of the kernel image. */
#నిర్వచించు K_MEM_KERNEL_VIRT_కొలత	(K_MEM_KERNEL_VIRT_చివర - K_MEM_KERNEL_VIRT_ప్రారంభం)

/**
 * @brief Offset for translating between static physical and virtual addresses.
 *
 * @note Do not use directly unless you know exactly what you are going.
 */
#నిర్వచించు K_MEM_VM_స్థానభ్రంశం	\
	((CONFIG_KERNEL_VM_పునాది + CONFIG_KERNEL_VM_స్థానభ్రంశం) - \
	 (K_MEM_PHYS_RAM_ప్రారంభం + CONFIG_SRAM_స్థానభ్రంశం))

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
#నిర్వచించు K_MEM_BOOT_VIRT_గమ్యం_PHYS(virt) ((uintptr_t)(((uint8_t *)(virt)) - K_MEM_VM_స్థానభ్రంశం))

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
#నిర్వచించు K_MEM_BOOT_PHYS_గమ్యం_VIRT(phys) ((uint8_t *)(((uintptr_t)(phys)) + K_MEM_VM_స్థానభ్రంశం))

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
#నిర్వచితమైతే CONFIG_ARCH_MAPS_ALL_RAM
#నిర్వచించు K_MEM_VM_FREE_ప్రారంభం	K_MEM_BOOT_PHYS_గమ్యం_VIRT(K_MEM_PHYS_RAM_చివర)
#లేకపోతే
#నిర్వచించు K_MEM_VM_FREE_ప్రారంభం	K_MEM_KERNEL_VIRT_చివర
#అయితే_ముగింపు /* CONFIG_ARCH_MAPS_ALL_RAM */

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
#నిర్వచించు K_MEM_అంకె_పుట_చట్రాలు	(K_MEM_PHYS_RAM_కొలత / (size_t)CONFIG_MMU_పుట_కొలత)

/*
 * k_mem_page_frame flags bits
 *
 * Requirements:
 * - K_MEM_PAGE_FRAME_FREE must be one of the possible sfnode flag bits
 * - All bit values must be lower than CONFIG_MMU_PAGE_SIZE
 */

/** This physical page is free and part of the free list */
#నిర్వచించు K_MEM_పుట_చట్రం_FREE		బిట్టు(0)

/** This physical page is reserved by hardware; we will never use it */
#నిర్వచించు K_MEM_పుట_చట్రం_RESERVED	బిట్టు(1)

/** This page contains critical kernel data and will never be swapped */
#నిర్వచించు K_MEM_పుట_చట్రం_PINNED		బిట్టు(2)

/**
 * This physical page is mapped to some virtual memory address
 *
 * Currently, we just support one mapping per page frame. If a page frame
 * is mapped to multiple virtual pages then it must be pinned.
 */
#నిర్వచించు K_MEM_పుట_చట్రం_MAPPED		బిట్టు(3)

/**
 * This page frame is currently involved in a page-in/out operation
 */
#నిర్వచించు K_MEM_పుట_చట్రం_BUSY		బిట్టు(4)

/**
 * This page frame has a clean copy in the backing store
 */
#నిర్వచించు K_MEM_పుట_చట్రం_BACKED		బిట్టు(5)

/**
 * Data structure for physical page frames
 *
 * An array of these is instantiated, one element per physical RAM page.
 * Hence it's necessary to constrain its size as much as possible.
 */
నిర్మాణం k_mem_పుట_చట్రం {
	సంఘం {
		/*
		 * If mapped, K_MEM_PAGE_FRAME_* flags and virtual address
		 * this page is mapped to.
		 */
		uintptr_t va_and_flags;

		/*
		 * If unmapped and available, free pages list membership
		 * with the K_MEM_PAGE_FRAME_FREE flag.
		 */
		sys_sfnode_t కణుపు;
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
స్థిర అంతఃవరుస bool k_mem_పుట_చట్రం_is_free(నిర్మాణం k_mem_పుట_చట్రం *pf)
{
	తిరిగి (pf->va_and_flags & K_MEM_పుట_చట్రం_FREE) != 0U;
}

స్థిర అంతఃవరుస bool k_mem_పుట_చట్రం_is_pinned(నిర్మాణం k_mem_పుట_చట్రం *pf)
{
	తిరిగి (pf->va_and_flags & K_MEM_పుట_చట్రం_PINNED) != 0U;
}

స్థిర అంతఃవరుస bool k_mem_పుట_చట్రం_is_reserved(నిర్మాణం k_mem_పుట_చట్రం *pf)
{
	తిరిగి (pf->va_and_flags & K_MEM_పుట_చట్రం_RESERVED) != 0U;
}

స్థిర అంతఃవరుస bool k_mem_పుట_చట్రం_is_mapped(నిర్మాణం k_mem_పుట_చట్రం *pf)
{
	తిరిగి (pf->va_and_flags & K_MEM_పుట_చట్రం_MAPPED) != 0U;
}

స్థిర అంతఃవరుస bool k_mem_పుట_చట్రం_is_busy(నిర్మాణం k_mem_పుట_చట్రం *pf)
{
	తిరిగి (pf->va_and_flags & K_MEM_పుట_చట్రం_BUSY) != 0U;
}

స్థిర అంతఃవరుస bool k_mem_పుట_చట్రం_is_backed(నిర్మాణం k_mem_పుట_చట్రం *pf)
{
	తిరిగి (pf->va_and_flags & K_MEM_పుట_చట్రం_BACKED) != 0U;
}

స్థిర అంతఃవరుస bool k_mem_పుట_చట్రం_is_evictable(నిర్మాణం k_mem_పుట_చట్రం *pf)
{
	తిరిగి (!k_mem_పుట_చట్రం_is_free(pf) &&
		!k_mem_పుట_చట్రం_is_reserved(pf) &&
		k_mem_పుట_చట్రం_is_mapped(pf) &&
		!k_mem_పుట_చట్రం_is_pinned(pf) &&
		!k_mem_పుట_చట్రం_is_busy(pf));
}

/* If true, page is not being used for anything, is not reserved, is not
 * a member of some free pages list, isn't busy, and is ready to be mapped
 * in memory
 */
స్థిర అంతఃవరుస bool k_mem_పుట_చట్రం_is_అందుబాటు(నిర్మాణం k_mem_పుట_చట్రం *పుట)
{
	తిరిగి పుట->va_and_flags == 0U;
}

స్థిర అంతఃవరుస శూన్య k_mem_పుట_చట్రం_సమితి(నిర్మాణం k_mem_పుట_చట్రం *pf, uint8_t flags)
{
	pf->va_and_flags |= flags;
}

స్థిర అంతఃవరుస శూన్య k_mem_పుట_చట్రం_clear(నిర్మాణం k_mem_పుట_చట్రం *pf, uint8_t flags)
{
	/* ensure bit inversion to follow is done on the proper type width */
	uintptr_t wide_flags = flags;

	pf->va_and_flags &= ~wide_flags;
}

స్థిర అంతఃవరుస శూన్య k_mem_assert_phys_aligned(uintptr_t phys)
{
	__ASSERT(phys % CONFIG_MMU_పుట_కొలత == 0U,
		 "physical address 0x%lx is not page-aligned", phys);
	(శూన్య)phys;
}

బాహ్య నిర్మాణం k_mem_పుట_చట్రం k_mem_పుట_చట్రాలు[K_MEM_అంకె_పుట_చట్రాలు];

స్థిర అంతఃవరుస uintptr_t k_mem_పుట_చట్రం_గమ్యం_phys(నిర్మాణం k_mem_పుట_చట్రం *pf)
{
	తిరిగి (uintptr_t)((pf - k_mem_పుట_చట్రాలు) * CONFIG_MMU_పుట_కొలత) +
			  K_MEM_PHYS_RAM_ప్రారంభం;
}

/* Presumes there is but one mapping in the virtual address space */
స్థిర అంతఃవరుస శూన్య *k_mem_పుట_చట్రం_గమ్యం_virt(నిర్మాణం k_mem_పుట_చట్రం *pf)
{
	uintptr_t flags_ముసుగు = CONFIG_MMU_పుట_కొలత - 1;

	తిరిగి (శూన్య *)(pf->va_and_flags & ~flags_ముసుగు);
}

స్థిర అంతఃవరుస bool k_mem_is_పుట_చట్రం(uintptr_t phys)
{
	k_mem_assert_phys_aligned(phys);
	తిరిగి లోపల_RANGE(phys, (uintptr_t)K_MEM_PHYS_RAM_ప్రారంభం,
			(uintptr_t)(K_MEM_PHYS_RAM_చివర - 1));
}

స్థిర అంతఃవరుస నిర్మాణం k_mem_పుట_చట్రం *k_mem_phys_గమ్యం_పుట_చట్రం(uintptr_t phys)
{
	__ASSERT(k_mem_is_పుట_చట్రం(phys),
		 "0x%lx not an SRAM physical address", phys);

	తిరిగి &k_mem_పుట_చట్రాలు[(phys - K_MEM_PHYS_RAM_ప్రారంభం) /
				  CONFIG_MMU_పుట_కొలత];
}

స్థిర అంతఃవరుస శూన్య k_mem_assert_virtual_region(uint8_t *addr, size_t కొలత)
{
	__ASSERT((uintptr_t)addr % CONFIG_MMU_పుట_కొలత == 0U,
		 "unaligned addr %p", addr);
	__ASSERT(కొలత % CONFIG_MMU_పుట_కొలత == 0U,
		 "unaligned size %zu", కొలత);
	__ASSERT(!Z_DETECT_సూచిక_అధికప్రవాహం(addr, కొలత),
		 "region %p size %zu zero or wraps around", addr, కొలత);
	__ASSERT(లోపల_RANGE((uintptr_t)addr,
			  (uintptr_t)K_MEM_VIRT_RAM_ప్రారంభం,
			  ((uintptr_t)K_MEM_VIRT_RAM_చివర - 1)) &&
		 లోపల_RANGE(((uintptr_t)addr + కొలత - 1),
			  (uintptr_t)K_MEM_VIRT_RAM_ప్రారంభం,
			  ((uintptr_t)K_MEM_VIRT_RAM_చివర - 1)),
		 "invalid virtual address region %p (%zu)", addr, కొలత);
}

/**
 * @brief Pretty-print page frame information for all page frames.
 *
 * Debug function, pretty-print page frame information for all frames
 * concisely to printk.
 */
శూన్య k_mem_పుట_చట్రాలు_dump(శూన్య);

/* Convenience macro for iterating over all page frames */
#నిర్వచించు K_MEM_పుట_చట్రం_FOREACH(_phys, _pageframe) \
	కొరకు ((_phys) = K_MEM_PHYS_RAM_ప్రారంభం, (_pageframe) = k_mem_పుట_చట్రాలు; \
	     (_phys) < K_MEM_PHYS_RAM_చివర; \
	     (_phys) += CONFIG_MMU_పుట_కొలత, (_pageframe)++)

/** @} */

/**
 * @def K_MEM_VM_RESERVED
 * @brief Reserve space at the end of virtual memory.
 */
#నిర్వచితమైతే CONFIG_DEMAND_PAGING
/* We reserve a virtual page as a scratch area for page-ins/outs at the end
 * of the address space
 */
#నిర్వచించు K_MEM_VM_RESERVED	CONFIG_MMU_పుట_కొలత

/**
 * @brief Location of the scratch page used for demand paging.
 */
#నిర్వచించు K_MEM_SCRATCH_పుట	((శూన్య *)((uintptr_t)CONFIG_KERNEL_VM_పునాది + \
					  (uintptr_t)CONFIG_KERNEL_VM_కొలత - \
					  CONFIG_MMU_పుట_కొలత))
#లేకపోతే
#నిర్వచించు K_MEM_VM_RESERVED	0
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING */

#నిర్వచితమైతే CONFIG_DEMAND_PAGING
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
గుర్తులేని దీర్ఘ k_mem_అంకె_pagefaults_get(శూన్య);

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
పూర్ణాంకం k_mem_పుట_చట్రం_evict(uintptr_t phys);

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
bool k_mem_పుట_fault(శూన్య *addr);

#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING */
#అయితే_ముగింపు /* CONFIG_MMU */
#అయితే_ముగింపు /* KERNEL_INCLUDE_MMU_H */
