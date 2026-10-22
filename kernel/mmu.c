/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Routines for managing virtual address spaces
 */

#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <kernel_arch_interface.h>
#ಅನ್ತರ್ಭಾವ <zephyr/spinlock.h>
#ಅನ್ತರ್ಭಾವ <mmu.h>
#ಅನ್ತರ್ಭಾವ <zephyr/init.h>
#ಅನ್ತರ್ಭಾವ <kernel_internal.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>
#ಅನ್ತರ್ಭಾವ <zephyr/toolchain.h>
#ಅನ್ತರ್ಭಾವ <zephyr/linker/linker-defs.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/bitarray.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/check.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/math_extras.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/minmax.h>
#ಅನ್ತರ್ಭಾವ <zephyr/timing/timing.h>
#ಅನ್ತರ್ಭಾವ <zephyr/arch/common/init.h>
#ಅನ್ತರ್ಭಾವ <zephyr/logging/log.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING
#ಅನ್ತರ್ಭಾವ <zephyr/kernel/mm/demand_paging.h>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING */

/*
 * General terminology:
 * - A page frame is a page-sized physical memory region in RAM. It is a
 *   container where a data page may be placed. It is always referred to by
 *   physical address. We have a convention of using uintptr_t for physical
 *   addresses. We instantiate a struct k_mem_page_frame to store metadata for
 *   every page frame.
 *
 * - A data page is a page-sized region of data. It may exist in a page frame,
 *   or be paged out to some backing store. Its location can always be looked
 *   up in the CPU's page tables (or equivalent) by virtual address.
 *   The data type will always be void * or in some cases uint8_t * when we
 *   want to do pointer arithmetic.
 */

/* Spinlock to protect any globals in this file and serialize page table
 * updates in arch code
 */
ರಚನೆ k_spinlock z_mm_ಬೀಗ;

/*
 * General page frame management
 */

/* Database of all RAM page frames */
ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು k_mem_ಪುಟ_ಚೌಕಟ್ಟುಗಳು[K_MEM_ಅಂಕೆ_ಪುಟ_ಚೌಕಟ್ಟುಗಳು];

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ASSERT
/* Indicator that k_mem_page_frames has been initialized, many of these APIs do
 * not work before POST_KERNEL
 */
ಸ್ಥಿರ bool ಪುಟ_ಚೌಕಟ್ಟುಗಳು_initialized;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_ASSERT */

/* Add colors to page table dumps to indicate mapping type */
#ವ್ಯಾಖ್ಯಾನಿಸು COLOR_ಪುಟ_ಚೌಕಟ್ಟುಗಳು	1

#ಆದರೆ COLOR_ಪುಟ_ಚೌಕಟ್ಟುಗಳು
#ವ್ಯಾಖ್ಯಾನಿಸು ANSI_DEFAULT "\x1B" "[0m"
#ವ್ಯಾಖ್ಯಾನಿಸು ANSI_RED     "\x1B" "[1;31m"
#ವ್ಯಾಖ್ಯಾನಿಸು ANSI_GREEN   "\x1B" "[1;32m"
#ವ್ಯಾಖ್ಯಾನಿಸು ANSI_YELLOW  "\x1B" "[1;33m"
#ವ್ಯಾಖ್ಯಾನಿಸು ANSI_BLUE    "\x1B" "[1;34m"
#ವ್ಯಾಖ್ಯಾನಿಸು ANSI_MAGENTA "\x1B" "[1;35m"
#ವ್ಯಾಖ್ಯಾನಿಸು ANSI_CYAN    "\x1B" "[1;36m"
#ವ್ಯಾಖ್ಯಾನಿಸು ANSI_GREY    "\x1B" "[1;90m"

#ವ್ಯಾಖ್ಯಾನಿಸು COLOR(x)	printk(_CONCAT(ANSI_, x))
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು COLOR(x)	ಮಾಡು { } ಆಗ (false)
#ಆದರೆ_ಅಂತ್ಯ /* COLOR_PAGE_FRAMES */

/* LCOV_EXCL_START */
ಸ್ಥಿರ ಶೂನ್ಯ ಪುಟ_ಚೌಕಟ್ಟು_dump(ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *pf)
{
	ಆದರೆ (k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_free(pf)) {
		COLOR(GREY);
		printk("-");
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_reserved(pf)) {
		COLOR(CYAN);
		printk("R");
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_busy(pf)) {
		COLOR(MAGENTA);
		printk("B");
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_pinned(pf)) {
		COLOR(YELLOW);
		printk("P");
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_ಲಭ್ಯ(pf)) {
		COLOR(GREY);
		printk(".");
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_mapped(pf)) {
		COLOR(DEFAULT);
		printk("M");
	} ಇಲ್ಲದಿದ್ದರೆ {
		COLOR(RED);
		printk("?");
	}
}

ಶೂನ್ಯ k_mem_ಪುಟ_ಚೌಕಟ್ಟುಗಳು_dump(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ ಸ್ತಂಭ = 0;

	__ASSERT(ಪುಟ_ಚೌಕಟ್ಟುಗಳು_initialized, "%s called too early", __func__);
	printk("Physical memory from 0x%lx to 0x%lx\n",
	       K_MEM_PHYS_RAM_ಆರಂಭ, K_MEM_PHYS_RAM_ಕೊನೆ);

	ಗಾಗಿ (ಪೂರ್ಣಾಂಕ i = 0; i < K_MEM_ಅಂಕೆ_ಪುಟ_ಚೌಕಟ್ಟುಗಳು; i++) {
		ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *pf = &k_mem_ಪುಟ_ಚೌಕಟ್ಟುಗಳು[i];

		ಪುಟ_ಚೌಕಟ್ಟು_dump(pf);

		ಸ್ತಂಭ++;
		ಆದರೆ (ಸ್ತಂಭ == 64) {
			ಸ್ತಂಭ = 0;
			printk("\n");
		}
	}

	COLOR(DEFAULT);
	ಆದರೆ (ಸ್ತಂಭ != 0) {
		printk("\n");
	}
}
/* LCOV_EXCL_STOP */

#ವ್ಯಾಖ್ಯಾನಿಸು VIRT_FOREACH(ತಳ_2, ಗಾತ್ರ_3, _pos) \
	ಗಾಗಿ ((_pos) = (ತಳ_2); \
	     (_pos) < ((uint8_t *)(ತಳ_2) + (ಗಾತ್ರ_3)); (_pos) += CONFIG_MMU_ಪುಟ_ಗಾತ್ರ)

#ವ್ಯಾಖ್ಯಾನಿಸು PHYS_FOREACH(ತಳ_2, ಗಾತ್ರ_3, _pos) \
	ಗಾಗಿ ((_pos) = (ತಳ_2); \
	     (_pos) < ((uintptr_t)(ತಳ_2) + (ಗಾತ್ರ_3)); (_pos) += CONFIG_MMU_ಪುಟ_ಗಾತ್ರ)


/*
 * Virtual address space management
 *
 * Call all of these functions with z_mm_lock held.
 *
 * Overall virtual memory map: When the kernel starts, it resides in
 * virtual memory in the region K_MEM_KERNEL_VIRT_START to
 * K_MEM_KERNEL_VIRT_END. Unused virtual memory past this, up to the limit
 * noted by CONFIG_KERNEL_VM_SIZE may be used for runtime memory mappings.
 *
 * If CONFIG_ARCH_MAPS_ALL_RAM is set, we do not just map the kernel image,
 * but have a mapping for all RAM in place. This is for special architectural
 * purposes and does not otherwise affect page frame accounting or flags;
 * the only guarantee is that such RAM mapping outside of the Zephyr image
 * won't be disturbed by subsequent memory mapping calls.
 *
 * +--------------+ <- K_MEM_VIRT_RAM_START
 * | Undefined VM | <- May contain ancillary regions like x86_64's locore
 * +--------------+ <- K_MEM_KERNEL_VIRT_START (often == K_MEM_VIRT_RAM_START)
 * | Mapping for  |
 * | main kernel  |
 * | image        |
 * |		  |
 * |		  |
 * +--------------+ <- K_MEM_VM_FREE_START
 * |              |
 * | Unused,      |
 * | Available VM |
 * |              |
 * |..............| <- mapping_pos (grows downward as more mappings are made)
 * | Mapping      |
 * +--------------+
 * | Mapping      |
 * +--------------+
 * | ...          |
 * +--------------+
 * | Mapping      |
 * +--------------+ <- mappings start here
 * | Reserved     | <- special purpose virtual page(s) of size K_MEM_VM_RESERVED
 * +--------------+ <- K_MEM_VIRT_RAM_END
 */

/* Bitmap of virtual addresses where one bit corresponds to one page.
 * This is being used for virt_region_alloc() to figure out which
 * region of virtual addresses can be used for memory mapping.
 *
 * Note that bit #0 is the highest address so that allocation is
 * done in reverse from highest address.
 */
SYS_BITARRAY_DEFINE_STATIC(virt_region_bitmap,
			   CONFIG_KERNEL_VM_ಗಾತ್ರ / CONFIG_MMU_ಪುಟ_ಗಾತ್ರ);

ಸ್ಥಿರ bool virt_region_inited;

#ವ್ಯಾಖ್ಯಾನಿಸು Z_VIRT_REGION_ಆರಂಭ_ADDR	K_MEM_VM_FREE_ಆರಂಭ
#ವ್ಯಾಖ್ಯಾನಿಸು Z_VIRT_REGION_ಕೊನೆ_ADDR		(K_MEM_VIRT_RAM_ಕೊನೆ - K_MEM_VM_RESERVED)

