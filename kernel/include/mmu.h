/*
 * Copyright (c) 2020 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#اگر_تعریف_نشده KERNEL_INCLUDE_MMU_H
#تعریف_کن KERNEL_INCLUDE_MMU_H

#اگر_تعریف_شده CONFIG_MMU

#ضمن <stdint.h>
#ضمن <zephyr/sys/sflist.h>
#ضمن <zephyr/sys/__assert.h>
#ضمن <zephyr/sys/util.h>
#ضمن <zephyr/kernel/mm.h>
#ضمن <zephyr/linker/linker-defs.h>
#ضمن <zephyr/devicetree.h>

/** Start address of physical memory. */
#تعریف_کن K_MEM_PHYS_RAM_شروع	((uintptr_t)DT_CHOSEN_SRAM_ADDR)

/** Size of physical memory. */
#تعریف_کن K_MEM_PHYS_RAM_اندازه	DT_CHOSEN_SRAM_اندازه

/** End address (exclusive) of physical memory. */
#تعریف_کن K_MEM_PHYS_RAM_انتها	(K_MEM_PHYS_RAM_شروع + K_MEM_PHYS_RAM_اندازه)

/** Start address of virtual memory. */
#تعریف_کن K_MEM_VIRT_RAM_شروع	((uint8_t *)CONFIG_KERNEL_VM_پایه)

/** Size of virtual memory. */
#تعریف_کن K_MEM_VIRT_RAM_اندازه	((size_t)CONFIG_KERNEL_VM_اندازه)

/** End address (exclusive) of virtual memory. */
#تعریف_کن K_MEM_VIRT_RAM_انتها	(K_MEM_VIRT_RAM_شروع + K_MEM_VIRT_RAM_اندازه)

/** Boot-time virtual start address of the kernel image. */
#تعریف_کن K_MEM_KERNEL_VIRT_شروع	((uint8_t *)&z_mapped_شروع[0])

/** Boot-time virtual end address of the kernel image. */
#تعریف_کن K_MEM_KERNEL_VIRT_انتها	((uint8_t *)&z_mapped_انتها[0])

/** Boot-time virtual address space size of the kernel image. */
#تعریف_کن K_MEM_KERNEL_VIRT_اندازه	(K_MEM_KERNEL_VIRT_انتها - K_MEM_KERNEL_VIRT_شروع)

/**
 * @brief Offset for translating between static physical and virtual addresses.
 *
 * @note Do not use directly unless you know exactly what you are going.
 */
#تعریف_کن K_MEM_VM_جابه_جایی	\
	((CONFIG_KERNEL_VM_پایه + CONFIG_KERNEL_VM_جابه_جایی) - \
	 (K_MEM_PHYS_RAM_شروع + CONFIG_SRAM_جابه_جایی))

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
#تعریف_کن K_MEM_BOOT_VIRT_مقصد_PHYS(virt) ((uintptr_t)(((uint8_t *)(virt)) - K_MEM_VM_جابه_جایی))

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
#تعریف_کن K_MEM_BOOT_PHYS_مقصد_VIRT(phys) ((uint8_t *)(((uintptr_t)(phys)) + K_MEM_VM_جابه_جایی))

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
#اگر_تعریف_شده CONFIG_ARCH_MAPS_ALL_RAM
#تعریف_کن K_MEM_VM_FREE_شروع	K_MEM_BOOT_PHYS_مقصد_VIRT(K_MEM_PHYS_RAM_انتها)
#وإلا
#تعریف_کن K_MEM_VM_FREE_شروع	K_MEM_KERNEL_VIRT_انتها
#پایان_اگر /* CONFIG_ARCH_MAPS_ALL_RAM */

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
#تعریف_کن K_MEM_رقم_صفحه_قابها	(K_MEM_PHYS_RAM_اندازه / (size_t)CONFIG_MMU_صفحه_اندازه)

/*
 * k_mem_page_frame flags bits
 *
 * Requirements:
 * - K_MEM_PAGE_FRAME_FREE must be one of the possible sfnode flag bits
 * - All bit values must be lower than CONFIG_MMU_PAGE_SIZE
 */

/** This physical page is free and part of the free list */
#تعریف_کن K_MEM_صفحه_قاب_FREE		بیت(0)

/** This physical page is reserved by hardware; we will never use it */
#تعریف_کن K_MEM_صفحه_قاب_RESERVED	بیت(1)