ಸ್ಥಿರ ಸಾಲೊಳಗೆ uintptr_t virt_from_bitmap_ಸ್ಥಾನಾಂತರ(size_t ಸ್ಥಾನಾಂತರ, size_t ಗಾತ್ರ_2)
{
	ಹಿಂತಿರುಗಿ ಸೂಚಕ_ಗಮ್ಯ_UINT(K_MEM_VIRT_RAM_ಕೊನೆ)
	       - (ಸ್ಥಾನಾಂತರ * CONFIG_MMU_ಪುಟ_ಗಾತ್ರ) - ಗಾತ್ರ_2;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ size_t virt_ಗಮ್ಯ_bitmap_ಸ್ಥಾನಾಂತರ(ಶೂನ್ಯ *vaddr, size_t ಗಾತ್ರ_2)
{
	ಹಿಂತಿರುಗಿ (ಸೂಚಕ_ಗಮ್ಯ_UINT(K_MEM_VIRT_RAM_ಕೊನೆ)
		- ಸೂಚಕ_ಗಮ್ಯ_UINT(vaddr) - ಗಾತ್ರ_2) / CONFIG_MMU_ಪುಟ_ಗಾತ್ರ;
}

ಸ್ಥಿರ ಶೂನ್ಯ virt_region_init(ಶೂನ್ಯ)
{
	size_t ಸ್ಥಾನಾಂತರ, ಅಂಕೆ_ಬಿಟ್ಟುಗಳು;

	/* There are regions where we should never map via
	 * k_mem_map() and k_mem_map_phys_bare(). Mark them as
	 * already allocated so they will never be used.
	 */

	ಆದರೆ (K_MEM_VM_RESERVED > 0) {
		/* Mark reserved region at end of virtual address space */
		ಅಂಕೆ_ಬಿಟ್ಟುಗಳು = K_MEM_VM_RESERVED / CONFIG_MMU_ಪುಟ_ಗಾತ್ರ;
		(ಶೂನ್ಯ)sys_bitarray_ಗಣ_region(&virt_region_bitmap,
					      ಅಂಕೆ_ಬಿಟ್ಟುಗಳು, 0);
	}

	/* Mark all bits up to Z_FREE_VM_START as allocated */
	ಅಂಕೆ_ಬಿಟ್ಟುಗಳು = ಸೂಚಕ_ಗಮ್ಯ_UINT(K_MEM_VM_FREE_ಆರಂಭ)
		   - ಸೂಚಕ_ಗಮ್ಯ_UINT(K_MEM_VIRT_RAM_ಆರಂಭ);
	ಸ್ಥಾನಾಂತರ = virt_ಗಮ್ಯ_bitmap_ಸ್ಥಾನಾಂತರ(K_MEM_VIRT_RAM_ಆರಂಭ, ಅಂಕೆ_ಬಿಟ್ಟುಗಳು);
	ಅಂಕೆ_ಬಿಟ್ಟುಗಳು /= CONFIG_MMU_ಪುಟ_ಗಾತ್ರ;
	(ಶೂನ್ಯ)sys_bitarray_ಗಣ_region(&virt_region_bitmap,
				      ಅಂಕೆ_ಬಿಟ್ಟುಗಳು, ಸ್ಥಾನಾಂತರ);

	virt_region_inited = true;
}

ಸ್ಥಿರ ಶೂನ್ಯ virt_region_free(ಶೂನ್ಯ *vaddr, size_t ಗಾತ್ರ_2)
{
	size_t ಸ್ಥಾನಾಂತರ, ಅಂಕೆ_ಬಿಟ್ಟುಗಳು;
	uint8_t *vaddr_u8 = (uint8_t *)vaddr;

	ಆದರೆ (unlikely(!virt_region_inited)) {
		virt_region_init();
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_KERNEL_ನೇರ_MAP
	/* Without the need to support K_MEM_DIRECT_MAP, the region must be
	 * able to be represented in the bitmap. So this case is
	 * simple.
	 */

	__ASSERT((vaddr_u8 >= Z_VIRT_REGION_ಆರಂಭ_ADDR)
		 && ((vaddr_u8 + ಗಾತ್ರ_2 - 1) < Z_VIRT_REGION_ಕೊನೆ_ADDR),
		 "invalid virtual address region %p (%zu)", vaddr_u8, ಗಾತ್ರ_2);
	ಆದರೆ (!((vaddr_u8 >= Z_VIRT_REGION_ಆರಂಭ_ADDR)
	      && ((vaddr_u8 + ಗಾತ್ರ_2 - 1) < Z_VIRT_REGION_ಕೊನೆ_ADDR))) {
		ಹಿಂತಿರುಗಿ;
	}

	ಸ್ಥಾನಾಂತರ = virt_ಗಮ್ಯ_bitmap_ಸ್ಥಾನಾಂತರ(vaddr, ಗಾತ್ರ_2);
	ಅಂಕೆ_ಬಿಟ್ಟುಗಳು = ಗಾತ್ರ_2 / CONFIG_MMU_ಪುಟ_ಗಾತ್ರ;
	(ಶೂನ್ಯ)sys_bitarray_free(&virt_region_bitmap, ಅಂಕೆ_ಬಿಟ್ಟುಗಳು, ಸ್ಥಾನಾಂತರ);
#ಇಲ್ಲದಿದ್ದರೆ /* !CONFIG_KERNEL_DIRECT_MAP */
	/* With K_MEM_DIRECT_MAP, the region can be outside of the virtual
	 * memory space, wholly within it, or overlap partially.
	 * So additional processing is needed to make sure we only
	 * mark the pages within the bitmap.
	 */
	ಆದರೆ (((vaddr_u8 >= Z_VIRT_REGION_ಆರಂಭ_ADDR) &&
	     (vaddr_u8 < Z_VIRT_REGION_ಕೊನೆ_ADDR)) ||
	    (((vaddr_u8 + ಗಾತ್ರ_2 - 1) >= Z_VIRT_REGION_ಆರಂಭ_ADDR) &&
	     ((vaddr_u8 + ಗಾತ್ರ_2 - 1) < Z_VIRT_REGION_ಕೊನೆ_ADDR))) {
		uint8_t *adjusted_ಆರಂಭ = max(vaddr_u8, Z_VIRT_REGION_ಆರಂಭ_ADDR);
		uint8_t *adjusted_ಕೊನೆ = min(vaddr_u8 + ಗಾತ್ರ_2,
					    Z_VIRT_REGION_ಕೊನೆ_ADDR);
		size_t adjusted_sz = adjusted_ಕೊನೆ - adjusted_ಆರಂಭ;

		ಸ್ಥಾನಾಂತರ = virt_ಗಮ್ಯ_bitmap_ಸ್ಥಾನಾಂತರ(adjusted_ಆರಂಭ, adjusted_sz);
		ಅಂಕೆ_ಬಿಟ್ಟುಗಳು = adjusted_sz / CONFIG_MMU_ಪುಟ_ಗಾತ್ರ;
		(ಶೂನ್ಯ)sys_bitarray_free(&virt_region_bitmap, ಅಂಕೆ_ಬಿಟ್ಟುಗಳು, ಸ್ಥಾನಾಂತರ);
	}
#ಆದರೆ_ಅಂತ್ಯ /* !CONFIG_KERNEL_DIRECT_MAP */
}

ಸ್ಥಿರ ಶೂನ್ಯ *virt_region_alloc(size_t ಗಾತ್ರ_2, size_t align)
{
	uintptr_t dest_addr;
	size_t alloc_ಗಾತ್ರ;
	size_t ಸ್ಥಾನಾಂತರ;
	size_t ಅಂಕೆ_ಬಿಟ್ಟುಗಳು;
	ಪೂರ್ಣಾಂಕ ret;

	ಆದರೆ (unlikely(!virt_region_inited)) {
		virt_region_init();
	}

	/* Possibly request more pages to ensure we can get an aligned virtual address */
	ಅಂಕೆ_ಬಿಟ್ಟುಗಳು = (ಗಾತ್ರ_2 + align - CONFIG_MMU_ಪುಟ_ಗಾತ್ರ) / CONFIG_MMU_ಪುಟ_ಗಾತ್ರ;
	alloc_ಗಾತ್ರ = ಅಂಕೆ_ಬಿಟ್ಟುಗಳು * CONFIG_MMU_ಪುಟ_ಗಾತ್ರ;
	ret = sys_bitarray_alloc(&virt_region_bitmap, ಅಂಕೆ_ಬಿಟ್ಟುಗಳು, &ಸ್ಥಾನಾಂತರ);
	ಆದರೆ (ret != 0) {
		LOG_ERR("insufficient virtual address space (requested %zu)",
			ಗಾತ್ರ_2);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	/* Remember that bit #0 in bitmap corresponds to the highest
	 * virtual address. So here we need to go downwards (backwards?)
	 * to get the starting address of the allocated region.
	 */
	dest_addr = virt_from_bitmap_ಸ್ಥಾನಾಂತರ(ಸ್ಥಾನಾಂತರ, alloc_ಗಾತ್ರ);

	ಆದರೆ (alloc_ಗಾತ್ರ > ಗಾತ್ರ_2) {
		uintptr_t aligned_dest_addr = ಸುತ್ತು_ಮೇಲೆ(dest_addr, align);

		/* Here is the memory organization when trying to get an aligned
		 * virtual address:
		 *
		 * +--------------+ <- K_MEM_VIRT_RAM_START
		 * | Undefined VM |
		 * +--------------+ <- K_MEM_KERNEL_VIRT_START (often == K_MEM_VIRT_RAM_START)
		 * | Mapping for  |
		 * | main kernel  |
		 * | image        |
		 * |		  |
		 * |		  |
		 * +--------------+ <- K_MEM_VM_FREE_START
		 * | ...          |
		 * +==============+ <- dest_addr
		 * | Unused       |
		 * |..............| <- aligned_dest_addr
		 * |              |
		 * | Aligned      |
		 * | Mapping      |
		 * |              |
		 * |..............| <- aligned_dest_addr + size
		 * | Unused       |
		 * +==============+ <- offset from K_MEM_VIRT_RAM_END == dest_addr + alloc_size
		 * | ...          |
		 * +--------------+
		 * | Mapping      |
		 * +--------------+
		 * | Reserved     |
		 * +--------------+ <- K_MEM_VIRT_RAM_END
		 */

		/* Free the two unused regions */
		virt_region_free(UINT_ಗಮ್ಯ_ಸೂಚಕ(dest_addr),
				 aligned_dest_addr - dest_addr);
		ಆದರೆ (((dest_addr + alloc_ಗಾತ್ರ) - (aligned_dest_addr + ಗಾತ್ರ_2)) > 0) {
			virt_region_free(UINT_ಗಮ್ಯ_ಸೂಚಕ(aligned_dest_addr + ಗಾತ್ರ_2),
					 (dest_addr + alloc_ಗಾತ್ರ) - (aligned_dest_addr + ಗಾತ್ರ_2));
		}

		dest_addr = aligned_dest_addr;
	}

	/* Need to make sure this does not step into kernel memory */
	ಆದರೆ (dest_addr < ಸೂಚಕ_ಗಮ್ಯ_UINT(Z_VIRT_REGION_ಆರಂಭ_ADDR)) {
		(ಶೂನ್ಯ)sys_bitarray_free(&virt_region_bitmap, ಅಂಕೆ_ಬಿಟ್ಟುಗಳು, ಸ್ಥಾನಾಂತರ);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ UINT_ಗಮ್ಯ_ಸೂಚಕ(dest_addr);
}

/*
 * Free page frames management
 *
 * Call all of these functions with z_mm_lock held.
 */

/* Linked list of unused and available page frames.
 *
 * TODO: This is very simple and treats all free page frames as being equal.
 * However, there are use-cases to consolidate free pages such that entire
 * SRAM banks can be switched off to save power, and so obtaining free pages
 * may require a more complex ontology which prefers page frames in RAM banks
 * which are still active.
 *
 * This implies in the future there may be multiple slists managing physical
 * pages. Each page frame will still just have one snode link.
 */
BUILD_ASSERT(SYS_SFLIST_FLAG_ಬಿಟ್ಟುಗಳು >= 1, "MMU free-page list needs one sflist flag bit");

ಸ್ಥಿರ sys_sflist_t free_ಪುಟ_ಚೌಕಟ್ಟು_list;

/* Number of unused and available free page frames.
 * This information may go stale immediately.
 */
ಸ್ಥಿರ size_t z_free_ಪುಟ_ಎಣಿಕೆ;

#ವ್ಯಾಖ್ಯಾನಿಸು PF_ASSERT(pf, ಅಭಿವ್ಯಕ್ತಿ, fmt, ...) \
	__ASSERT(ಅಭಿವ್ಯಕ್ತಿ, "page frame 0x%lx: " fmt, k_mem_ಪುಟ_ಚೌಕಟ್ಟು_ಗಮ್ಯ_phys(pf), \
		 ##__VA_ARGS__)

/* Get an unused page frame. don't care which one, or NULL if there are none */
ಸ್ಥಿರ ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *free_ಪುಟ_ಚೌಕಟ್ಟು_list_get(ಶೂನ್ಯ)
{
	sys_sfnode_t *ಗಂಟು;
	ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *pf = ಖಾಲಿ;

	ಗಂಟು = sys_sflist_get(&free_ಪುಟ_ಚೌಕಟ್ಟು_list);
	ಆದರೆ (ಗಂಟು != ಖಾಲಿ) {
		z_free_ಪುಟ_ಎಣಿಕೆ--;
		pf = CONTAINER_ದ(ಗಂಟು, ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು, ಗಂಟು);
		PF_ASSERT(pf, k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_free(pf),
			 "on free list but not free");
		pf->va_and_flags = 0;
	}

	ಹಿಂತಿರುಗಿ pf;
}

/* Release a page frame back into the list of free pages */
ಸ್ಥಿರ ಶೂನ್ಯ free_ಪುಟ_ಚೌಕಟ್ಟು_list_put(ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *pf)
{
	PF_ASSERT(pf, k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_ಲಭ್ಯ(pf),
		 "unavailable page put on free list");

	sys_sfnode_init(&pf->ಗಂಟು, K_MEM_ಪುಟ_ಚೌಕಟ್ಟು_FREE);
	sys_sflist_append(&free_ಪುಟ_ಚೌಕಟ್ಟು_list, &pf->ಗಂಟು);
	z_free_ಪುಟ_ಎಣಿಕೆ++;
}

ಸ್ಥಿರ ಶೂನ್ಯ free_ಪುಟ_ಚೌಕಟ್ಟು_list_init(ಶೂನ್ಯ)
{
	sys_sflist_init(&free_ಪುಟ_ಚೌಕಟ್ಟು_list);
}

ಸ್ಥಿರ ಶೂನ್ಯ ಪುಟ_ಚೌಕಟ್ಟು_free_locked(ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *pf)
{
	pf->va_and_flags = 0;
	free_ಪುಟ_ಚೌಕಟ್ಟು_list_put(pf);
}

/*
 * Memory Mapping
 */

/* Called after the frame is mapped in the arch layer, to update our
 * local ontology (and do some assertions while we're at it)
 */
ಸ್ಥಿರ ಶೂನ್ಯ ಚೌಕಟ್ಟು_mapped_ಗಣ(ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *pf, ಶೂನ್ಯ *addr)
{
	PF_ASSERT(pf, !k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_free(pf),
		  "attempted to map a page frame on the free list");
	PF_ASSERT(pf, !k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_reserved(pf),
		  "attempted to map a reserved page frame");

	/* We do allow multiple mappings for pinned page frames
	 * since we will never need to reverse map them.
	 * This is uncommon, use-cases are for things like the
	 * Zephyr equivalent of VSDOs
	 */
	PF_ASSERT(pf, !k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_mapped(pf) || k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_pinned(pf),
		 "non-pinned and already mapped to %p",
		 k_mem_ಪುಟ_ಚೌಕಟ್ಟು_ಗಮ್ಯ_virt(pf));

	uintptr_t flags_ಮುಖವಾಡ = CONFIG_MMU_ಪುಟ_ಗಾತ್ರ - 1;
	uintptr_t va = (uintptr_t)addr & ~flags_ಮುಖವಾಡ;

	pf->va_and_flags &= flags_ಮುಖವಾಡ;
	pf->va_and_flags |= va | K_MEM_ಪುಟ_ಚೌಕಟ್ಟು_MAPPED;
}

/* LCOV_EXCL_START */
/* Go through page frames to find the physical address mapped
 * by a virtual address.
 *
 * @param[in]  virt Virtual Address
 * @param[out] phys Physical address mapped to the input virtual address
 *                  if such mapping exists.
 *
 * @retval 0 if mapping is found and valid
 * @retval -EFAULT if virtual address is not mapped
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ virt_ಗಮ್ಯ_ಪುಟ_ಚೌಕಟ್ಟು(ಶೂನ್ಯ *virt, uintptr_t *phys)
{
	uintptr_t paddr;
	ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *pf;
	ಪೂರ್ಣಾಂಕ ret = -EFAULT;

	K_MEM_ಪುಟ_ಚೌಕಟ್ಟು_FOREACH(paddr, pf) {
		ಆದರೆ (k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_mapped(pf)) {
			ಆದರೆ (virt == k_mem_ಪುಟ_ಚೌಕಟ್ಟು_ಗಮ್ಯ_virt(pf)) {
				ret = 0;
				ಆದರೆ (phys != ಖಾಲಿ) {
					*phys = k_mem_ಪುಟ_ಚೌಕಟ್ಟು_ಗಮ್ಯ_phys(pf);
				}
				ನಿಲ್ಲಿಸು;
			}
		}
	}

	ಹಿಂತಿರುಗಿ ret;
}
/* LCOV_EXCL_STOP */

__weak FUNC_ALIAS(virt_ಗಮ್ಯ_ಪುಟ_ಚೌಕಟ್ಟು, arch_ಪುಟ_phys_get, ಪೂರ್ಣಾಂಕ);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಪುಟ_ಚೌಕಟ್ಟು_prepare_locked(ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *pf, bool *dirty_ptr,
				     bool ಪುಟ_ಒಳ, uintptr_t *location_ptr);

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ do_backing_store_ಪುಟ_ಒಳ(uintptr_t location);
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ do_backing_store_ಪುಟ_ಹೊರಹೋಗು(uintptr_t location);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING */

/* Allocate a free page frame, and map it to a specified virtual address
 *
 * TODO: Add optional support for copy-on-write mappings to a zero page instead
 * of allocating, in which case page frames will be allocated lazily as
 * the mappings to the zero page get touched. This will avoid expensive
 * page-ins as memory is mapped and physical RAM or backing store storage will
 * not be used if the mapped memory is unused. The cost is an empty physical
 * page of zeroes.
 *
 * @param addr Virtual address to map the page frame to
 * @param flags Mapping flags
 * @param[out] recovered Set to false if a failed mapping could not be undone,
 *                       which means the page may still be mapped and its page
 *                       frame cannot be reused.
 *
 * @retval 0 On success
 * @retval -errno On failure
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ map_anon_ಪುಟ(ಶೂನ್ಯ *addr, uint32_t flags, bool *recovered)
{
	ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *pf;
	uintptr_t phys;
	bool ಬೀಗ = (flags & K_MEM_MAP_ಬೀಗ) != 0U;
	ಪೂರ್ಣಾಂಕ ret;

	*recovered = true;

	pf = free_ಪುಟ_ಚೌಕಟ್ಟು_list_get();
	ಆದರೆ (pf == ಖಾಲಿ) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING
		uintptr_t location;
		bool dirty;

		pf = k_mem_paging_eviction_select(&dirty);
		ಆದರೆ (pf == ಖಾಲಿ) {
			/* Every evictable page frame is pinned or busy */
			ಹಿಂತಿರುಗಿ -ENOMEM;
		}
		LOG_DBG("evicting %p at 0x%lx",
			k_mem_ಪುಟ_ಚೌಕಟ್ಟು_ಗಮ್ಯ_virt(pf),
			k_mem_ಪುಟ_ಚೌಕಟ್ಟು_ಗಮ್ಯ_phys(pf));
		ret = ಪುಟ_ಚೌಕಟ್ಟು_prepare_locked(pf, &dirty, false, &location);
		ಆದರೆ (ret != 0) {
			ಹಿಂತಿರುಗಿ -ENOMEM;
		}
		ಆದರೆ (dirty) {
			do_backing_store_ಪುಟ_ಹೊರಹೋಗು(location);
		}
		pf->va_and_flags = 0;
#ಇಲ್ಲದಿದ್ದರೆ
		ಹಿಂತಿರುಗಿ -ENOMEM;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING */
	}

	phys = k_mem_ಪುಟ_ಚೌಕಟ್ಟು_ಗಮ್ಯ_phys(pf);
	ret = arch_mem_map(addr, phys, CONFIG_MMU_ಪುಟ_ಗಾತ್ರ, flags);
	__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
	ಆದರೆ (ret != 0) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
		/* The mapping above may have already succeeded in some
		 * memory domains but failing in the middle. So we need to
		 * unmap those first before returning the page frame back
		 * to the list.
		 */
		ಪೂರ್ಣಾಂಕ unmap_ret = arch_mem_unmap(addr, CONFIG_MMU_ಪುಟ_ಗಾತ್ರ);

		__ASSERT(unmap_ret == 0, "%s: cannot undo failed mapping %d\n", __func__,
			 unmap_ret);

		ಆದರೆ (unmap_ret != 0) {
			/* The page may still be mapped in some memory domains,
			 * so the page frame cannot be given back to the free
			 * list for reuse.
			 */
			*recovered = false;
			ಹಿಂತಿರುಗಿ ret;
		}
#ಆದರೆ_ಅಂತ್ಯ

		/* Mapping failed: give the page frame back before bailing out. */
		ಪುಟ_ಚೌಕಟ್ಟು_free_locked(pf);
		ಹಿಂತಿರುಗಿ ret;
	}

	ಆದರೆ (ಬೀಗ) {
		k_mem_ಪುಟ_ಚೌಕಟ್ಟು_ಗಣ(pf, K_MEM_ಪುಟ_ಚೌಕಟ್ಟು_PINNED);
	}
	ಚೌಕಟ್ಟು_mapped_ಗಣ(pf, addr);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING
	ಆದರೆ (IS_ENABLED(CONFIG_EVICTION_TRACKING) && (!ಬೀಗ)) {
		k_mem_paging_eviction_ಕೂಡಿಸು(pf);
	}
#ಆದರೆ_ಅಂತ್ಯ

	LOG_DBG("memory mapping anon page %p -> 0x%lx", addr, phys);

	ಹಿಂತಿರುಗಿ 0;
}

/**
 * Undo pages previously established by map_anon_page().
 *
 * Only used to roll back a partial k_mem_map_phys_guard() failure
 * via the anonymous memory path, the pages are known to be
 * currently mapped and not paged out.
 *
 * @param virt Start virtual address to unmap pages
 * @param sz Size of memory region to be unmapped
 *
 * @retval 0 On success
 * @retval -errno On failure
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ unmap_anon_ಪುಟಗಳು(ಶೂನ್ಯ *virt, size_t sz)
{
	uint8_t *pos;
	uintptr_t phys;
	ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *pf;
	ಪೂರ್ಣಾಂಕ ret = 0;

	VIRT_FOREACH(virt, sz, pos) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING
		ಎಣಿಕೆ arch_ಪುಟ_location ಸ್ಥಿತಿ_2;
		uintptr_t location;

		ಸ್ಥಿತಿ_2 = arch_ಪುಟ_location_get(pos, &location);
		ಆಯ್ಕೆ (ಸ್ಥಿತಿ_2) {
		ಸ್ಥಿತಿ ARCH_ಪುಟ_LOCATION_PAGED_ಹೊರಹೋಗು:
			/*
			 * No pf is associated with this mapping.
			 * Simply get rid of the MMU entry and free
			 * corresponding backing store.
			 */
			ret = arch_mem_unmap(pos, CONFIG_MMU_ಪುಟ_ಗಾತ್ರ);
			__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__,
				 ret);

			k_mem_paging_backing_store_location_free(location);

			ಆದರೆ (ret != 0) {
				ಹೋಗು unmap_anon_ಹೊರಹೋಗು;
			}

			ಮುಂದುವರಿ;
		ಸ್ಥಿತಿ ARCH_ಪುಟ_LOCATION_PAGED_ಒಳ:
			/*
			 * The page is in memory but it may not be
			 * accessible in order to manage tracking
			 * of the ARCH_DATA_PAGE_ACCESSED flag
			 * meaning arch_page_phys_get() could fail.
			 * Still, we know the actual phys address.
			 */
			phys = location;
			ret = 0;
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			ret = arch_ಪುಟ_phys_get(pos, &phys);
			ನಿಲ್ಲಿಸು;
		}
#ಇಲ್ಲದಿದ್ದರೆ
		ret = arch_ಪುಟ_phys_get(pos, &phys);
#ಆದರೆ_ಅಂತ್ಯ
		__ASSERT(ret == 0, "%s: cannot unmap an unmapped address %p", __func__, pos);
		ಆದರೆ (ret != 0) {
			/* Found an address not mapped. Do not continue. */
			ಹೋಗು unmap_anon_ಹೊರಹೋಗು;
		}

		__ASSERT(k_mem_is_ಪುಟ_ಚೌಕಟ್ಟು(phys), "%s: 0x%lx is not a page frame", __func__,
			 phys);
		ಆದರೆ (!k_mem_is_ಪುಟ_ಚೌಕಟ್ಟು(phys)) {
			/* Physical address has no corresponding page frame
			 * description in the page frame array.
			 * This should not happen. Do not continue.
			 */
			ಹೋಗು unmap_anon_ಹೊರಹೋಗು;
		}

		/* Grab the corresponding page frame from physical address */
		pf = k_mem_phys_ಗಮ್ಯ_ಪುಟ_ಚೌಕಟ್ಟು(phys);

		__ASSERT(k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_mapped(pf), "%s: 0x%lx is not a mapped page frame",
			 __func__, phys);
		ಆದರೆ (!k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_mapped(pf)) {
			/* Page frame is not marked mapped.
			 * This should not happen. Do not continue.
			 */
			ಹೋಗು unmap_anon_ಹೊರಹೋಗು;
		}

		ret = arch_mem_unmap(pos, CONFIG_MMU_ಪುಟ_ಗಾತ್ರ);
		__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
		ಆದರೆ (ret != 0) {
			/* Fail to unmap. Must bail without releasing any resources
			 * as the page is still mapped.
			 */
			ಹೋಗು unmap_anon_ಹೊರಹೋಗು;
		}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING
		ಆದರೆ (IS_ENABLED(CONFIG_EVICTION_TRACKING) &&
		    (!k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_pinned(pf))) {
			k_mem_paging_eviction_remove(pf);
		}
#ಆದರೆ_ಅಂತ್ಯ

		/* Put the page frame back into free list */
		ಪುಟ_ಚೌಕಟ್ಟು_free_locked(pf);
	}

unmap_anon_ಹೊರಹೋಗು:
	ಹಿಂತಿರುಗಿ ret;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_MAPPING