/** This page contains critical kernel data and will never be swapped */
#تعریف_کن K_MEM_صفحه_قاب_PINNED		بیت(2)

/**
 * This physical page is mapped to some virtual memory address
 *
 * Currently, we just support one mapping per page frame. If a page frame
 * is mapped to multiple virtual pages then it must be pinned.
 */
#تعریف_کن K_MEM_صفحه_قاب_MAPPED		بیت(3)

/**
 * This page frame is currently involved in a page-in/out operation
 */
#تعریف_کن K_MEM_صفحه_قاب_BUSY		بیت(4)

/**
 * This page frame has a clean copy in the backing store
 */
#تعریف_کن K_MEM_صفحه_قاب_BACKED		بیت(5)

/**
 * Data structure for physical page frames
 *
 * An array of these is instantiated, one element per physical RAM page.
 * Hence it's necessary to constrain its size as much as possible.
 */
بنية k_mem_صفحه_قاب {
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
		sys_sfnode_t گره;
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
ساكن مضمن bool k_mem_صفحه_قاب_is_free(بنية k_mem_صفحه_قاب *pf)
{
	أرجع (pf->va_and_flags & K_MEM_صفحه_قاب_FREE) != 0U;
}

ساكن مضمن bool k_mem_صفحه_قاب_is_pinned(بنية k_mem_صفحه_قاب *pf)
{
	أرجع (pf->va_and_flags & K_MEM_صفحه_قاب_PINNED) != 0U;
}

ساكن مضمن bool k_mem_صفحه_قاب_is_reserved(بنية k_mem_صفحه_قاب *pf)
{
	أرجع (pf->va_and_flags & K_MEM_صفحه_قاب_RESERVED) != 0U;
}

ساكن مضمن bool k_mem_صفحه_قاب_is_mapped(بنية k_mem_صفحه_قاب *pf)
{
	أرجع (pf->va_and_flags & K_MEM_صفحه_قاب_MAPPED) != 0U;
}

ساكن مضمن bool k_mem_صفحه_قاب_is_busy(بنية k_mem_صفحه_قاب *pf)
{
	أرجع (pf->va_and_flags & K_MEM_صفحه_قاب_BUSY) != 0U;
}

ساكن مضمن bool k_mem_صفحه_قاب_is_backed(بنية k_mem_صفحه_قاب *pf)
{
	أرجع (pf->va_and_flags & K_MEM_صفحه_قاب_BACKED) != 0U;
}

ساكن مضمن bool k_mem_صفحه_قاب_is_evictable(بنية k_mem_صفحه_قاب *pf)
{
	أرجع (!k_mem_صفحه_قاب_is_free(pf) &&
		!k_mem_صفحه_قاب_is_reserved(pf) &&
		k_mem_صفحه_قاب_is_mapped(pf) &&
		!k_mem_صفحه_قاب_is_pinned(pf) &&
		!k_mem_صفحه_قاب_is_busy(pf));
}

/* If true, page is not being used for anything, is not reserved, is not
 * a member of some free pages list, isn't busy, and is ready to be mapped
 * in memory
 */
ساكن مضمن bool k_mem_صفحه_قاب_is_موجود(بنية k_mem_صفحه_قاب *صفحه)
{
	أرجع صفحه->va_and_flags == 0U;
}

ساكن مضمن فراغ k_mem_صفحه_قاب_مجموعه(بنية k_mem_صفحه_قاب *pf, uint8_t flags)
{
	pf->va_and_flags |= flags;
}

ساكن مضمن فراغ k_mem_صفحه_قاب_clear(بنية k_mem_صفحه_قاب *pf, uint8_t flags)
{
	/* ensure bit inversion to follow is done on the proper type width */
	uintptr_t wide_flags = flags;

	pf->va_and_flags &= ~wide_flags;
}

ساكن مضمن فراغ k_mem_assert_phys_aligned(uintptr_t phys)
{
	__ASSERT(phys % CONFIG_MMU_صفحه_اندازه == 0U,
		 "physical address 0x%lx is not page-aligned", phys);
	(فراغ)phys;
}

خارجي بنية k_mem_صفحه_قاب k_mem_صفحه_قابها[K_MEM_رقم_صفحه_قابها];

ساكن مضمن uintptr_t k_mem_صفحه_قاب_مقصد_phys(بنية k_mem_صفحه_قاب *pf)
{
	أرجع (uintptr_t)((pf - k_mem_صفحه_قابها) * CONFIG_MMU_صفحه_اندازه) +
			  K_MEM_PHYS_RAM_شروع;
}

/* Presumes there is but one mapping in the virtual address space */
ساكن مضمن فراغ *k_mem_صفحه_قاب_مقصد_virt(بنية k_mem_صفحه_قاب *pf)
{
	uintptr_t flags_ماسک = CONFIG_MMU_صفحه_اندازه - 1;

	أرجع (فراغ *)(pf->va_and_flags & ~flags_ماسک);
}

ساكن مضمن bool k_mem_is_صفحه_قاب(uintptr_t phys)
{
	k_mem_assert_phys_aligned(phys);
	أرجع درون_RANGE(phys, (uintptr_t)K_MEM_PHYS_RAM_شروع,
			(uintptr_t)(K_MEM_PHYS_RAM_انتها - 1));
}

ساكن مضمن بنية k_mem_صفحه_قاب *k_mem_phys_مقصد_صفحه_قاب(uintptr_t phys)
{
	__ASSERT(k_mem_is_صفحه_قاب(phys),
		 "0x%lx not an SRAM physical address", phys);

	أرجع &k_mem_صفحه_قابها[(phys - K_MEM_PHYS_RAM_شروع) /
				  CONFIG_MMU_صفحه_اندازه];
}

ساكن مضمن فراغ k_mem_assert_virtual_region(uint8_t *addr, size_t اندازه)
{
	__ASSERT((uintptr_t)addr % CONFIG_MMU_صفحه_اندازه == 0U,
		 "unaligned addr %p", addr);
	__ASSERT(اندازه % CONFIG_MMU_صفحه_اندازه == 0U,
		 "unaligned size %zu", اندازه);
	__ASSERT(!Z_DETECT_اشاره_گر_سرریز(addr, اندازه),
		 "region %p size %zu zero or wraps around", addr, اندازه);
	__ASSERT(درون_RANGE((uintptr_t)addr,
			  (uintptr_t)K_MEM_VIRT_RAM_شروع,
			  ((uintptr_t)K_MEM_VIRT_RAM_انتها - 1)) &&
		 درون_RANGE(((uintptr_t)addr + اندازه - 1),
			  (uintptr_t)K_MEM_VIRT_RAM_شروع,
			  ((uintptr_t)K_MEM_VIRT_RAM_انتها - 1)),
		 "invalid virtual address region %p (%zu)", addr, اندازه);
}

/**
 * @brief Pretty-print page frame information for all page frames.
 *
 * Debug function, pretty-print page frame information for all frames
 * concisely to printk.
 */
فراغ k_mem_صفحه_قابها_dump(فراغ);

/* Convenience macro for iterating over all page frames */
#تعریف_کن K_MEM_صفحه_قاب_FOREACH(_phys, _pageframe) \
	لكل ((_phys) = K_MEM_PHYS_RAM_شروع, (_pageframe) = k_mem_صفحه_قابها; \
	     (_phys) < K_MEM_PHYS_RAM_انتها; \
	     (_phys) += CONFIG_MMU_صفحه_اندازه, (_pageframe)++)

/** @} */

/**
 * @def K_MEM_VM_RESERVED
 * @brief Reserve space at the end of virtual memory.
 */
#اگر_تعریف_شده CONFIG_DEMAND_PAGING
/* We reserve a virtual page as a scratch area for page-ins/outs at the end
 * of the address space
 */
#تعریف_کن K_MEM_VM_RESERVED	CONFIG_MMU_صفحه_اندازه

/**
 * @brief Location of the scratch page used for demand paging.
 */
#تعریف_کن K_MEM_SCRATCH_صفحه	((فراغ *)((uintptr_t)CONFIG_KERNEL_VM_پایه + \
					  (uintptr_t)CONFIG_KERNEL_VM_اندازه - \
					  CONFIG_MMU_صفحه_اندازه))
#وإلا
#تعریف_کن K_MEM_VM_RESERVED	0
#پایان_اگر /* CONFIG_DEMAND_PAGING */

#اگر_تعریف_شده CONFIG_DEMAND_PAGING
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
بیعلامت دراز k_mem_رقم_pagefaults_get(فراغ);

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
صحيح k_mem_صفحه_قاب_evict(uintptr_t phys);

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
bool k_mem_صفحه_fault(فراغ *addr);

#پایان_اگر /* CONFIG_DEMAND_PAGING */
#پایان_اگر /* CONFIG_MMU */
#پایان_اگر /* KERNEL_INCLUDE_MMU_H */