/**
 * Undo demand mapped pages previously established by map_anon_page().
 *
 * Only used to roll back a partial k_mem_map_phys_guard() failure
 * via the demand mapped path where pages are mapped with flags
 * K_MEM_MAP_UNPAGED.
 *
 * @param virt Start virtual address to umap pages
 * @param sz Size of memory region to be unmapped
 *
 * @retval true When able to unmap all pages.
 * @retval false When fails to unmap some pages.
 */
ಸ್ಥಿರ bool undo_demand_mapped_ಪುಟಗಳು(ಶೂನ್ಯ *virt, size_t sz)
{
	uint8_t *undo_pos;
	bool success = true;
	ಪೂರ್ಣಾಂಕ ret;

	/* Undo the pages mapped before the failure. */
	VIRT_FOREACH(virt, sz, undo_pos) {
		ret = arch_mem_unmap(undo_pos, CONFIG_MMU_ಪುಟ_ಗಾತ್ರ);
		__ASSERT(ret == 0, "%s: cannot undo mapped pages %d", __func__, ret);

		/* Failed to undo one mapped page. Though we still
		 * want to continue unmapping the remaining mapped
		 * pages to reclaim them.
		 */
		ಆದರೆ (ret != 0) {
			success = false;
		}
	}

	ಹಿಂತಿರುಗಿ success;
}
#ಆದರೆ_ಅಂತ್ಯ

ಶೂನ್ಯ *k_mem_map_phys_guard(uintptr_t phys, size_t ಗಾತ್ರ_2, uint32_t flags, bool is_anon)
{
	uint8_t *dst;
	uint8_t *region;
	size_t ಒಟ್ಟು_ಗಾತ್ರ;
	ಪೂರ್ಣಾಂಕ ret;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	uint8_t *pos;
	bool uninit = (flags & K_MEM_MAP_UNINIT) != 0U;

	__ASSERT(!is_anon || (is_anon && ಪುಟ_ಚೌಕಟ್ಟುಗಳು_initialized),
		 "%s called too early", __func__);
	__ASSERT((flags & K_MEM_CACHE_ಮುಖವಾಡ) == 0U,
		 "%s does not support explicit cache settings", __func__);

	ಆದರೆ (((flags & K_MEM_PERM_USER) != 0U) &&
	    ((flags & K_MEM_MAP_UNINIT) != 0U)) {
		LOG_ERR("user access to anonymous uninitialized pages is forbidden");
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}
	ಆದರೆ ((ಗಾತ್ರ_2 % CONFIG_MMU_ಪುಟ_ಗಾತ್ರ) != 0U) {
		LOG_ERR("unaligned size %zu passed to %s", ಗಾತ್ರ_2, __func__);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}
	ಆದರೆ (ಗಾತ್ರ_2 == 0) {
		LOG_ERR("zero sized memory mapping");
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	/* Need extra for the guard pages (before and after) which we
	 * won't map.
	 */
	ಆದರೆ (ಗಾತ್ರ_ಕೂಡಿಸು_ಅತಿಪ್ರವಾಹ(ಗಾತ್ರ_2, CONFIG_MMU_ಪುಟ_ಗಾತ್ರ * 2, &ಒಟ್ಟು_ಗಾತ್ರ)) {
		LOG_ERR("too large size %zu passed to %s", ಗಾತ್ರ_2, __func__);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಕೀಲಿ = k_spin_ಬೀಗ(&z_mm_ಬೀಗ);

	region = virt_region_alloc(ಒಟ್ಟು_ಗಾತ್ರ, CONFIG_MMU_ಪುಟ_ಗಾತ್ರ);
	ಆದರೆ (region == ಖಾಲಿ) {
		/* Address space has no free region */
		dst = ಖಾಲಿ;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	/* Unmap both guard pages to make sure accessing them
	 * will generate fault.
	 *
	 * Note that a failed unmap may have left the guard page mapped in
	 * some memory domains, so the virtual region cannot be freed for
	 * reuse.
	 */
	ret = arch_mem_unmap(region, CONFIG_MMU_ಪುಟ_ಗಾತ್ರ);
	__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
	ಆದರೆ (ret != 0) {
		dst = ಖಾಲಿ;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ret = arch_mem_unmap(region + CONFIG_MMU_ಪುಟ_ಗಾತ್ರ + ಗಾತ್ರ_2, CONFIG_MMU_ಪುಟ_ಗಾತ್ರ);
	__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
	ಆದರೆ (ret != 0) {
		dst = ಖಾಲಿ;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	/* Skip over the "before" guard page in returned address. */
	dst = region + CONFIG_MMU_ಪುಟ_ಗಾತ್ರ;

	ಆದರೆ (is_anon) {
		/* Mapping from anonymous memory */
		flags |= K_MEM_CACHE_WB;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_MAPPING
		ಆದರೆ ((flags & K_MEM_MAP_ಬೀಗ) == 0) {
			flags |= K_MEM_MAP_UNPAGED;
			VIRT_FOREACH(dst, ಗಾತ್ರ_2, pos) {
				ret = arch_mem_map(pos,
						   uninit ? ARCH_UNPAGED_ANON_UNINIT
							  : ARCH_UNPAGED_ANON_ZERO,
						   CONFIG_MMU_ಪುಟ_ಗಾತ್ರ, flags);
				__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
				ಆದರೆ (ret != 0) {
					size_t unmap_sz = pos - dst;

					ಆದರೆ (IS_ENABLED(CONFIG_USERSPACE)) {
						/* Note that it is unmapping an extra page due to
						 * the possibility that the mapping above has
						 * already updated some memory domains but fail in
						 * the middle. So we need to include the failed
						 * page to unmap those that have been mapped so
						 * far.
						 */
						unmap_sz += CONFIG_MMU_ಪುಟ_ಗಾತ್ರ;
					}

					bool success = undo_demand_mapped_ಪುಟಗಳು(dst, unmap_sz);

					dst = ಖಾಲಿ;

					ಆದರೆ (success) {
						ಹೋಗು fail;
					} ಇಲ್ಲದಿದ್ದರೆ {
						/* Cannot fully recover from mapping
						 * failure. We cannot free the allocated
						 * virtual memory region since some of
						 * the page may still be mapped.
						 */
						ಹೋಗು ಹೊರಹೋಗು;
					}
				}
			}
			LOG_DBG("memory mapping anon pages %p to %p unpaged", dst, pos-1);
			/* skip the memset() below */
			uninit = true;
		} ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
		{
			VIRT_FOREACH(dst, ಗಾತ್ರ_2, pos) {
				bool recovered;

				ret = map_anon_ಪುಟ(pos, flags, &recovered);

				ಆದರೆ (ret != 0) {
					ಪೂರ್ಣಾಂಕ unmap_ret = unmap_anon_ಪುಟಗಳು(dst, pos - dst);

					dst = ಖಾಲಿ;

					ಆದರೆ ((unmap_ret == 0) && recovered) {
						ಹೋಗು fail;
					} ಇಲ್ಲದಿದ್ದರೆ {
						/* Cannot fully recover from mapping
						 * failure. We cannot free the allocated
						 * virtual memory region since some of
						 * the page may still be mapped.
						 */
						ಹೋಗು ಹೊರಹೋಗು;
					}
				}
			}
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Mapping known physical memory. */
		ret = arch_mem_map(dst, phys, ಗಾತ್ರ_2, flags);
		__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
		ಆದರೆ (ret != 0) {
			/* Clean up any partial mapping left behind. */
			ret = arch_mem_unmap(dst, ಗಾತ್ರ_2);
			__ASSERT(ret == 0, "%s: cannot undo mapped pages %d", __func__, ret);

			dst = ಖಾಲಿ;

			ಆದರೆ (ret != 0) {
				/* Cannot fully undo all the mappings. There might still be pages
				 * mapped. So we cannot free the allocated virtual memory region.
				 */
				ಹೋಗು ಹೊರಹೋಗು;
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಹೋಗು fail;
			}
		}
	}

ಹೊರಹೋಗು:
	k_spin_unlock(&z_mm_ಬೀಗ, ಕೀಲಿ);

	ಆದರೆ (dst != ಖಾಲಿ && !uninit) {
		/* If we later implement mappings to a copy-on-write
		 * zero page, won't need this step
		 */
		memset(dst, 0, ಗಾತ್ರ_2);
	}

	ಹಿಂತಿರುಗಿ dst;

fail:
	/* Release the whole region, including the guard pages. */
	virt_region_free(region, ಒಟ್ಟು_ಗಾತ್ರ);

	ಹೋಗು ಹೊರಹೋಗು;
}

ಶೂನ್ಯ k_mem_unmap_phys_guard(ಶೂನ್ಯ *addr, size_t ಗಾತ್ರ_2, bool is_anon)
{
	uint8_t *pos;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	size_t ಒಟ್ಟು_ಗಾತ್ರ;
	ಪೂರ್ಣಾಂಕ ret;

	/* Need space for the "before" guard page */
	__ASSERT_NO_MSG(ಸೂಚಕ_ಗಮ್ಯ_UINT(addr) >= CONFIG_MMU_ಪುಟ_ಗಾತ್ರ);

	/* Make sure address range is still valid after accounting
	 * for two guard pages.
	 */
	pos = (uint8_t *)addr - CONFIG_MMU_ಪುಟ_ಗಾತ್ರ;
	k_mem_assert_virtual_region(pos, ಗಾತ್ರ_2 + (CONFIG_MMU_ಪುಟ_ಗಾತ್ರ * 2));

	ಕೀಲಿ = k_spin_ಬೀಗ(&z_mm_ಬೀಗ);

	/* Check if both guard pages are unmapped.
	 * Bail if not, as this is probably a region not mapped
	 * using k_mem_map().
	 */
	pos = addr;
	ret = arch_ಪುಟ_phys_get(pos - CONFIG_MMU_ಪುಟ_ಗಾತ್ರ, ಖಾಲಿ);
	__ASSERT(ret != 0,
		 "%s: cannot find preceding guard page for (%p, %zu)",
		 __func__, addr, ಗಾತ್ರ_2);
	ಆದರೆ (ret == 0) {
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ret = arch_ಪುಟ_phys_get(pos + ಗಾತ್ರ_2, ಖಾಲಿ);
	__ASSERT(ret != 0,
		 "%s: cannot find succeeding guard page for (%p, %zu)",
		 __func__, addr, ಗಾತ್ರ_2);
	ಆದರೆ (ret == 0) {
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ಆದರೆ (is_anon) {
		/* Unmapping anonymous memory */
		ret = unmap_anon_ಪುಟಗಳು(addr, ಗಾತ್ರ_2);
		__ASSERT(ret == 0, "%s: unmap_anon_pages() returned %d", __func__, ret);
		ಆದರೆ (ret != 0) {
			/* Fail to unmap. Must bail without releasing any resources
			 * as some pages are still be mapped.
			 */
			ಹೋಗು ಹೊರಹೋಗು;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		/*
		 * Unmapping previous mapped memory with specific physical address.
		 *
		 * Note that we don't have to unmap the guard pages, as they should
		 * have been unmapped. We just need to unmapped the in-between
		 * region [addr, (addr + size)).
		 */
		ret = arch_mem_unmap(addr, ಗಾತ್ರ_2);
		__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
		ಆದರೆ (ret != 0) {
			/* Fail to unmap. Must bail without releasing any resources
			 * as some pages are still be mapped.
			 */
			ಹೋಗು ಹೊರಹೋಗು;
		}
	}

	/* There are guard pages just before and after the mapped
	 * region. So we also need to free them from the bitmap.
	 */
	pos = (uint8_t *)addr - CONFIG_MMU_ಪುಟ_ಗಾತ್ರ;
	ಒಟ್ಟು_ಗಾತ್ರ = ಗಾತ್ರ_2 + (CONFIG_MMU_ಪುಟ_ಗಾತ್ರ * 2);
	virt_region_free(pos, ಒಟ್ಟು_ಗಾತ್ರ);

ಹೊರಹೋಗು:
	k_spin_unlock(&z_mm_ಬೀಗ, ಕೀಲಿ);
}

ಪೂರ್ಣಾಂಕ k_mem_update_flags(ಶೂನ್ಯ *addr, size_t ಗಾತ್ರ_2, uint32_t flags)
{
	uintptr_t phys;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ಪೂರ್ಣಾಂಕ ret;

	k_mem_assert_virtual_region(addr, ಗಾತ್ರ_2);

	ಕೀಲಿ = k_spin_ಬೀಗ(&z_mm_ಬೀಗ);

	/*
	 * We can achieve desired result without explicit architecture support
	 * by unmapping and remapping the same physical memory using new flags.
	 */

	ret = arch_ಪುಟ_phys_get(addr, &phys);
	ಆದರೆ (ret < 0) {
		ಹೋಗು ಹೊರಹೋಗು;
	}

	/* TODO: detect and handle paged-out memory as well */

	ret = arch_mem_unmap(addr, ಗಾತ್ರ_2);
	__ASSERT(ret == 0, "%s: arch_mem_unmap failed %d\n", __func__, ret);
	ಆದರೆ (ret != 0) {
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ret = arch_mem_map(addr, phys, ಗಾತ್ರ_2, flags);
	__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);

ಹೊರಹೋಗು:
	k_spin_unlock(&z_mm_ಬೀಗ, ಕೀಲಿ);
	ಹಿಂತಿರುಗಿ ret;
}

size_t k_mem_free_get(ಶೂನ್ಯ)
{
	size_t ret;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;

	__ASSERT(ಪುಟ_ಚೌಕಟ್ಟುಗಳು_initialized, "%s called too early", __func__);

	ಕೀಲಿ = k_spin_ಬೀಗ(&z_mm_ಬೀಗ);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING
	ಆದರೆ (z_free_ಪುಟ_ಎಣಿಕೆ > CONFIG_DEMAND_PAGING_ಪುಟ_ಚೌಕಟ್ಟುಗಳು_RESERVE) {
		ret = z_free_ಪುಟ_ಎಣಿಕೆ - CONFIG_DEMAND_PAGING_ಪುಟ_ಚೌಕಟ್ಟುಗಳು_RESERVE;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ret = 0;
	}
#ಇಲ್ಲದಿದ್ದರೆ
	ret = z_free_ಪುಟ_ಎಣಿಕೆ;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING */
	k_spin_unlock(&z_mm_ಬೀಗ, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ ret * (size_t)CONFIG_MMU_ಪುಟ_ಗಾತ್ರ;
}

/* Get the default virtual region alignment, here the default MMU page size
 *
 * @param[in] phys Physical address of region to be mapped, aligned to MMU_PAGE_SIZE
 * @param[in] size Size of region to be mapped, aligned to MMU_PAGE_SIZE
 *
 * @return alignment to apply on the virtual address of this region
 */
ಸ್ಥಿರ size_t virt_region_align(uintptr_t phys, size_t ಗಾತ್ರ_2)
{
	ARG_ಬಳಸದ(phys);
	ARG_ಬಳಸದ(ಗಾತ್ರ_2);

	ಹಿಂತಿರುಗಿ CONFIG_MMU_ಪುಟ_ಗಾತ್ರ;
}

__weak FUNC_ALIAS(virt_region_align, arch_virt_region_align, size_t);

/* This may be called from arch early boot code before z_cstart() is invoked.
 * Data will be copied and BSS zeroed, but this must not rely on any
 * initialization functions being called prior to work correctly.
 */
ಶೂನ್ಯ k_mem_map_phys_bare(uint8_t **virt_ptr, uintptr_t phys, size_t ಗಾತ್ರ_2, uint32_t flags)
{
	uintptr_t aligned_phys, addr_ಸ್ಥಾನಾಂತರ;
	size_t aligned_ಗಾತ್ರ, align_boundary;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	uint8_t *dest_addr;
	size_t ಅಂಕೆ_ಬಿಟ್ಟುಗಳು;
	size_t ಸ್ಥಾನಾಂತರ;
	ಪೂರ್ಣಾಂಕ ret;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_KERNEL_ನೇರ_MAP
	__ASSERT(!(flags & K_MEM_ನೇರ_MAP), "The direct-map is not enabled");
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_KERNEL_DIRECT_MAP */
	addr_ಸ್ಥಾನಾಂತರ = k_mem_region_align(&aligned_phys, &aligned_ಗಾತ್ರ,
					 phys, ಗಾತ್ರ_2,
					 CONFIG_MMU_ಪುಟ_ಗಾತ್ರ);
	__ASSERT(aligned_ಗಾತ್ರ != 0U, "0-length mapping at 0x%lx", aligned_phys);
	__ASSERT(aligned_ಗಾತ್ರ - 1 <= (UINTPTR_MAX - aligned_phys),
		 "wraparound for physical address 0x%lx (size %zu)",
		 aligned_phys, aligned_ಗಾತ್ರ);

	align_boundary = arch_virt_region_align(aligned_phys, aligned_ಗಾತ್ರ);

	ಕೀಲಿ = k_spin_ಬೀಗ(&z_mm_ಬೀಗ);

	ಆದರೆ (IS_ENABLED(CONFIG_KERNEL_ನೇರ_MAP) &&
	    (flags & K_MEM_ನೇರ_MAP)) {
		dest_addr = (uint8_t *)aligned_phys;

		/* Mark the region of virtual memory bitmap as used
		 * if the region overlaps the virtual memory space.
		 *
		 * Basically if either end of region is within
		 * virtual memory space, we need to mark the bits.
		 */

		ಆದರೆ (ಒಳ_RANGE(aligned_phys,
			      (uintptr_t)K_MEM_VIRT_RAM_ಆರಂಭ,
			      (uintptr_t)(K_MEM_VIRT_RAM_ಕೊನೆ - 1)) ||
		    ಒಳ_RANGE(aligned_phys + aligned_ಗಾತ್ರ - 1,
			      (uintptr_t)K_MEM_VIRT_RAM_ಆರಂಭ,
			      (uintptr_t)(K_MEM_VIRT_RAM_ಕೊನೆ - 1))) {
			uint8_t *adjusted_ಆರಂಭ = max(dest_addr, K_MEM_VIRT_RAM_ಆರಂಭ);
			uint8_t *adjusted_ಕೊನೆ = min(dest_addr + aligned_ಗಾತ್ರ,
						    K_MEM_VIRT_RAM_ಕೊನೆ);
			size_t adjusted_sz = adjusted_ಕೊನೆ - adjusted_ಆರಂಭ;

			ಅಂಕೆ_ಬಿಟ್ಟುಗಳು = adjusted_sz / CONFIG_MMU_ಪುಟ_ಗಾತ್ರ;
			ಸ್ಥಾನಾಂತರ = virt_ಗಮ್ಯ_bitmap_ಸ್ಥಾನಾಂತರ(adjusted_ಆರಂಭ, adjusted_sz);
			ಆದರೆ (sys_bitarray_test_and_ಗಣ_region(
			    &virt_region_bitmap, ಅಂಕೆ_ಬಿಟ್ಟುಗಳು, ಸ್ಥಾನಾಂತರ, true)) {
				ಹೋಗು fail;
			}
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Obtain an appropriately sized chunk of virtual memory */
		dest_addr = virt_region_alloc(aligned_ಗಾತ್ರ, align_boundary);
		ಆದರೆ (!dest_addr) {
			ಹೋಗು fail;
		}
	}

	/* If this fails there's something amiss with virt_region_get */
	__ASSERT((uintptr_t)dest_addr <
		 ((uintptr_t)dest_addr + (ಗಾತ್ರ_2 - 1)),
		 "wraparound for virtual address %p (size %zu)",
		 dest_addr, ಗಾತ್ರ_2);

	LOG_DBG("arch_mem_map(%p, 0x%lx, %zu, %x) offset %lu", (ಶೂನ್ಯ *)dest_addr,
		aligned_phys, aligned_ಗಾತ್ರ, flags, addr_ಸ್ಥಾನಾಂತರ);

	ret = arch_mem_map(dest_addr, aligned_phys, aligned_ಗಾತ್ರ, flags);
	__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
	ಆದರೆ (ret != 0) {
		ಹೋಗು fail;
	}
	k_spin_unlock(&z_mm_ಬೀಗ, ಕೀಲಿ);

	*virt_ptr = dest_addr + addr_ಸ್ಥಾನಾಂತರ;
	ಹಿಂತಿರುಗಿ;
fail:
	/* May re-visit this in the future, but for now running out of
	 * virtual address space or failing the arch_mem_map() call is
	 * an unrecoverable situation.
	 *
	 * Other problems not related to resource exhaustion we leave as
	 * assertions since they are clearly programming mistakes.
	 */
	LOG_ERR("memory mapping 0x%lx (size %zu, flags 0x%x) failed",
		phys, ಗಾತ್ರ_2, flags);
	k_panic();
}

ಶೂನ್ಯ k_mem_unmap_phys_bare(uint8_t *virt, size_t ಗಾತ್ರ_2)
{
	uintptr_t aligned_virt, addr_ಸ್ಥಾನಾಂತರ;
	size_t aligned_ಗಾತ್ರ;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ಪೂರ್ಣಾಂಕ ret;

	addr_ಸ್ಥಾನಾಂತರ = k_mem_region_align(&aligned_virt, &aligned_ಗಾತ್ರ,
					 ಸೂಚಕ_ಗಮ್ಯ_UINT(virt), ಗಾತ್ರ_2,
					 CONFIG_MMU_ಪುಟ_ಗಾತ್ರ);
	__ASSERT(aligned_ಗಾತ್ರ != 0U, "0-length mapping at 0x%lx", aligned_virt);
	__ASSERT(aligned_ಗಾತ್ರ - 1 <= (UINTPTR_MAX - aligned_virt),
		 "wraparound for virtual address 0x%lx (size %zu)",
		 aligned_virt, aligned_ಗಾತ್ರ);

	ಕೀಲಿ = k_spin_ಬೀಗ(&z_mm_ಬೀಗ);

	LOG_DBG("arch_mem_unmap(0x%lx, %zu) offset %lu",
		aligned_virt, aligned_ಗಾತ್ರ, addr_ಸ್ಥಾನಾಂತರ);

	ret = arch_mem_unmap(UINT_ಗಮ್ಯ_ಸೂಚಕ(aligned_virt), aligned_ಗಾತ್ರ);
	__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
	ಆದರೆ (ret != 0) {
		/* Fail to unmap. We cannot free the virtual memory region
		 * as some pages are still be mapped.
		 */
		ಹೋಗು ಹೊರಹೋಗು;
	}

	virt_region_free(UINT_ಗಮ್ಯ_ಸೂಚಕ(aligned_virt), aligned_ಗಾತ್ರ);

ಹೊರಹೋಗು:
	k_spin_unlock(&z_mm_ಬೀಗ, ಕೀಲಿ);
}

/*
 * Miscellaneous
 */

size_t k_mem_region_align(uintptr_t *aligned_addr, size_t *aligned_ಗಾತ್ರ,
			  uintptr_t addr, size_t ಗಾತ್ರ_2, size_t align)
{
	size_t addr_ಸ್ಥಾನಾಂತರ;

	/* The actual mapped region must be page-aligned. Round down the
	 * physical address and pad the region size appropriately
	 */
	*aligned_addr = ಸುತ್ತು_ಕೆಳಕ್ಕೆ(addr, align);
	addr_ಸ್ಥಾನಾಂತರ = addr - *aligned_addr;
	*aligned_ಗಾತ್ರ = ಸುತ್ತು_ಮೇಲೆ(ಗಾತ್ರ_2 + addr_ಸ್ಥಾನಾಂತರ, align);

	ಹಿಂತಿರುಗಿ addr_ಸ್ಥಾನಾಂತರ;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_LINKER_USE_BOOT_SECTION
ಸ್ಥಿರ ಶೂನ್ಯ mark_linker_section_pinned(ಶೂನ್ಯ *ಆರಂಭ_addr, ಶೂನ್ಯ *ಕೊನೆ_addr,
				       bool pin)
{
	ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *pf;
	uint8_t *addr;

	uintptr_t pinned_ಆರಂಭ = ಸುತ್ತು_ಕೆಳಕ್ಕೆ(ಸೂಚಕ_ಗಮ್ಯ_UINT(ಆರಂಭ_addr),
					    CONFIG_MMU_ಪುಟ_ಗಾತ್ರ);
	uintptr_t pinned_ಕೊನೆ = ಸುತ್ತು_ಮೇಲೆ(ಸೂಚಕ_ಗಮ್ಯ_UINT(ಕೊನೆ_addr),
					CONFIG_MMU_ಪುಟ_ಗಾತ್ರ);
	size_t pinned_ಗಾತ್ರ = pinned_ಕೊನೆ - pinned_ಆರಂಭ;

	VIRT_FOREACH(UINT_ಗಮ್ಯ_ಸೂಚಕ(pinned_ಆರಂಭ), pinned_ಗಾತ್ರ, addr)
	{
		pf = k_mem_phys_ಗಮ್ಯ_ಪುಟ_ಚೌಕಟ್ಟು(K_MEM_BOOT_VIRT_ಗಮ್ಯ_PHYS(addr));
		ಚೌಕಟ್ಟು_mapped_ಗಣ(pf, addr);

		ಆದರೆ (pin) {
			k_mem_ಪುಟ_ಚೌಕಟ್ಟು_ಗಣ(pf, K_MEM_ಪುಟ_ಚೌಕಟ್ಟು_PINNED);
		} ಇಲ್ಲದಿದ್ದರೆ {
			k_mem_ಪುಟ_ಚೌಕಟ್ಟು_clear(pf, K_MEM_ಪುಟ_ಚೌಕಟ್ಟು_PINNED);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING
			ಆದರೆ (IS_ENABLED(CONFIG_EVICTION_TRACKING) &&
			    k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_evictable(pf)) {
				k_mem_paging_eviction_ಕೂಡಿಸು(pf);
			}
#ಆದರೆ_ಅಂತ್ಯ
		}
	}
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_LINKER_USE_BOOT_SECTION */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_LINKER_USE_ONDEMAND_SECTION
ಸ್ಥಿರ ಶೂನ್ಯ z_paging_ondemand_section_map(ಶೂನ್ಯ)
{
	uint8_t *addr;
	size_t ಗಾತ್ರ_2;
	uintptr_t location;
	uint32_t flags;
	ಪೂರ್ಣಾಂಕ ret;

	ಗಾತ್ರ_2 = (uintptr_t)lnkr_ondemand_ಪಠ್ಯ_ಗಾತ್ರ;
	flags = K_MEM_MAP_UNPAGED | K_MEM_PERM_EXEC | K_MEM_CACHE_WB;
	VIRT_FOREACH(lnkr_ondemand_ಪಠ್ಯ_ಆರಂಭ, ಗಾತ್ರ_2, addr) {
		k_mem_paging_backing_store_location_query(addr, &location);

		ret = arch_mem_map(addr, location, CONFIG_MMU_ಪುಟ_ಗಾತ್ರ, flags);
		__ASSERT(ret == 0, "arch_mem_map() returned %d", ret);
		ಆದರೆ (ret != 0) {
			k_panic();
		}

		sys_bitarray_ಗಣ_region(&virt_region_bitmap, 1,
					virt_ಗಮ್ಯ_bitmap_ಸ್ಥಾನಾಂತರ(addr, CONFIG_MMU_ಪುಟ_ಗಾತ್ರ));
	}

	ಗಾತ್ರ_2 = (uintptr_t)lnkr_ondemand_rodata_ಗಾತ್ರ;
	flags = K_MEM_MAP_UNPAGED | K_MEM_CACHE_WB;
	VIRT_FOREACH(lnkr_ondemand_rodata_ಆರಂಭ, ಗಾತ್ರ_2, addr) {
		k_mem_paging_backing_store_location_query(addr, &location);

		ret = arch_mem_map(addr, location, CONFIG_MMU_ಪುಟ_ಗಾತ್ರ, flags);
		__ASSERT(ret == 0, "arch_mem_map() returned %d", ret);
		ಆದರೆ (ret != 0) {
			k_panic();
		}

		sys_bitarray_ಗಣ_region(&virt_region_bitmap, 1,
					virt_ಗಮ್ಯ_bitmap_ಸ್ಥಾನಾಂತರ(addr, CONFIG_MMU_ಪುಟ_ಗಾತ್ರ));
	}
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_LINKER_USE_ONDEMAND_SECTION */

ಶೂನ್ಯ z_mem_manage_init(ಶೂನ್ಯ)
{
	uintptr_t phys;
	uint8_t *addr;
	ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *pf;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&z_mm_ಬೀಗ);

	free_ಪುಟ_ಚೌಕಟ್ಟು_list_init();

	ARG_ಬಳಸದ(addr);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ARCH_HAS_RESERVED_ಪುಟ_ಚೌಕಟ್ಟುಗಳು
	/* If some page frames are unavailable for use as memory, arch
	 * code will mark K_MEM_PAGE_FRAME_RESERVED in their flags
	 */
	arch_reserved_ಪುಟಗಳು_update();
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_ARCH_HAS_RESERVED_PAGE_FRAMES */

	/* The entire Zephyr image is mapped and pinned at boot. Demand
	 * paging applies only to anonymous mappings created via k_mem_map()
	 * and to memory placed in explicit __ondemand_* linker sections;
	 * the kernel image itself is never a candidate for eviction.
	 */
	VIRT_FOREACH(K_MEM_KERNEL_VIRT_ಆರಂಭ, K_MEM_KERNEL_VIRT_ಗಾತ್ರ, addr)
	{
		pf = k_mem_phys_ಗಮ್ಯ_ಪುಟ_ಚೌಕಟ್ಟು(K_MEM_BOOT_VIRT_ಗಮ್ಯ_PHYS(addr));
		ಚೌಕಟ್ಟು_mapped_ಗಣ(pf, addr);
		k_mem_ಪುಟ_ಚೌಕಟ್ಟು_ಗಣ(pf, K_MEM_ಪುಟ_ಚೌಕಟ್ಟು_PINNED);
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_LINKER_USE_BOOT_SECTION
	/* Pin the boot section to prevent it from being swapped out during
	 * boot process. Will be un-pinned once boot process completes.
	 */
	mark_linker_section_pinned(lnkr_boot_ಆರಂಭ, lnkr_boot_ಕೊನೆ, true);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_LINKER_USE_BOOT_SECTION */

	/* Any remaining pages that aren't mapped, reserved, or pinned get
	 * added to the free pages list
	 */
	K_MEM_ಪುಟ_ಚೌಕಟ್ಟು_FOREACH(phys, pf) {
		ಆದರೆ (k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_ಲಭ್ಯ(pf)) {
			free_ಪುಟ_ಚೌಕಟ್ಟು_list_put(pf);
		}
	}
	LOG_DBG("free page frames: %zu", z_free_ಪುಟ_ಎಣಿಕೆ);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	z_paging_histogram_init();
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
	k_mem_paging_backing_store_init();
	k_mem_paging_eviction_init();

	ಆದರೆ (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
		/* start tracking evictable page installed above if any */
		K_MEM_ಪುಟ_ಚೌಕಟ್ಟು_FOREACH(phys, pf) {
			ಆದರೆ (k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_evictable(pf)) {
				k_mem_paging_eviction_ಕೂಡಿಸು(pf);
			}
		}
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_LINKER_USE_ONDEMAND_SECTION
	z_paging_ondemand_section_map();
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ASSERT
	ಪುಟ_ಚೌಕಟ್ಟುಗಳು_initialized = true;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_ASSERT */

	k_spin_unlock(&z_mm_ಬೀಗ, ಕೀಲಿ);
}

ಶೂನ್ಯ z_mem_manage_boot_ಮುಕ್ತಾಯ(ಶೂನ್ಯ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_LINKER_USE_BOOT_SECTION
	/* At the end of boot process, unpin the boot sections
	 * as they don't need to be in memory all the time anymore.
	 */
	mark_linker_section_pinned(lnkr_boot_ಆರಂಭ, lnkr_boot_ಕೊನೆ, false);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_LINKER_USE_BOOT_SECTION */
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_STATS
ರಚನೆ k_mem_paging_stats_t paging_stats;
ಬಾಹ್ಯ ರಚನೆ k_mem_paging_histogram_t z_paging_histogram_eviction;
ಬಾಹ್ಯ ರಚನೆ k_mem_paging_histogram_t z_paging_histogram_backing_store_ಪುಟ_ಒಳ;
ಬಾಹ್ಯ ರಚನೆ k_mem_paging_histogram_t z_paging_histogram_backing_store_ಪುಟ_ಹೊರಹೋಗು;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_STATS */

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ do_backing_store_ಪುಟ_ಒಳ(uintptr_t location)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_MAPPING
	/* Check for special cases */
	ಆಯ್ಕೆ (location) {
	ಸ್ಥಿತಿ ARCH_UNPAGED_ANON_ZERO:
		memset(K_MEM_SCRATCH_ಪುಟ, 0, CONFIG_MMU_ಪುಟ_ಗಾತ್ರ);
		__fallthrough;
	ಸ್ಥಿತಿ ARCH_UNPAGED_ANON_UNINIT:
		/* nothing else to do */
		ಹಿಂತಿರುಗಿ;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_MAPPING */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	uint32_t ಸಮಯ_diff;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	timing_t ಸಮಯ_ಆರಂಭ, ಸಮಯ_ಕೊನೆ;

	ಸಮಯ_ಆರಂಭ = timing_counter_get();
#ಇಲ್ಲದಿದ್ದರೆ
	uint32_t ಸಮಯ_ಆರಂಭ;

	ಸಮಯ_ಆರಂಭ = k_ಚಕ್ರ_get_32();
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	k_mem_paging_backing_store_ಪುಟ_ಒಳ(location);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	ಸಮಯ_ಕೊನೆ = timing_counter_get();
	ಸಮಯ_diff = (uint32_t)timing_ಚಕ್ರಗಳು_get(&ಸಮಯ_ಆರಂಭ, &ಸಮಯ_ಕೊನೆ);
#ಇಲ್ಲದಿದ್ದರೆ
	ಸಮಯ_diff = k_ಚಕ್ರ_get_32() - ಸಮಯ_ಆರಂಭ;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */

	z_paging_histogram_inc(&z_paging_histogram_backing_store_ಪುಟ_ಒಳ,
			       ಸಮಯ_diff);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ do_backing_store_ಪುಟ_ಹೊರಹೋಗು(uintptr_t location)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	uint32_t ಸಮಯ_diff;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	timing_t ಸಮಯ_ಆರಂಭ, ಸಮಯ_ಕೊನೆ;

	ಸಮಯ_ಆರಂಭ = timing_counter_get();
#ಇಲ್ಲದಿದ್ದರೆ
	uint32_t ಸಮಯ_ಆರಂಭ;

	ಸಮಯ_ಆರಂಭ = k_ಚಕ್ರ_get_32();
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	k_mem_paging_backing_store_ಪುಟ_ಹೊರಹೋಗು(location);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	ಸಮಯ_ಕೊನೆ = timing_counter_get();
	ಸಮಯ_diff = (uint32_t)timing_ಚಕ್ರಗಳು_get(&ಸಮಯ_ಆರಂಭ, &ಸಮಯ_ಕೊನೆ);
#ಇಲ್ಲದಿದ್ದರೆ
	ಸಮಯ_diff = k_ಚಕ್ರ_get_32() - ಸಮಯ_ಆರಂಭ;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */

	z_paging_histogram_inc(&z_paging_histogram_backing_store_ಪುಟ_ಹೊರಹೋಗು,
			       ಸಮಯ_diff);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
}

#ಆದರೆ defined(CONFIG_SMP) && defined(CONFIG_DEMAND_PAGING_ALLOW_IRQ)
/*
 * SMP support is very simple. Some resources such as the scratch page could
 * be made per CPU, backing store driver execution be confined to the faulting
 * CPU, statistics be made to cope with access concurrency, etc. But in the
 * end we're dealing with memory transfer to/from some external storage which
 * is inherently slow and whose access is most likely serialized anyway.
 * So let's simply enforce global demand paging serialization across all CPUs
 * with a mutex as there is no real gain from added parallelism here.
 */
ಸ್ಥಿರ K_MUTEX_DEFINE(z_mm_paging_ಬೀಗ);
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಶೂನ್ಯ virt_region_foreach(ಶೂನ್ಯ *addr, size_t ಗಾತ್ರ_2,
				ಶೂನ್ಯ (*func)(ಶೂನ್ಯ *vaddr))
{
	k_mem_assert_virtual_region(addr, ಗಾತ್ರ_2);

	ಗಾಗಿ (size_t ಸ್ಥಾನಾಂತರ = 0; ಸ್ಥಾನಾಂತರ < ಗಾತ್ರ_2; ಸ್ಥಾನಾಂತರ += CONFIG_MMU_ಪುಟ_ಗಾತ್ರ) {
		func((uint8_t *)addr + ಸ್ಥಾನಾಂತರ);
	}
}

/*
 * Perform some preparatory steps before paging out. The provided page frame
 * must be evicted to the backing store immediately after this is called
 * with a call to k_mem_paging_backing_store_page_out() if it contains
 * a data page.
 *
 * - Map page frame to scratch area if requested. This always is true if we're
 *   doing a page fault, but is only set on manual evictions if the page is
 *   dirty.
 * - If mapped:
 *    - obtain backing store location and populate location parameter
 *    - Update page tables with location
 * - Mark page frame as busy
 *
 * Returns -ENOMEM if the backing store is full
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಪುಟ_ಚೌಕಟ್ಟು_prepare_locked(ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *pf, bool *dirty_ptr,
				     bool ಪುಟ_fault, uintptr_t *location_ptr)
{
	uintptr_t phys;
	ಪೂರ್ಣಾಂಕ ret;
	bool dirty = *dirty_ptr;

	phys = k_mem_ಪುಟ_ಚೌಕಟ್ಟು_ಗಮ್ಯ_phys(pf);
	__ASSERT(!k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_pinned(pf), "page frame 0x%lx is pinned",
		 phys);

	/* If the backing store doesn't have a copy of the page, even if it
	 * wasn't modified, treat as dirty. This can happen for a few
	 * reasons:
	 * 1) Page has never been swapped out before, and the backing store
	 *    wasn't pre-populated with this data page.
	 * 2) Page was swapped out before, but the page contents were not
	 *    preserved after swapping back in.
	 * 3) Page contents were preserved when swapped back in, but were later
	 *    evicted from the backing store to make room for other evicted
	 *    pages.
	 */
	ಆದರೆ (k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_mapped(pf)) {
		dirty = dirty || !k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_backed(pf);
	}

	ಆದರೆ (dirty || ಪುಟ_fault) {
		arch_mem_scratch(phys);
	}

	ಆದರೆ (k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_mapped(pf)) {
		ret = k_mem_paging_backing_store_location_get(pf, location_ptr,
							      ಪುಟ_fault);
		ಆದರೆ (ret != 0) {
			LOG_ERR("out of backing store memory");
			ಹಿಂತಿರುಗಿ -ENOMEM;
		}
		arch_mem_ಪುಟ_ಹೊರಹೋಗು(k_mem_ಪುಟ_ಚೌಕಟ್ಟು_ಗಮ್ಯ_virt(pf), *location_ptr);

		ಆದರೆ (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
			k_mem_paging_eviction_remove(pf);
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Shouldn't happen unless this function is mis-used */
		__ASSERT(!dirty, "un-mapped page determined to be dirty");
	}
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	/* Mark as busy so that k_mem_page_frame_is_evictable() returns false */
	__ASSERT(!k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_busy(pf), "page frame 0x%lx is already busy",
		 phys);
	k_mem_ಪುಟ_ಚೌಕಟ್ಟು_ಗಣ(pf, K_MEM_ಪುಟ_ಚೌಕಟ್ಟು_BUSY);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	/* Update dirty parameter, since we set to true if it wasn't backed
	 * even if otherwise clean
	 */
	*dirty_ptr = dirty;

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ do_mem_evict(ಶೂನ್ಯ *addr)
{
	bool dirty;
	ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *pf;
	uintptr_t location;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	uintptr_t flags, phys;
	ಪೂರ್ಣಾಂಕ ret;

#ಆದರೆ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	__ASSERT(!k_is_ಒಳ_isr(),
		 "%s is unavailable in ISRs with CONFIG_DEMAND_PAGING_ALLOW_IRQ",
		 __func__);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
	k_mutex_ಬೀಗ(&z_mm_paging_ಬೀಗ, K_FOREVER);
#ಇಲ್ಲದಿದ್ದರೆ
	k_sched_ಬೀಗ();
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	ಕೀಲಿ = k_spin_ಬೀಗ(&z_mm_ಬೀಗ);
	flags = arch_ಪುಟ_info_get(addr, &phys, false);
	__ASSERT((flags & ARCH_ದತ್ತಾಂಶ_ಪುಟ_NOT_MAPPED) == 0,
		 "address %p isn't mapped", addr);
	ಆದರೆ ((flags & ARCH_ದತ್ತಾಂಶ_ಪುಟ_LOADED) == 0) {
		/* Un-mapped or already evicted. Nothing to do */
		ret = 0;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	dirty = (flags & ARCH_ದತ್ತಾಂಶ_ಪುಟ_DIRTY) != 0;
	pf = k_mem_phys_ಗಮ್ಯ_ಪುಟ_ಚೌಕಟ್ಟು(phys);
	__ASSERT(k_mem_ಪುಟ_ಚೌಕಟ್ಟು_ಗಮ್ಯ_virt(pf) == addr, "page frame address mismatch");
	ret = ಪುಟ_ಚೌಕಟ್ಟು_prepare_locked(pf, &dirty, false, &location);
	ಆದರೆ (ret != 0) {
		ಹೋಗು ಹೊರಹೋಗು;
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	k_spin_unlock(&z_mm_ಬೀಗ, ಕೀಲಿ);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	ಆದರೆ (dirty) {
		do_backing_store_ಪುಟ_ಹೊರಹೋಗು(location);
	}
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	ಕೀಲಿ = k_spin_ಬೀಗ(&z_mm_ಬೀಗ);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	ಪುಟ_ಚೌಕಟ್ಟು_free_locked(pf);
ಹೊರಹೋಗು:
	k_spin_unlock(&z_mm_ಬೀಗ, ಕೀಲಿ);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_ALLOW_IRQ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
	k_mutex_unlock(&z_mm_paging_ಬೀಗ);
#ಇಲ್ಲದಿದ್ದರೆ
	k_sched_unlock();
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ k_mem_ಪುಟ_ಹೊರಹೋಗು(ಶೂನ್ಯ *addr, size_t ಗಾತ್ರ_2)
{
	__ASSERT(ಪುಟ_ಚೌಕಟ್ಟುಗಳು_initialized, "%s called on %p too early", __func__,
		 addr);
	k_mem_assert_virtual_region(addr, ಗಾತ್ರ_2);

	ಗಾಗಿ (size_t ಸ್ಥಾನಾಂತರ = 0; ಸ್ಥಾನಾಂತರ < ಗಾತ್ರ_2; ಸ್ಥಾನಾಂತರ += CONFIG_MMU_ಪುಟ_ಗಾತ್ರ) {
		ಶೂನ್ಯ *pos = (uint8_t *)addr + ಸ್ಥಾನಾಂತರ;
		ಪೂರ್ಣಾಂಕ ret;

		ret = do_mem_evict(pos);
		ಆದರೆ (ret != 0) {
			ಹಿಂತಿರುಗಿ ret;
		}
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ k_mem_ಪುಟ_ಚೌಕಟ್ಟು_evict(uintptr_t phys)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *pf;
	bool dirty;
	uintptr_t flags;
	uintptr_t location;
	ಪೂರ್ಣಾಂಕ ret;

	__ASSERT(ಪುಟ_ಚೌಕಟ್ಟುಗಳು_initialized, "%s called on 0x%lx too early",
		 __func__, phys);

	/* Implementation is similar to do_page_fault() except there is no
	 * data page to page-in, see comments in that function.
	 */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	__ASSERT(!k_is_ಒಳ_isr(),
		 "%s is unavailable in ISRs with CONFIG_DEMAND_PAGING_ALLOW_IRQ",
		 __func__);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
	k_mutex_ಬೀಗ(&z_mm_paging_ಬೀಗ, K_FOREVER);
#ಇಲ್ಲದಿದ್ದರೆ
	k_sched_ಬೀಗ();
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	ಕೀಲಿ = k_spin_ಬೀಗ(&z_mm_ಬೀಗ);
	pf = k_mem_phys_ಗಮ್ಯ_ಪುಟ_ಚೌಕಟ್ಟು(phys);
	ಆದರೆ (!k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_mapped(pf)) {
		/* Nothing to do, free page */
		ret = 0;
		ಹೋಗು ಹೊರಹೋಗು;
	}
	flags = arch_ಪುಟ_info_get(k_mem_ಪುಟ_ಚೌಕಟ್ಟು_ಗಮ್ಯ_virt(pf), ಖಾಲಿ, false);
	/* Shouldn't ever happen */
	__ASSERT((flags & ARCH_ದತ್ತಾಂಶ_ಪುಟ_LOADED) != 0, "data page not loaded");
	dirty = (flags & ARCH_ದತ್ತಾಂಶ_ಪುಟ_DIRTY) != 0;
	ret = ಪುಟ_ಚೌಕಟ್ಟು_prepare_locked(pf, &dirty, false, &location);
	ಆದರೆ (ret != 0) {
		ಹೋಗು ಹೊರಹೋಗು;
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	k_spin_unlock(&z_mm_ಬೀಗ, ಕೀಲಿ);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	ಆದರೆ (dirty) {
		do_backing_store_ಪುಟ_ಹೊರಹೋಗು(location);
	}
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	ಕೀಲಿ = k_spin_ಬೀಗ(&z_mm_ಬೀಗ);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	ಪುಟ_ಚೌಕಟ್ಟು_free_locked(pf);
ಹೊರಹೋಗು:
	k_spin_unlock(&z_mm_ಬೀಗ, ಕೀಲಿ);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_ALLOW_IRQ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
	k_mutex_unlock(&z_mm_paging_ಬೀಗ);
#ಇಲ್ಲದಿದ್ದರೆ
	k_sched_unlock();
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ paging_stats_ದೋಷಗಳು_inc(ರಚನೆ k_thread *faulting_thread,
					   ಪೂರ್ಣಾಂಕ ಕೀಲಿ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_STATS
	bool is_irq_unlocked = arch_irq_unlocked(ಕೀಲಿ);

	paging_stats.pagefaults.cnt++;

	ಆದರೆ (is_irq_unlocked) {
		paging_stats.pagefaults.irq_unlocked++;
	} ಇಲ್ಲದಿದ್ದರೆ {
		paging_stats.pagefaults.irq_locked++;
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_THREAD_STATS
	faulting_thread->paging_stats.pagefaults.cnt++;

	ಆದರೆ (is_irq_unlocked) {
		faulting_thread->paging_stats.pagefaults.irq_unlocked++;
	} ಇಲ್ಲದಿದ್ದರೆ {
		faulting_thread->paging_stats.pagefaults.irq_locked++;
	}
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(faulting_thread);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_THREAD_STATS */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	ಆದರೆ (k_is_ಒಳ_isr()) {
		paging_stats.pagefaults.ಒಳ_isr++;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_THREAD_STATS
		faulting_thread->paging_stats.pagefaults.ಒಳ_isr++;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_THREAD_STATS */
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_STATS */
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ paging_stats_eviction_inc(ರಚನೆ k_thread *faulting_thread,
					     bool dirty)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_STATS
	ಆದರೆ (dirty) {
		paging_stats.eviction.dirty++;
	} ಇಲ್ಲದಿದ್ದರೆ {
		paging_stats.eviction.clean++;
	}
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_THREAD_STATS
	ಆದರೆ (dirty) {
		faulting_thread->paging_stats.eviction.dirty++;
	} ಇಲ್ಲದಿದ್ದರೆ {
		faulting_thread->paging_stats.eviction.clean++;
	}
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(faulting_thread);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_THREAD_STATS */
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_STATS */
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *do_eviction_select(bool *dirty)
{
	ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *pf;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	uint32_t ಸಮಯ_diff;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	timing_t ಸಮಯ_ಆರಂಭ, ಸಮಯ_ಕೊನೆ;

	ಸಮಯ_ಆರಂಭ = timing_counter_get();
#ಇಲ್ಲದಿದ್ದರೆ
	uint32_t ಸಮಯ_ಆರಂಭ;

	ಸಮಯ_ಆರಂಭ = k_ಚಕ್ರ_get_32();
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	pf = k_mem_paging_eviction_select(dirty);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	ಸಮಯ_ಕೊನೆ = timing_counter_get();
	ಸಮಯ_diff = (uint32_t)timing_ಚಕ್ರಗಳು_get(&ಸಮಯ_ಆರಂಭ, &ಸಮಯ_ಕೊನೆ);
#ಇಲ್ಲದಿದ್ದರೆ
	ಸಮಯ_diff = k_ಚಕ್ರ_get_32() - ಸಮಯ_ಆರಂಭ;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */

	z_paging_histogram_inc(&z_paging_histogram_eviction, ಸಮಯ_diff);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	ಹಿಂತಿರುಗಿ pf;
}

ಸ್ಥಿರ bool do_ಪುಟ_fault(ಶೂನ್ಯ *addr, bool pin)
{
	ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *pf;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	uintptr_t ಪುಟ_ಒಳ_location, ಪುಟ_ಹೊರಹೋಗು_location;
	ಎಣಿಕೆ arch_ಪುಟ_location ಸ್ಥಿತಿ_2;
	bool ಫಲಿತಾಂಶ;
	bool dirty = false;
	ರಚನೆ k_thread *faulting_thread;
	ಪೂರ್ಣಾಂಕ ret;

	__ASSERT(ಪುಟ_ಚೌಕಟ್ಟುಗಳು_initialized, "page fault at %p happened too early",
		 addr);

	LOG_DBG("page fault at %p", addr);

	/*
	 * TODO: Add performance accounting:
	 * - k_mem_paging_eviction_select() metrics
	 *   * periodic timer execution time histogram (if implemented)
	 */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	/*
	 * We do re-enable interrupts during the page-in/page-out operation
	 * if and only if interrupts were enabled when the exception was
	 * taken; in this configuration page faults in an ISR are a bug; all
	 * their code/data must be pinned.
	 *
	 * If interrupts were disabled when the exception was taken, the
	 * arch code is responsible for keeping them that way when entering
	 * this function.
	 *
	 * If this is not enabled, then interrupts are always locked for the
	 * entire operation. This is far worse for system interrupt latency
	 * but requires less pinned pages and ISRs may also take page faults.
	 *
	 * On UP we lock the scheduler so that other threads are never
	 * scheduled during the page-in/out operation. Support for
	 * allowing k_mem_paging_backing_store_page_out() and
	 * k_mem_paging_backing_store_page_in() to also sleep and allow
	 * other threads to run (such as in the case where the transfer is
	 * async DMA) is not supported on UP. Even if limited to thread
	 * context, arbitrary memory access triggering exceptions that put
	 * a thread to sleep on a contended page fault operation will break
	 * scheduling assumptions of cooperative threads or threads that
	 * implement critical sections with spinlocks or disabling IRQs.
	 *
	 * On SMP, though, exclusivity cannot be assumed solely from being
	 * a cooperative thread. Another thread with any prio may be running
	 * on another CPU so exclusion must already be enforced by other
	 * means. Therefore trying to prevent scheduling on SMP is pointless,
	 * and k_sched_lock()  is equivalent to a no-op on SMP anyway.
	 * As a result, sleeping/rescheduling in the SMP case is fine.
	 */
	__ASSERT(!k_is_ಒಳ_isr(), "ISR page faults are forbidden");
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
	k_mutex_ಬೀಗ(&z_mm_paging_ಬೀಗ, K_FOREVER);
#ಇಲ್ಲದಿದ್ದರೆ
	k_sched_ಬೀಗ();
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */

	ಕೀಲಿ = k_spin_ಬೀಗ(&z_mm_ಬೀಗ);
	faulting_thread = _current;

	ಸ್ಥಿತಿ_2 = arch_ಪುಟ_location_get(addr, &ಪುಟ_ಒಳ_location);
	ಆದರೆ (ಸ್ಥಿತಿ_2 == ARCH_ಪುಟ_LOCATION_BAD) {
		/* Return false to treat as a fatal error */
		ಫಲಿತಾಂಶ = false;
		ಹೋಗು ಹೊರಹೋಗು;
	}
	ಫಲಿತಾಂಶ = true;

	ಆದರೆ (ಸ್ಥಿತಿ_2 == ARCH_ಪುಟ_LOCATION_PAGED_ಒಳ) {
		ಆದರೆ (pin) {
			/* It's a physical memory address */
			uintptr_t phys = ಪುಟ_ಒಳ_location;

			pf = k_mem_phys_ಗಮ್ಯ_ಪುಟ_ಚೌಕಟ್ಟು(phys);
			ಆದರೆ (!k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_pinned(pf)) {
				ಆದರೆ (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
					k_mem_paging_eviction_remove(pf);
				}
				k_mem_ಪುಟ_ಚೌಕಟ್ಟು_ಗಣ(pf, K_MEM_ಪುಟ_ಚೌಕಟ್ಟು_PINNED);
			}
		}

		/* This if-block is to pin the page if it is
		 * already present in physical memory. There is
		 * no need to go through the following code to
		 * pull in the data pages. So skip to the end.
		 */
		ಹೋಗು ಹೊರಹೋಗು;
	}
	__ASSERT(ಸ್ಥಿತಿ_2 == ARCH_ಪುಟ_LOCATION_PAGED_ಹೊರಹೋಗು,
		 "unexpected status value %d", ಸ್ಥಿತಿ_2);

	paging_stats_ದೋಷಗಳು_inc(faulting_thread, ಕೀಲಿ.ಕೀಲಿ);

	pf = free_ಪುಟ_ಚೌಕಟ್ಟು_list_get();
	ಆದರೆ (pf == ಖಾಲಿ) {
		/* Need to evict a page frame */
		pf = do_eviction_select(&dirty);
		ಆದರೆ (pf == ಖಾಲಿ) {
			/* Every evictable page frame is pinned or busy.
			 * Fail the fault so it is reported as fatal.
			 */
			ಫಲಿತಾಂಶ = false;
			ಹೋಗು ಹೊರಹೋಗು;
		}
		LOG_DBG("evicting %p at 0x%lx",
			k_mem_ಪುಟ_ಚೌಕಟ್ಟು_ಗಮ್ಯ_virt(pf),
			k_mem_ಪುಟ_ಚೌಕಟ್ಟು_ಗಮ್ಯ_phys(pf));

		paging_stats_eviction_inc(faulting_thread, dirty);
	}
	ret = ಪುಟ_ಚೌಕಟ್ಟು_prepare_locked(pf, &dirty, true, &ಪುಟ_ಹೊರಹೋಗು_location);
	ಆದರೆ (ret != 0) {
		/* Backing store is full. Fail the fault so it is
		 * reported as fatal.
		 */
		ಫಲಿತಾಂಶ = false;
		ಹೋಗು ಹೊರಹೋಗು;
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	k_spin_unlock(&z_mm_ಬೀಗ, ಕೀಲಿ);
	/* Interrupts are now unlocked if they were not locked when we entered
	 * this function, and we may service ISRs. The scheduler is still
	 * locked.
	 */
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	ಆದರೆ (dirty) {
		do_backing_store_ಪುಟ_ಹೊರಹೋಗು(ಪುಟ_ಹೊರಹೋಗು_location);
	}
	do_backing_store_ಪುಟ_ಒಳ(ಪುಟ_ಒಳ_location);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	ಕೀಲಿ = k_spin_ಬೀಗ(&z_mm_ಬೀಗ);
	k_mem_ಪುಟ_ಚೌಕಟ್ಟು_clear(pf, K_MEM_ಪುಟ_ಚೌಕಟ್ಟು_BUSY);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	k_mem_ಪುಟ_ಚೌಕಟ್ಟು_clear(pf, K_MEM_ಪುಟ_ಚೌಕಟ್ಟು_MAPPED);
	ಚೌಕಟ್ಟು_mapped_ಗಣ(pf, addr);
	ಆದರೆ (pin) {
		k_mem_ಪುಟ_ಚೌಕಟ್ಟು_ಗಣ(pf, K_MEM_ಪುಟ_ಚೌಕಟ್ಟು_PINNED);
	}

	arch_mem_ಪುಟ_ಒಳ(addr, k_mem_ಪುಟ_ಚೌಕಟ್ಟು_ಗಮ್ಯ_phys(pf));
	k_mem_paging_backing_store_ಪುಟ_finalize(pf, ಪುಟ_ಒಳ_location);
	ಆದರೆ (IS_ENABLED(CONFIG_EVICTION_TRACKING) && (!pin)) {
		k_mem_paging_eviction_ಕೂಡಿಸು(pf);
	}
ಹೊರಹೋಗು:
	k_spin_unlock(&z_mm_ಬೀಗ, ಕೀಲಿ);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_ALLOW_IRQ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
	k_mutex_unlock(&z_mm_paging_ಬೀಗ);
#ಇಲ್ಲದಿದ್ದರೆ
	k_sched_unlock();
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */

	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

ಸ್ಥಿರ ಶೂನ್ಯ do_ಪುಟ_ಒಳ(ಶೂನ್ಯ *addr)
{
	bool ret;

	ret = do_ಪುಟ_fault(addr, false);
	ಆದರೆ (!ret) {
		/* There is no error reporting channel to the caller, and
		 * continuing would defer the failure to an arbitrary later
		 * access.
		 */
		LOG_ERR("cannot page in address %p", addr);
		k_panic();
	}
}

ಶೂನ್ಯ k_mem_ಪುಟ_ಒಳ(ಶೂನ್ಯ *addr, size_t ಗಾತ್ರ_2)
{
	__ASSERT(!IS_ENABLED(CONFIG_DEMAND_PAGING_ALLOW_IRQ) || !k_is_ಒಳ_isr(),
		 "%s may not be called in ISRs if CONFIG_DEMAND_PAGING_ALLOW_IRQ is enabled",
		 __func__);
	virt_region_foreach(addr, ಗಾತ್ರ_2, do_ಪುಟ_ಒಳ);
}

ಸ್ಥಿರ ಶೂನ್ಯ do_mem_pin(ಶೂನ್ಯ *addr)
{
	bool ret;

	ret = do_ಪುಟ_fault(addr, true);
	ಆದರೆ (!ret) {
		/* There is no error reporting channel to the caller, and
		 * continuing with the memory not actually pinned would
		 * defer the failure to an arbitrary later access, possibly
		 * from a context that cannot handle a page fault.
		 */
		LOG_ERR("cannot pin address %p", addr);
		k_panic();
	}
}

ಶೂನ್ಯ k_mem_pin(ಶೂನ್ಯ *addr, size_t ಗಾತ್ರ_2)
{
	__ASSERT(!IS_ENABLED(CONFIG_DEMAND_PAGING_ALLOW_IRQ) || !k_is_ಒಳ_isr(),
		 "%s may not be called in ISRs if CONFIG_DEMAND_PAGING_ALLOW_IRQ is enabled",
		 __func__);
	virt_region_foreach(addr, ಗಾತ್ರ_2, do_mem_pin);
}

bool k_mem_ಪುಟ_fault(ಶೂನ್ಯ *addr)
{
	ಹಿಂತಿರುಗಿ do_ಪುಟ_fault(addr, false);
}

ಸ್ಥಿರ ಶೂನ್ಯ do_mem_unpin(ಶೂನ್ಯ *addr)
{
	ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *pf;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	uintptr_t flags, phys;

	ಕೀಲಿ = k_spin_ಬೀಗ(&z_mm_ಬೀಗ);
	flags = arch_ಪುಟ_info_get(addr, &phys, false);
	__ASSERT((flags & ARCH_ದತ್ತಾಂಶ_ಪುಟ_NOT_MAPPED) == 0,
		 "invalid data page at %p", addr);
	ಆದರೆ ((flags & ARCH_ದತ್ತಾಂಶ_ಪುಟ_LOADED) != 0) {
		pf = k_mem_phys_ಗಮ್ಯ_ಪುಟ_ಚೌಕಟ್ಟು(phys);
		ಆದರೆ (k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_pinned(pf)) {
			k_mem_ಪುಟ_ಚೌಕಟ್ಟು_clear(pf, K_MEM_ಪುಟ_ಚೌಕಟ್ಟು_PINNED);

			ಆದರೆ (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
				k_mem_paging_eviction_ಕೂಡಿಸು(pf);
			}
		}
	}
	k_spin_unlock(&z_mm_ಬೀಗ, ಕೀಲಿ);
}

ಶೂನ್ಯ k_mem_unpin(ಶೂನ್ಯ *addr, size_t ಗಾತ್ರ_2)
{
	__ASSERT(ಪುಟ_ಚೌಕಟ್ಟುಗಳು_initialized, "%s called on %p too early", __func__,
		 addr);
	virt_region_foreach(addr, ಗಾತ್ರ_2, do_mem_unpin);
}

#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING */
