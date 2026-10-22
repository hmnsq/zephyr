/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Routines for managing virtual address spaces
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kernel_arch_interface.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/spinlock.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <mmu.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/init.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kernel_internal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/internal/syscall_handler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/toolchain.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/linker/linker-defs.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/bitarray.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/check.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/math_extras.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/minmax.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/timing/timing.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/arch/common/init.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/logging/log.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel/mm/demand_paging.h>
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING */

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
ਬਣਤਰ k_spinlock z_mm_ਤਾਲਾ;

/*
 * General page frame management
 */

/* Database of all RAM page frames */
ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ k_mem_ਪੰਨਾ_ਚੌਖਟੇ[K_MEM_ਅੰਕ_ਪੰਨਾ_ਚੌਖਟੇ];

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ASSERT
/* Indicator that k_mem_page_frames has been initialized, many of these APIs do
 * not work before POST_KERNEL
 */
ਸਥਿਰ bool ਪੰਨਾ_ਚੌਖਟੇ_initialized;
#ਜੇ_ਅੰਤ /* CONFIG_ASSERT */

/* Add colors to page table dumps to indicate mapping type */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COLOR_ਪੰਨਾ_ਚੌਖਟੇ	1

#ਜੇ COLOR_ਪੰਨਾ_ਚੌਖਟੇ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ANSI_DEFAULT "\x1B" "[0m"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ANSI_RED     "\x1B" "[1;31m"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ANSI_GREEN   "\x1B" "[1;32m"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ANSI_YELLOW  "\x1B" "[1;33m"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ANSI_BLUE    "\x1B" "[1;34m"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ANSI_MAGENTA "\x1B" "[1;35m"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ANSI_CYAN    "\x1B" "[1;36m"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ANSI_GREY    "\x1B" "[1;90m"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COLOR(x)	printk(_CONCAT(ANSI_, x))
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COLOR(x)	ਕਰੋ { } ਜਦੋਂ (false)
#ਜੇ_ਅੰਤ /* COLOR_PAGE_FRAMES */

/* LCOV_EXCL_START */
ਸਥਿਰ ਖਾਲੀ ਪੰਨਾ_ਚੌਖਟਾ_dump(ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *pf)
{
	ਜੇ (k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_free(pf)) {
		COLOR(GREY);
		printk("-");
	} ਨਹੀਂਤਾਂ ਜੇ (k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_reserved(pf)) {
		COLOR(CYAN);
		printk("R");
	} ਨਹੀਂਤਾਂ ਜੇ (k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_busy(pf)) {
		COLOR(MAGENTA);
		printk("B");
	} ਨਹੀਂਤਾਂ ਜੇ (k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_pinned(pf)) {
		COLOR(YELLOW);
		printk("P");
	} ਨਹੀਂਤਾਂ ਜੇ (k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_ਉਪਲਬਧ(pf)) {
		COLOR(GREY);
		printk(".");
	} ਨਹੀਂਤਾਂ ਜੇ (k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_mapped(pf)) {
		COLOR(DEFAULT);
		printk("M");
	} ਨਹੀਂਤਾਂ {
		COLOR(RED);
		printk("?");
	}
}

ਖਾਲੀ k_mem_ਪੰਨਾ_ਚੌਖਟੇ_dump(ਖਾਲੀ)
{
	ਪੂਰਨ ਕਾਲਮ = 0;

	__ASSERT(ਪੰਨਾ_ਚੌਖਟੇ_initialized, "%s called too early", __func__);
	printk("Physical memory from 0x%lx to 0x%lx\n",
	       K_MEM_PHYS_RAM_ਸ਼ੁਰੂ, K_MEM_PHYS_RAM_ਸਿਰਾ);

	ਲਈ (ਪੂਰਨ i = 0; i < K_MEM_ਅੰਕ_ਪੰਨਾ_ਚੌਖਟੇ; i++) {
		ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *pf = &k_mem_ਪੰਨਾ_ਚੌਖਟੇ[i];

		ਪੰਨਾ_ਚੌਖਟਾ_dump(pf);

		ਕਾਲਮ++;
		ਜੇ (ਕਾਲਮ == 64) {
			ਕਾਲਮ = 0;
			printk("\n");
		}
	}

	COLOR(DEFAULT);
	ਜੇ (ਕਾਲਮ != 0) {
		printk("\n");
	}
}
/* LCOV_EXCL_STOP */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VIRT_FOREACH(ਅਧਾਰ_2, ਮਾਪ_2, _pos) \
	ਲਈ ((_pos) = (ਅਧਾਰ_2); \
	     (_pos) < ((uint8_t *)(ਅਧਾਰ_2) + (ਮਾਪ_2)); (_pos) += CONFIG_MMU_ਪੰਨਾ_ਮਾਪ)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PHYS_FOREACH(ਅਧਾਰ_2, ਮਾਪ_2, _pos) \
	ਲਈ ((_pos) = (ਅਧਾਰ_2); \
	     (_pos) < ((uintptr_t)(ਅਧਾਰ_2) + (ਮਾਪ_2)); (_pos) += CONFIG_MMU_ਪੰਨਾ_ਮਾਪ)


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
			   CONFIG_KERNEL_VM_ਮਾਪ / CONFIG_MMU_ਪੰਨਾ_ਮਾਪ);

ਸਥਿਰ bool virt_region_inited;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Z_VIRT_REGION_ਸ਼ੁਰੂ_ADDR	K_MEM_VM_FREE_ਸ਼ੁਰੂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Z_VIRT_REGION_ਸਿਰਾ_ADDR		(K_MEM_VIRT_RAM_ਸਿਰਾ - K_MEM_VM_RESERVED)

ਸਥਿਰ ਸਤਰਅੰਦਰ uintptr_t virt_from_bitmap_ਵਿੱਥ(size_t ਵਿੱਥ, size_t ਮਾਪ)
{
	ਵਾਪਸ ਸੂਚਕ_ਮੰਜ਼ਿਲ_UINT(K_MEM_VIRT_RAM_ਸਿਰਾ)
	       - (ਵਿੱਥ * CONFIG_MMU_ਪੰਨਾ_ਮਾਪ) - ਮਾਪ;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ size_t virt_ਮੰਜ਼ਿਲ_bitmap_ਵਿੱਥ(ਖਾਲੀ *vaddr, size_t ਮਾਪ)
{
	ਵਾਪਸ (ਸੂਚਕ_ਮੰਜ਼ਿਲ_UINT(K_MEM_VIRT_RAM_ਸਿਰਾ)
		- ਸੂਚਕ_ਮੰਜ਼ਿਲ_UINT(vaddr) - ਮਾਪ) / CONFIG_MMU_ਪੰਨਾ_ਮਾਪ;
}

ਸਥਿਰ ਖਾਲੀ virt_region_init(ਖਾਲੀ)
{
	size_t ਵਿੱਥ, ਅੰਕ_ਬਿੱਟ;

	/* There are regions where we should never map via
	 * k_mem_map() and k_mem_map_phys_bare(). Mark them as
	 * already allocated so they will never be used.
	 */

	ਜੇ (K_MEM_VM_RESERVED > 0) {
		/* Mark reserved region at end of virtual address space */
		ਅੰਕ_ਬਿੱਟ = K_MEM_VM_RESERVED / CONFIG_MMU_ਪੰਨਾ_ਮਾਪ;
		(ਖਾਲੀ)sys_bitarray_ਸੈੱਟ_region(&virt_region_bitmap,
					      ਅੰਕ_ਬਿੱਟ, 0);
	}

	/* Mark all bits up to Z_FREE_VM_START as allocated */
	ਅੰਕ_ਬਿੱਟ = ਸੂਚਕ_ਮੰਜ਼ਿਲ_UINT(K_MEM_VM_FREE_ਸ਼ੁਰੂ)
		   - ਸੂਚਕ_ਮੰਜ਼ਿਲ_UINT(K_MEM_VIRT_RAM_ਸ਼ੁਰੂ);
	ਵਿੱਥ = virt_ਮੰਜ਼ਿਲ_bitmap_ਵਿੱਥ(K_MEM_VIRT_RAM_ਸ਼ੁਰੂ, ਅੰਕ_ਬਿੱਟ);
	ਅੰਕ_ਬਿੱਟ /= CONFIG_MMU_ਪੰਨਾ_ਮਾਪ;
	(ਖਾਲੀ)sys_bitarray_ਸੈੱਟ_region(&virt_region_bitmap,
				      ਅੰਕ_ਬਿੱਟ, ਵਿੱਥ);

	virt_region_inited = true;
}

ਸਥਿਰ ਖਾਲੀ virt_region_free(ਖਾਲੀ *vaddr, size_t ਮਾਪ)
{
	size_t ਵਿੱਥ, ਅੰਕ_ਬਿੱਟ;
	uint8_t *vaddr_u8 = (uint8_t *)vaddr;

	ਜੇ (unlikely(!virt_region_inited)) {
		virt_region_init();
	}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_KERNEL_ਸਿੱਧਾ_MAP
	/* Without the need to support K_MEM_DIRECT_MAP, the region must be
	 * able to be represented in the bitmap. So this case is
	 * simple.
	 */

	__ASSERT((vaddr_u8 >= Z_VIRT_REGION_ਸ਼ੁਰੂ_ADDR)
		 && ((vaddr_u8 + ਮਾਪ - 1) < Z_VIRT_REGION_ਸਿਰਾ_ADDR),
		 "invalid virtual address region %p (%zu)", vaddr_u8, ਮਾਪ);
	ਜੇ (!((vaddr_u8 >= Z_VIRT_REGION_ਸ਼ੁਰੂ_ADDR)
	      && ((vaddr_u8 + ਮਾਪ - 1) < Z_VIRT_REGION_ਸਿਰਾ_ADDR))) {
		ਵਾਪਸ;
	}

	ਵਿੱਥ = virt_ਮੰਜ਼ਿਲ_bitmap_ਵਿੱਥ(vaddr, ਮਾਪ);
	ਅੰਕ_ਬਿੱਟ = ਮਾਪ / CONFIG_MMU_ਪੰਨਾ_ਮਾਪ;
	(ਖਾਲੀ)sys_bitarray_free(&virt_region_bitmap, ਅੰਕ_ਬਿੱਟ, ਵਿੱਥ);
#ਨਹੀਂਤਾਂ /* !CONFIG_KERNEL_DIRECT_MAP */
	/* With K_MEM_DIRECT_MAP, the region can be outside of the virtual
	 * memory space, wholly within it, or overlap partially.
	 * So additional processing is needed to make sure we only
	 * mark the pages within the bitmap.
	 */
	ਜੇ (((vaddr_u8 >= Z_VIRT_REGION_ਸ਼ੁਰੂ_ADDR) &&
	     (vaddr_u8 < Z_VIRT_REGION_ਸਿਰਾ_ADDR)) ||
	    (((vaddr_u8 + ਮਾਪ - 1) >= Z_VIRT_REGION_ਸ਼ੁਰੂ_ADDR) &&
	     ((vaddr_u8 + ਮਾਪ - 1) < Z_VIRT_REGION_ਸਿਰਾ_ADDR))) {
		uint8_t *adjusted_ਸ਼ੁਰੂ = max(vaddr_u8, Z_VIRT_REGION_ਸ਼ੁਰੂ_ADDR);
		uint8_t *adjusted_ਸਿਰਾ = min(vaddr_u8 + ਮਾਪ,
					    Z_VIRT_REGION_ਸਿਰਾ_ADDR);
		size_t adjusted_sz = adjusted_ਸਿਰਾ - adjusted_ਸ਼ੁਰੂ;

		ਵਿੱਥ = virt_ਮੰਜ਼ਿਲ_bitmap_ਵਿੱਥ(adjusted_ਸ਼ੁਰੂ, adjusted_sz);
		ਅੰਕ_ਬਿੱਟ = adjusted_sz / CONFIG_MMU_ਪੰਨਾ_ਮਾਪ;
		(ਖਾਲੀ)sys_bitarray_free(&virt_region_bitmap, ਅੰਕ_ਬਿੱਟ, ਵਿੱਥ);
	}
#ਜੇ_ਅੰਤ /* !CONFIG_KERNEL_DIRECT_MAP */
}

ਸਥਿਰ ਖਾਲੀ *virt_region_alloc(size_t ਮਾਪ, size_t align)
{
	uintptr_t dest_addr;
	size_t alloc_ਮਾਪ;
	size_t ਵਿੱਥ;
	size_t ਅੰਕ_ਬਿੱਟ;
	ਪੂਰਨ ret;

	ਜੇ (unlikely(!virt_region_inited)) {
		virt_region_init();
	}

	/* Possibly request more pages to ensure we can get an aligned virtual address */
	ਅੰਕ_ਬਿੱਟ = (ਮਾਪ + align - CONFIG_MMU_ਪੰਨਾ_ਮਾਪ) / CONFIG_MMU_ਪੰਨਾ_ਮਾਪ;
	alloc_ਮਾਪ = ਅੰਕ_ਬਿੱਟ * CONFIG_MMU_ਪੰਨਾ_ਮਾਪ;
	ret = sys_bitarray_alloc(&virt_region_bitmap, ਅੰਕ_ਬਿੱਟ, &ਵਿੱਥ);
	ਜੇ (ret != 0) {
		LOG_ERR("insufficient virtual address space (requested %zu)",
			ਮਾਪ);
		ਵਾਪਸ ਨਿਰਾ;
	}

	/* Remember that bit #0 in bitmap corresponds to the highest
	 * virtual address. So here we need to go downwards (backwards?)
	 * to get the starting address of the allocated region.
	 */
	dest_addr = virt_from_bitmap_ਵਿੱਥ(ਵਿੱਥ, alloc_ਮਾਪ);

	ਜੇ (alloc_ਮਾਪ > ਮਾਪ) {
		uintptr_t aligned_dest_addr = ਗੇੜਾ_ਉੱਪਰ(dest_addr, align);

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
		virt_region_free(UINT_ਮੰਜ਼ਿਲ_ਸੂਚਕ(dest_addr),
				 aligned_dest_addr - dest_addr);
		ਜੇ (((dest_addr + alloc_ਮਾਪ) - (aligned_dest_addr + ਮਾਪ)) > 0) {
			virt_region_free(UINT_ਮੰਜ਼ਿਲ_ਸੂਚਕ(aligned_dest_addr + ਮਾਪ),
					 (dest_addr + alloc_ਮਾਪ) - (aligned_dest_addr + ਮਾਪ));
		}

		dest_addr = aligned_dest_addr;
	}

	/* Need to make sure this does not step into kernel memory */
	ਜੇ (dest_addr < ਸੂਚਕ_ਮੰਜ਼ਿਲ_UINT(Z_VIRT_REGION_ਸ਼ੁਰੂ_ADDR)) {
		(ਖਾਲੀ)sys_bitarray_free(&virt_region_bitmap, ਅੰਕ_ਬਿੱਟ, ਵਿੱਥ);
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਵਾਪਸ UINT_ਮੰਜ਼ਿਲ_ਸੂਚਕ(dest_addr);
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
BUILD_ASSERT(SYS_SFLIST_FLAG_ਬਿੱਟ >= 1, "MMU free-page list needs one sflist flag bit");

ਸਥਿਰ sys_sflist_t free_ਪੰਨਾ_ਚੌਖਟਾ_list;

/* Number of unused and available free page frames.
 * This information may go stale immediately.
 */
ਸਥਿਰ size_t z_free_ਪੰਨਾ_ਗਿਣਤੀ;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PF_ASSERT(pf, ਵਿਅੰਜਕ, fmt, ...) \
	__ASSERT(ਵਿਅੰਜਕ, "page frame 0x%lx: " fmt, k_mem_ਪੰਨਾ_ਚੌਖਟਾ_ਮੰਜ਼ਿਲ_phys(pf), \
		 ##__VA_ARGS__)

/* Get an unused page frame. don't care which one, or NULL if there are none */
ਸਥਿਰ ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *free_ਪੰਨਾ_ਚੌਖਟਾ_list_get(ਖਾਲੀ)
{
	sys_sfnode_t *ਗੰਢ;
	ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *pf = ਨਿਰਾ;

	ਗੰਢ = sys_sflist_get(&free_ਪੰਨਾ_ਚੌਖਟਾ_list);
	ਜੇ (ਗੰਢ != ਨਿਰਾ) {
		z_free_ਪੰਨਾ_ਗਿਣਤੀ--;
		pf = CONTAINER_ਦਾ(ਗੰਢ, ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ, ਗੰਢ);
		PF_ASSERT(pf, k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_free(pf),
			 "on free list but not free");
		pf->va_and_flags = 0;
	}

	ਵਾਪਸ pf;
}

/* Release a page frame back into the list of free pages */
ਸਥਿਰ ਖਾਲੀ free_ਪੰਨਾ_ਚੌਖਟਾ_list_put(ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *pf)
{
	PF_ASSERT(pf, k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_ਉਪਲਬਧ(pf),
		 "unavailable page put on free list");

	sys_sfnode_init(&pf->ਗੰਢ, K_MEM_ਪੰਨਾ_ਚੌਖਟਾ_FREE);
	sys_sflist_append(&free_ਪੰਨਾ_ਚੌਖਟਾ_list, &pf->ਗੰਢ);
	z_free_ਪੰਨਾ_ਗਿਣਤੀ++;
}

ਸਥਿਰ ਖਾਲੀ free_ਪੰਨਾ_ਚੌਖਟਾ_list_init(ਖਾਲੀ)
{
	sys_sflist_init(&free_ਪੰਨਾ_ਚੌਖਟਾ_list);
}

ਸਥਿਰ ਖਾਲੀ ਪੰਨਾ_ਚੌਖਟਾ_free_locked(ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *pf)
{
	pf->va_and_flags = 0;
	free_ਪੰਨਾ_ਚੌਖਟਾ_list_put(pf);
}

/*
 * Memory Mapping
 */

/* Called after the frame is mapped in the arch layer, to update our
 * local ontology (and do some assertions while we're at it)
 */
ਸਥਿਰ ਖਾਲੀ ਚੌਖਟਾ_mapped_ਸੈੱਟ(ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *pf, ਖਾਲੀ *addr)
{
	PF_ASSERT(pf, !k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_free(pf),
		  "attempted to map a page frame on the free list");
	PF_ASSERT(pf, !k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_reserved(pf),
		  "attempted to map a reserved page frame");

	/* We do allow multiple mappings for pinned page frames
	 * since we will never need to reverse map them.
	 * This is uncommon, use-cases are for things like the
	 * Zephyr equivalent of VSDOs
	 */
	PF_ASSERT(pf, !k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_mapped(pf) || k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_pinned(pf),
		 "non-pinned and already mapped to %p",
		 k_mem_ਪੰਨਾ_ਚੌਖਟਾ_ਮੰਜ਼ਿਲ_virt(pf));

	uintptr_t flags_ਨਕਾਬ = CONFIG_MMU_ਪੰਨਾ_ਮਾਪ - 1;
	uintptr_t va = (uintptr_t)addr & ~flags_ਨਕਾਬ;

	pf->va_and_flags &= flags_ਨਕਾਬ;
	pf->va_and_flags |= va | K_MEM_ਪੰਨਾ_ਚੌਖਟਾ_MAPPED;
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
ਸਥਿਰ ਪੂਰਨ virt_ਮੰਜ਼ਿਲ_ਪੰਨਾ_ਚੌਖਟਾ(ਖਾਲੀ *virt, uintptr_t *phys)
{
	uintptr_t paddr;
	ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *pf;
	ਪੂਰਨ ret = -EFAULT;

	K_MEM_ਪੰਨਾ_ਚੌਖਟਾ_FOREACH(paddr, pf) {
		ਜੇ (k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_mapped(pf)) {
			ਜੇ (virt == k_mem_ਪੰਨਾ_ਚੌਖਟਾ_ਮੰਜ਼ਿਲ_virt(pf)) {
				ret = 0;
				ਜੇ (phys != ਨਿਰਾ) {
					*phys = k_mem_ਪੰਨਾ_ਚੌਖਟਾ_ਮੰਜ਼ਿਲ_phys(pf);
				}
				ਰੋਕੋ;
			}
		}
	}

	ਵਾਪਸ ret;
}
/* LCOV_EXCL_STOP */

__weak FUNC_ALIAS(virt_ਮੰਜ਼ਿਲ_ਪੰਨਾ_ਚੌਖਟਾ, arch_ਪੰਨਾ_phys_get, ਪੂਰਨ);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING
ਸਥਿਰ ਪੂਰਨ ਪੰਨਾ_ਚੌਖਟਾ_prepare_locked(ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *pf, bool *dirty_ptr,
				     bool ਪੰਨਾ_ਵਿੱਚ, uintptr_t *location_ptr);

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ do_backing_store_ਪੰਨਾ_ਵਿੱਚ(uintptr_t location);
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ do_backing_store_ਪੰਨਾ_ਬਾਹਰਲਾ(uintptr_t location);
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING */

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
ਸਥਿਰ ਪੂਰਨ map_anon_ਪੰਨਾ(ਖਾਲੀ *addr, uint32_t flags, bool *recovered)
{
	ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *pf;
	uintptr_t phys;
	bool ਤਾਲਾ = (flags & K_MEM_MAP_ਤਾਲਾ) != 0U;
	ਪੂਰਨ ret;

	*recovered = true;

	pf = free_ਪੰਨਾ_ਚੌਖਟਾ_list_get();
	ਜੇ (pf == ਨਿਰਾ) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING
		uintptr_t location;
		bool dirty;

		pf = k_mem_paging_eviction_select(&dirty);
		ਜੇ (pf == ਨਿਰਾ) {
			/* Every evictable page frame is pinned or busy */
			ਵਾਪਸ -ENOMEM;
		}
		LOG_DBG("evicting %p at 0x%lx",
			k_mem_ਪੰਨਾ_ਚੌਖਟਾ_ਮੰਜ਼ਿਲ_virt(pf),
			k_mem_ਪੰਨਾ_ਚੌਖਟਾ_ਮੰਜ਼ਿਲ_phys(pf));
		ret = ਪੰਨਾ_ਚੌਖਟਾ_prepare_locked(pf, &dirty, false, &location);
		ਜੇ (ret != 0) {
			ਵਾਪਸ -ENOMEM;
		}
		ਜੇ (dirty) {
			do_backing_store_ਪੰਨਾ_ਬਾਹਰਲਾ(location);
		}
		pf->va_and_flags = 0;
#ਨਹੀਂਤਾਂ
		ਵਾਪਸ -ENOMEM;
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING */
	}

	phys = k_mem_ਪੰਨਾ_ਚੌਖਟਾ_ਮੰਜ਼ਿਲ_phys(pf);
	ret = arch_mem_map(addr, phys, CONFIG_MMU_ਪੰਨਾ_ਮਾਪ, flags);
	__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
	ਜੇ (ret != 0) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
		/* The mapping above may have already succeeded in some
		 * memory domains but failing in the middle. So we need to
		 * unmap those first before returning the page frame back
		 * to the list.
		 */
		ਪੂਰਨ unmap_ret = arch_mem_unmap(addr, CONFIG_MMU_ਪੰਨਾ_ਮਾਪ);

		__ASSERT(unmap_ret == 0, "%s: cannot undo failed mapping %d\n", __func__,
			 unmap_ret);

		ਜੇ (unmap_ret != 0) {
			/* The page may still be mapped in some memory domains,
			 * so the page frame cannot be given back to the free
			 * list for reuse.
			 */
			*recovered = false;
			ਵਾਪਸ ret;
		}
#ਜੇ_ਅੰਤ

		/* Mapping failed: give the page frame back before bailing out. */
		ਪੰਨਾ_ਚੌਖਟਾ_free_locked(pf);
		ਵਾਪਸ ret;
	}

	ਜੇ (ਤਾਲਾ) {
		k_mem_ਪੰਨਾ_ਚੌਖਟਾ_ਸੈੱਟ(pf, K_MEM_ਪੰਨਾ_ਚੌਖਟਾ_PINNED);
	}
	ਚੌਖਟਾ_mapped_ਸੈੱਟ(pf, addr);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING
	ਜੇ (IS_ENABLED(CONFIG_EVICTION_TRACKING) && (!ਤਾਲਾ)) {
		k_mem_paging_eviction_ਜੋੜੋ(pf);
	}
#ਜੇ_ਅੰਤ

	LOG_DBG("memory mapping anon page %p -> 0x%lx", addr, phys);

	ਵਾਪਸ 0;
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
ਸਥਿਰ ਪੂਰਨ unmap_anon_ਪੰਨੇ(ਖਾਲੀ *virt, size_t sz)
{
	uint8_t *pos;
	uintptr_t phys;
	ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *pf;
	ਪੂਰਨ ret = 0;

	VIRT_FOREACH(virt, sz, pos) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING
		ਗਿਣਤੀ arch_ਪੰਨਾ_location ਸਥਿਤੀ;
		uintptr_t location;

		ਸਥਿਤੀ = arch_ਪੰਨਾ_location_get(pos, &location);
		ਚੋਣ (ਸਥਿਤੀ) {
		ਹਾਲਤ ARCH_ਪੰਨਾ_LOCATION_PAGED_ਬਾਹਰਲਾ:
			/*
			 * No pf is associated with this mapping.
			 * Simply get rid of the MMU entry and free
			 * corresponding backing store.
			 */
			ret = arch_mem_unmap(pos, CONFIG_MMU_ਪੰਨਾ_ਮਾਪ);
			__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__,
				 ret);

			k_mem_paging_backing_store_location_free(location);

			ਜੇ (ret != 0) {
				ਜਾਓ unmap_anon_ਬਾਹਰਲਾ;
			}

			ਜਾਰੀ;
		ਹਾਲਤ ARCH_ਪੰਨਾ_LOCATION_PAGED_ਵਿੱਚ:
			/*
			 * The page is in memory but it may not be
			 * accessible in order to manage tracking
			 * of the ARCH_DATA_PAGE_ACCESSED flag
			 * meaning arch_page_phys_get() could fail.
			 * Still, we know the actual phys address.
			 */
			phys = location;
			ret = 0;
			ਰੋਕੋ;
		ਮੂਲ:
			ret = arch_ਪੰਨਾ_phys_get(pos, &phys);
			ਰੋਕੋ;
		}
#ਨਹੀਂਤਾਂ
		ret = arch_ਪੰਨਾ_phys_get(pos, &phys);
#ਜੇ_ਅੰਤ
		__ASSERT(ret == 0, "%s: cannot unmap an unmapped address %p", __func__, pos);
		ਜੇ (ret != 0) {
			/* Found an address not mapped. Do not continue. */
			ਜਾਓ unmap_anon_ਬਾਹਰਲਾ;
		}

		__ASSERT(k_mem_is_ਪੰਨਾ_ਚੌਖਟਾ(phys), "%s: 0x%lx is not a page frame", __func__,
			 phys);
		ਜੇ (!k_mem_is_ਪੰਨਾ_ਚੌਖਟਾ(phys)) {
			/* Physical address has no corresponding page frame
			 * description in the page frame array.
			 * This should not happen. Do not continue.
			 */
			ਜਾਓ unmap_anon_ਬਾਹਰਲਾ;
		}

		/* Grab the corresponding page frame from physical address */
		pf = k_mem_phys_ਮੰਜ਼ਿਲ_ਪੰਨਾ_ਚੌਖਟਾ(phys);

		__ASSERT(k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_mapped(pf), "%s: 0x%lx is not a mapped page frame",
			 __func__, phys);
		ਜੇ (!k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_mapped(pf)) {
			/* Page frame is not marked mapped.
			 * This should not happen. Do not continue.
			 */
			ਜਾਓ unmap_anon_ਬਾਹਰਲਾ;
		}

		ret = arch_mem_unmap(pos, CONFIG_MMU_ਪੰਨਾ_ਮਾਪ);
		__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
		ਜੇ (ret != 0) {
			/* Fail to unmap. Must bail without releasing any resources
			 * as the page is still mapped.
			 */
			ਜਾਓ unmap_anon_ਬਾਹਰਲਾ;
		}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING
		ਜੇ (IS_ENABLED(CONFIG_EVICTION_TRACKING) &&
		    (!k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_pinned(pf))) {
			k_mem_paging_eviction_remove(pf);
		}
#ਜੇ_ਅੰਤ

		/* Put the page frame back into free list */
		ਪੰਨਾ_ਚੌਖਟਾ_free_locked(pf);
	}

unmap_anon_ਬਾਹਰਲਾ:
	ਵਾਪਸ ret;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_MAPPING
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
ਸਥਿਰ bool undo_demand_mapped_ਪੰਨੇ(ਖਾਲੀ *virt, size_t sz)
{
	uint8_t *undo_pos;
	bool success = true;
	ਪੂਰਨ ret;

	/* Undo the pages mapped before the failure. */
	VIRT_FOREACH(virt, sz, undo_pos) {
		ret = arch_mem_unmap(undo_pos, CONFIG_MMU_ਪੰਨਾ_ਮਾਪ);
		__ASSERT(ret == 0, "%s: cannot undo mapped pages %d", __func__, ret);

		/* Failed to undo one mapped page. Though we still
		 * want to continue unmapping the remaining mapped
		 * pages to reclaim them.
		 */
		ਜੇ (ret != 0) {
			success = false;
		}
	}

	ਵਾਪਸ success;
}
#ਜੇ_ਅੰਤ

ਖਾਲੀ *k_mem_map_phys_guard(uintptr_t phys, size_t ਮਾਪ, uint32_t flags, bool is_anon)
{
	uint8_t *dst;
	uint8_t *region;
	size_t ਕੁੱਲ_ਮਾਪ;
	ਪੂਰਨ ret;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;
	uint8_t *pos;
	bool uninit = (flags & K_MEM_MAP_UNINIT) != 0U;

	__ASSERT(!is_anon || (is_anon && ਪੰਨਾ_ਚੌਖਟੇ_initialized),
		 "%s called too early", __func__);
	__ASSERT((flags & K_MEM_CACHE_ਨਕਾਬ) == 0U,
		 "%s does not support explicit cache settings", __func__);

	ਜੇ (((flags & K_MEM_PERM_USER) != 0U) &&
	    ((flags & K_MEM_MAP_UNINIT) != 0U)) {
		LOG_ERR("user access to anonymous uninitialized pages is forbidden");
		ਵਾਪਸ ਨਿਰਾ;
	}
	ਜੇ ((ਮਾਪ % CONFIG_MMU_ਪੰਨਾ_ਮਾਪ) != 0U) {
		LOG_ERR("unaligned size %zu passed to %s", ਮਾਪ, __func__);
		ਵਾਪਸ ਨਿਰਾ;
	}
	ਜੇ (ਮਾਪ == 0) {
		LOG_ERR("zero sized memory mapping");
		ਵਾਪਸ ਨਿਰਾ;
	}

	/* Need extra for the guard pages (before and after) which we
	 * won't map.
	 */
	ਜੇ (ਮਾਪ_ਜੋੜੋ_ਉਛਾਲ(ਮਾਪ, CONFIG_MMU_ਪੰਨਾ_ਮਾਪ * 2, &ਕੁੱਲ_ਮਾਪ)) {
		LOG_ERR("too large size %zu passed to %s", ਮਾਪ, __func__);
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&z_mm_ਤਾਲਾ);

	region = virt_region_alloc(ਕੁੱਲ_ਮਾਪ, CONFIG_MMU_ਪੰਨਾ_ਮਾਪ);
	ਜੇ (region == ਨਿਰਾ) {
		/* Address space has no free region */
		dst = ਨਿਰਾ;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	/* Unmap both guard pages to make sure accessing them
	 * will generate fault.
	 *
	 * Note that a failed unmap may have left the guard page mapped in
	 * some memory domains, so the virtual region cannot be freed for
	 * reuse.
	 */
	ret = arch_mem_unmap(region, CONFIG_MMU_ਪੰਨਾ_ਮਾਪ);
	__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
	ਜੇ (ret != 0) {
		dst = ਨਿਰਾ;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ret = arch_mem_unmap(region + CONFIG_MMU_ਪੰਨਾ_ਮਾਪ + ਮਾਪ, CONFIG_MMU_ਪੰਨਾ_ਮਾਪ);
	__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
	ਜੇ (ret != 0) {
		dst = ਨਿਰਾ;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	/* Skip over the "before" guard page in returned address. */
	dst = region + CONFIG_MMU_ਪੰਨਾ_ਮਾਪ;

	ਜੇ (is_anon) {
		/* Mapping from anonymous memory */
		flags |= K_MEM_CACHE_WB;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_MAPPING
		ਜੇ ((flags & K_MEM_MAP_ਤਾਲਾ) == 0) {
			flags |= K_MEM_MAP_UNPAGED;
			VIRT_FOREACH(dst, ਮਾਪ, pos) {
				ret = arch_mem_map(pos,
						   uninit ? ARCH_UNPAGED_ANON_UNINIT
							  : ARCH_UNPAGED_ANON_ZERO,
						   CONFIG_MMU_ਪੰਨਾ_ਮਾਪ, flags);
				__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
				ਜੇ (ret != 0) {
					size_t unmap_sz = pos - dst;

					ਜੇ (IS_ENABLED(CONFIG_USERSPACE)) {
						/* Note that it is unmapping an extra page due to
						 * the possibility that the mapping above has
						 * already updated some memory domains but fail in
						 * the middle. So we need to include the failed
						 * page to unmap those that have been mapped so
						 * far.
						 */
						unmap_sz += CONFIG_MMU_ਪੰਨਾ_ਮਾਪ;
					}

					bool success = undo_demand_mapped_ਪੰਨੇ(dst, unmap_sz);

					dst = ਨਿਰਾ;

					ਜੇ (success) {
						ਜਾਓ fail;
					} ਨਹੀਂਤਾਂ {
						/* Cannot fully recover from mapping
						 * failure. We cannot free the allocated
						 * virtual memory region since some of
						 * the page may still be mapped.
						 */
						ਜਾਓ ਬਾਹਰਲਾ;
					}
				}
			}
			LOG_DBG("memory mapping anon pages %p to %p unpaged", dst, pos-1);
			/* skip the memset() below */
			uninit = true;
		} ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
		{
			VIRT_FOREACH(dst, ਮਾਪ, pos) {
				bool recovered;

				ret = map_anon_ਪੰਨਾ(pos, flags, &recovered);

				ਜੇ (ret != 0) {
					ਪੂਰਨ unmap_ret = unmap_anon_ਪੰਨੇ(dst, pos - dst);

					dst = ਨਿਰਾ;

					ਜੇ ((unmap_ret == 0) && recovered) {
						ਜਾਓ fail;
					} ਨਹੀਂਤਾਂ {
						/* Cannot fully recover from mapping
						 * failure. We cannot free the allocated
						 * virtual memory region since some of
						 * the page may still be mapped.
						 */
						ਜਾਓ ਬਾਹਰਲਾ;
					}
				}
			}
		}
	} ਨਹੀਂਤਾਂ {
		/* Mapping known physical memory. */
		ret = arch_mem_map(dst, phys, ਮਾਪ, flags);
		__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
		ਜੇ (ret != 0) {
			/* Clean up any partial mapping left behind. */
			ret = arch_mem_unmap(dst, ਮਾਪ);
			__ASSERT(ret == 0, "%s: cannot undo mapped pages %d", __func__, ret);

			dst = ਨਿਰਾ;

			ਜੇ (ret != 0) {
				/* Cannot fully undo all the mappings. There might still be pages
				 * mapped. So we cannot free the allocated virtual memory region.
				 */
				ਜਾਓ ਬਾਹਰਲਾ;
			} ਨਹੀਂਤਾਂ {
				ਜਾਓ fail;
			}
		}
	}

ਬਾਹਰਲਾ:
	k_spin_unlock(&z_mm_ਤਾਲਾ, ਕੁੰਜੀ);

	ਜੇ (dst != ਨਿਰਾ && !uninit) {
		/* If we later implement mappings to a copy-on-write
		 * zero page, won't need this step
		 */
		memset(dst, 0, ਮਾਪ);
	}

	ਵਾਪਸ dst;

fail:
	/* Release the whole region, including the guard pages. */
	virt_region_free(region, ਕੁੱਲ_ਮਾਪ);

	ਜਾਓ ਬਾਹਰਲਾ;
}

ਖਾਲੀ k_mem_unmap_phys_guard(ਖਾਲੀ *addr, size_t ਮਾਪ, bool is_anon)
{
	uint8_t *pos;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;
	size_t ਕੁੱਲ_ਮਾਪ;
	ਪੂਰਨ ret;

	/* Need space for the "before" guard page */
	__ASSERT_NO_MSG(ਸੂਚਕ_ਮੰਜ਼ਿਲ_UINT(addr) >= CONFIG_MMU_ਪੰਨਾ_ਮਾਪ);

	/* Make sure address range is still valid after accounting
	 * for two guard pages.
	 */
	pos = (uint8_t *)addr - CONFIG_MMU_ਪੰਨਾ_ਮਾਪ;
	k_mem_assert_virtual_region(pos, ਮਾਪ + (CONFIG_MMU_ਪੰਨਾ_ਮਾਪ * 2));

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&z_mm_ਤਾਲਾ);

	/* Check if both guard pages are unmapped.
	 * Bail if not, as this is probably a region not mapped
	 * using k_mem_map().
	 */
	pos = addr;
	ret = arch_ਪੰਨਾ_phys_get(pos - CONFIG_MMU_ਪੰਨਾ_ਮਾਪ, ਨਿਰਾ);
	__ASSERT(ret != 0,
		 "%s: cannot find preceding guard page for (%p, %zu)",
		 __func__, addr, ਮਾਪ);
	ਜੇ (ret == 0) {
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ret = arch_ਪੰਨਾ_phys_get(pos + ਮਾਪ, ਨਿਰਾ);
	__ASSERT(ret != 0,
		 "%s: cannot find succeeding guard page for (%p, %zu)",
		 __func__, addr, ਮਾਪ);
	ਜੇ (ret == 0) {
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ਜੇ (is_anon) {
		/* Unmapping anonymous memory */
		ret = unmap_anon_ਪੰਨੇ(addr, ਮਾਪ);
		__ASSERT(ret == 0, "%s: unmap_anon_pages() returned %d", __func__, ret);
		ਜੇ (ret != 0) {
			/* Fail to unmap. Must bail without releasing any resources
			 * as some pages are still be mapped.
			 */
			ਜਾਓ ਬਾਹਰਲਾ;
		}
	} ਨਹੀਂਤਾਂ {
		/*
		 * Unmapping previous mapped memory with specific physical address.
		 *
		 * Note that we don't have to unmap the guard pages, as they should
		 * have been unmapped. We just need to unmapped the in-between
		 * region [addr, (addr + size)).
		 */
		ret = arch_mem_unmap(addr, ਮਾਪ);
		__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
		ਜੇ (ret != 0) {
			/* Fail to unmap. Must bail without releasing any resources
			 * as some pages are still be mapped.
			 */
			ਜਾਓ ਬਾਹਰਲਾ;
		}
	}

	/* There are guard pages just before and after the mapped
	 * region. So we also need to free them from the bitmap.
	 */
	pos = (uint8_t *)addr - CONFIG_MMU_ਪੰਨਾ_ਮਾਪ;
	ਕੁੱਲ_ਮਾਪ = ਮਾਪ + (CONFIG_MMU_ਪੰਨਾ_ਮਾਪ * 2);
	virt_region_free(pos, ਕੁੱਲ_ਮਾਪ);

ਬਾਹਰਲਾ:
	k_spin_unlock(&z_mm_ਤਾਲਾ, ਕੁੰਜੀ);
}

ਪੂਰਨ k_mem_update_flags(ਖਾਲੀ *addr, size_t ਮਾਪ, uint32_t flags)
{
	uintptr_t phys;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;
	ਪੂਰਨ ret;

	k_mem_assert_virtual_region(addr, ਮਾਪ);

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&z_mm_ਤਾਲਾ);

	/*
	 * We can achieve desired result without explicit architecture support
	 * by unmapping and remapping the same physical memory using new flags.
	 */

	ret = arch_ਪੰਨਾ_phys_get(addr, &phys);
	ਜੇ (ret < 0) {
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	/* TODO: detect and handle paged-out memory as well */

	ret = arch_mem_unmap(addr, ਮਾਪ);
	__ASSERT(ret == 0, "%s: arch_mem_unmap failed %d\n", __func__, ret);
	ਜੇ (ret != 0) {
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ret = arch_mem_map(addr, phys, ਮਾਪ, flags);
	__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);

ਬਾਹਰਲਾ:
	k_spin_unlock(&z_mm_ਤਾਲਾ, ਕੁੰਜੀ);
	ਵਾਪਸ ret;
}

size_t k_mem_free_get(ਖਾਲੀ)
{
	size_t ret;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;

	__ASSERT(ਪੰਨਾ_ਚੌਖਟੇ_initialized, "%s called too early", __func__);

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&z_mm_ਤਾਲਾ);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING
	ਜੇ (z_free_ਪੰਨਾ_ਗਿਣਤੀ > CONFIG_DEMAND_PAGING_ਪੰਨਾ_ਚੌਖਟੇ_RESERVE) {
		ret = z_free_ਪੰਨਾ_ਗਿਣਤੀ - CONFIG_DEMAND_PAGING_ਪੰਨਾ_ਚੌਖਟੇ_RESERVE;
	} ਨਹੀਂਤਾਂ {
		ret = 0;
	}
#ਨਹੀਂਤਾਂ
	ret = z_free_ਪੰਨਾ_ਗਿਣਤੀ;
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING */
	k_spin_unlock(&z_mm_ਤਾਲਾ, ਕੁੰਜੀ);

	ਵਾਪਸ ret * (size_t)CONFIG_MMU_ਪੰਨਾ_ਮਾਪ;
}

/* Get the default virtual region alignment, here the default MMU page size
 *
 * @param[in] phys Physical address of region to be mapped, aligned to MMU_PAGE_SIZE
 * @param[in] size Size of region to be mapped, aligned to MMU_PAGE_SIZE
 *
 * @return alignment to apply on the virtual address of this region
 */
ਸਥਿਰ size_t virt_region_align(uintptr_t phys, size_t ਮਾਪ)
{
	ARG_ਅਣਵਰਤਿਆ(phys);
	ARG_ਅਣਵਰਤਿਆ(ਮਾਪ);

	ਵਾਪਸ CONFIG_MMU_ਪੰਨਾ_ਮਾਪ;
}

__weak FUNC_ALIAS(virt_region_align, arch_virt_region_align, size_t);

/* This may be called from arch early boot code before z_cstart() is invoked.
 * Data will be copied and BSS zeroed, but this must not rely on any
 * initialization functions being called prior to work correctly.
 */
ਖਾਲੀ k_mem_map_phys_bare(uint8_t **virt_ptr, uintptr_t phys, size_t ਮਾਪ, uint32_t flags)
{
	uintptr_t aligned_phys, addr_ਵਿੱਥ;
	size_t aligned_ਮਾਪ, align_boundary;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;
	uint8_t *dest_addr;
	size_t ਅੰਕ_ਬਿੱਟ;
	size_t ਵਿੱਥ;
	ਪੂਰਨ ret;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_KERNEL_ਸਿੱਧਾ_MAP
	__ASSERT(!(flags & K_MEM_ਸਿੱਧਾ_MAP), "The direct-map is not enabled");
#ਜੇ_ਅੰਤ /* CONFIG_KERNEL_DIRECT_MAP */
	addr_ਵਿੱਥ = k_mem_region_align(&aligned_phys, &aligned_ਮਾਪ,
					 phys, ਮਾਪ,
					 CONFIG_MMU_ਪੰਨਾ_ਮਾਪ);
	__ASSERT(aligned_ਮਾਪ != 0U, "0-length mapping at 0x%lx", aligned_phys);
	__ASSERT(aligned_ਮਾਪ - 1 <= (UINTPTR_MAX - aligned_phys),
		 "wraparound for physical address 0x%lx (size %zu)",
		 aligned_phys, aligned_ਮਾਪ);

	align_boundary = arch_virt_region_align(aligned_phys, aligned_ਮਾਪ);

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&z_mm_ਤਾਲਾ);

	ਜੇ (IS_ENABLED(CONFIG_KERNEL_ਸਿੱਧਾ_MAP) &&
	    (flags & K_MEM_ਸਿੱਧਾ_MAP)) {
		dest_addr = (uint8_t *)aligned_phys;

		/* Mark the region of virtual memory bitmap as used
		 * if the region overlaps the virtual memory space.
		 *
		 * Basically if either end of region is within
		 * virtual memory space, we need to mark the bits.
		 */

		ਜੇ (ਵਿੱਚ_RANGE(aligned_phys,
			      (uintptr_t)K_MEM_VIRT_RAM_ਸ਼ੁਰੂ,
			      (uintptr_t)(K_MEM_VIRT_RAM_ਸਿਰਾ - 1)) ||
		    ਵਿੱਚ_RANGE(aligned_phys + aligned_ਮਾਪ - 1,
			      (uintptr_t)K_MEM_VIRT_RAM_ਸ਼ੁਰੂ,
			      (uintptr_t)(K_MEM_VIRT_RAM_ਸਿਰਾ - 1))) {
			uint8_t *adjusted_ਸ਼ੁਰੂ = max(dest_addr, K_MEM_VIRT_RAM_ਸ਼ੁਰੂ);
			uint8_t *adjusted_ਸਿਰਾ = min(dest_addr + aligned_ਮਾਪ,
						    K_MEM_VIRT_RAM_ਸਿਰਾ);
			size_t adjusted_sz = adjusted_ਸਿਰਾ - adjusted_ਸ਼ੁਰੂ;

			ਅੰਕ_ਬਿੱਟ = adjusted_sz / CONFIG_MMU_ਪੰਨਾ_ਮਾਪ;
			ਵਿੱਥ = virt_ਮੰਜ਼ਿਲ_bitmap_ਵਿੱਥ(adjusted_ਸ਼ੁਰੂ, adjusted_sz);
			ਜੇ (sys_bitarray_test_and_ਸੈੱਟ_region(
			    &virt_region_bitmap, ਅੰਕ_ਬਿੱਟ, ਵਿੱਥ, true)) {
				ਜਾਓ fail;
			}
		}
	} ਨਹੀਂਤਾਂ {
		/* Obtain an appropriately sized chunk of virtual memory */
		dest_addr = virt_region_alloc(aligned_ਮਾਪ, align_boundary);
		ਜੇ (!dest_addr) {
			ਜਾਓ fail;
		}
	}

	/* If this fails there's something amiss with virt_region_get */
	__ASSERT((uintptr_t)dest_addr <
		 ((uintptr_t)dest_addr + (ਮਾਪ - 1)),
		 "wraparound for virtual address %p (size %zu)",
		 dest_addr, ਮਾਪ);

	LOG_DBG("arch_mem_map(%p, 0x%lx, %zu, %x) offset %lu", (ਖਾਲੀ *)dest_addr,
		aligned_phys, aligned_ਮਾਪ, flags, addr_ਵਿੱਥ);

	ret = arch_mem_map(dest_addr, aligned_phys, aligned_ਮਾਪ, flags);
	__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
	ਜੇ (ret != 0) {
		ਜਾਓ fail;
	}
	k_spin_unlock(&z_mm_ਤਾਲਾ, ਕੁੰਜੀ);

	*virt_ptr = dest_addr + addr_ਵਿੱਥ;
	ਵਾਪਸ;
fail:
	/* May re-visit this in the future, but for now running out of
	 * virtual address space or failing the arch_mem_map() call is
	 * an unrecoverable situation.
	 *
	 * Other problems not related to resource exhaustion we leave as
	 * assertions since they are clearly programming mistakes.
	 */
	LOG_ERR("memory mapping 0x%lx (size %zu, flags 0x%x) failed",
		phys, ਮਾਪ, flags);
	k_panic();
}

ਖਾਲੀ k_mem_unmap_phys_bare(uint8_t *virt, size_t ਮਾਪ)
{
	uintptr_t aligned_virt, addr_ਵਿੱਥ;
	size_t aligned_ਮਾਪ;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;
	ਪੂਰਨ ret;

	addr_ਵਿੱਥ = k_mem_region_align(&aligned_virt, &aligned_ਮਾਪ,
					 ਸੂਚਕ_ਮੰਜ਼ਿਲ_UINT(virt), ਮਾਪ,
					 CONFIG_MMU_ਪੰਨਾ_ਮਾਪ);
	__ASSERT(aligned_ਮਾਪ != 0U, "0-length mapping at 0x%lx", aligned_virt);
	__ASSERT(aligned_ਮਾਪ - 1 <= (UINTPTR_MAX - aligned_virt),
		 "wraparound for virtual address 0x%lx (size %zu)",
		 aligned_virt, aligned_ਮਾਪ);

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&z_mm_ਤਾਲਾ);

	LOG_DBG("arch_mem_unmap(0x%lx, %zu) offset %lu",
		aligned_virt, aligned_ਮਾਪ, addr_ਵਿੱਥ);

	ret = arch_mem_unmap(UINT_ਮੰਜ਼ਿਲ_ਸੂਚਕ(aligned_virt), aligned_ਮਾਪ);
	__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
	ਜੇ (ret != 0) {
		/* Fail to unmap. We cannot free the virtual memory region
		 * as some pages are still be mapped.
		 */
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	virt_region_free(UINT_ਮੰਜ਼ਿਲ_ਸੂਚਕ(aligned_virt), aligned_ਮਾਪ);

ਬਾਹਰਲਾ:
	k_spin_unlock(&z_mm_ਤਾਲਾ, ਕੁੰਜੀ);
}

/*
 * Miscellaneous
 */

size_t k_mem_region_align(uintptr_t *aligned_addr, size_t *aligned_ਮਾਪ,
			  uintptr_t addr, size_t ਮਾਪ, size_t align)
{
	size_t addr_ਵਿੱਥ;

	/* The actual mapped region must be page-aligned. Round down the
	 * physical address and pad the region size appropriately
	 */
	*aligned_addr = ਗੇੜਾ_ਥੱਲੇ(addr, align);
	addr_ਵਿੱਥ = addr - *aligned_addr;
	*aligned_ਮਾਪ = ਗੇੜਾ_ਉੱਪਰ(ਮਾਪ + addr_ਵਿੱਥ, align);

	ਵਾਪਸ addr_ਵਿੱਥ;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_LINKER_USE_BOOT_SECTION
ਸਥਿਰ ਖਾਲੀ mark_linker_section_pinned(ਖਾਲੀ *ਸ਼ੁਰੂ_addr, ਖਾਲੀ *ਸਿਰਾ_addr,
				       bool pin)
{
	ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *pf;
	uint8_t *addr;

	uintptr_t pinned_ਸ਼ੁਰੂ = ਗੇੜਾ_ਥੱਲੇ(ਸੂਚਕ_ਮੰਜ਼ਿਲ_UINT(ਸ਼ੁਰੂ_addr),
					    CONFIG_MMU_ਪੰਨਾ_ਮਾਪ);
	uintptr_t pinned_ਸਿਰਾ = ਗੇੜਾ_ਉੱਪਰ(ਸੂਚਕ_ਮੰਜ਼ਿਲ_UINT(ਸਿਰਾ_addr),
					CONFIG_MMU_ਪੰਨਾ_ਮਾਪ);
	size_t pinned_ਮਾਪ = pinned_ਸਿਰਾ - pinned_ਸ਼ੁਰੂ;

	VIRT_FOREACH(UINT_ਮੰਜ਼ਿਲ_ਸੂਚਕ(pinned_ਸ਼ੁਰੂ), pinned_ਮਾਪ, addr)
	{
		pf = k_mem_phys_ਮੰਜ਼ਿਲ_ਪੰਨਾ_ਚੌਖਟਾ(K_MEM_BOOT_VIRT_ਮੰਜ਼ਿਲ_PHYS(addr));
		ਚੌਖਟਾ_mapped_ਸੈੱਟ(pf, addr);

		ਜੇ (pin) {
			k_mem_ਪੰਨਾ_ਚੌਖਟਾ_ਸੈੱਟ(pf, K_MEM_ਪੰਨਾ_ਚੌਖਟਾ_PINNED);
		} ਨਹੀਂਤਾਂ {
			k_mem_ਪੰਨਾ_ਚੌਖਟਾ_clear(pf, K_MEM_ਪੰਨਾ_ਚੌਖਟਾ_PINNED);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING
			ਜੇ (IS_ENABLED(CONFIG_EVICTION_TRACKING) &&
			    k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_evictable(pf)) {
				k_mem_paging_eviction_ਜੋੜੋ(pf);
			}
#ਜੇ_ਅੰਤ
		}
	}
}
#ਜੇ_ਅੰਤ /* CONFIG_LINKER_USE_BOOT_SECTION */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_LINKER_USE_ONDEMAND_SECTION
ਸਥਿਰ ਖਾਲੀ z_paging_ondemand_section_map(ਖਾਲੀ)
{
	uint8_t *addr;
	size_t ਮਾਪ;
	uintptr_t location;
	uint32_t flags;
	ਪੂਰਨ ret;

	ਮਾਪ = (uintptr_t)lnkr_ondemand_ਪਾਠ_ਮਾਪ;
	flags = K_MEM_MAP_UNPAGED | K_MEM_PERM_EXEC | K_MEM_CACHE_WB;
	VIRT_FOREACH(lnkr_ondemand_ਪਾਠ_ਸ਼ੁਰੂ, ਮਾਪ, addr) {
		k_mem_paging_backing_store_location_query(addr, &location);

		ret = arch_mem_map(addr, location, CONFIG_MMU_ਪੰਨਾ_ਮਾਪ, flags);
		__ASSERT(ret == 0, "arch_mem_map() returned %d", ret);
		ਜੇ (ret != 0) {
			k_panic();
		}

		sys_bitarray_ਸੈੱਟ_region(&virt_region_bitmap, 1,
					virt_ਮੰਜ਼ਿਲ_bitmap_ਵਿੱਥ(addr, CONFIG_MMU_ਪੰਨਾ_ਮਾਪ));
	}

	ਮਾਪ = (uintptr_t)lnkr_ondemand_rodata_ਮਾਪ;
	flags = K_MEM_MAP_UNPAGED | K_MEM_CACHE_WB;
	VIRT_FOREACH(lnkr_ondemand_rodata_ਸ਼ੁਰੂ, ਮਾਪ, addr) {
		k_mem_paging_backing_store_location_query(addr, &location);

		ret = arch_mem_map(addr, location, CONFIG_MMU_ਪੰਨਾ_ਮਾਪ, flags);
		__ASSERT(ret == 0, "arch_mem_map() returned %d", ret);
		ਜੇ (ret != 0) {
			k_panic();
		}

		sys_bitarray_ਸੈੱਟ_region(&virt_region_bitmap, 1,
					virt_ਮੰਜ਼ਿਲ_bitmap_ਵਿੱਥ(addr, CONFIG_MMU_ਪੰਨਾ_ਮਾਪ));
	}
}
#ਜੇ_ਅੰਤ /* CONFIG_LINKER_USE_ONDEMAND_SECTION */

ਖਾਲੀ z_mem_manage_init(ਖਾਲੀ)
{
	uintptr_t phys;
	uint8_t *addr;
	ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *pf;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&z_mm_ਤਾਲਾ);

	free_ਪੰਨਾ_ਚੌਖਟਾ_list_init();

	ARG_ਅਣਵਰਤਿਆ(addr);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ARCH_HAS_RESERVED_ਪੰਨਾ_ਚੌਖਟੇ
	/* If some page frames are unavailable for use as memory, arch
	 * code will mark K_MEM_PAGE_FRAME_RESERVED in their flags
	 */
	arch_reserved_ਪੰਨੇ_update();
#ਜੇ_ਅੰਤ /* CONFIG_ARCH_HAS_RESERVED_PAGE_FRAMES */

	/* The entire Zephyr image is mapped and pinned at boot. Demand
	 * paging applies only to anonymous mappings created via k_mem_map()
	 * and to memory placed in explicit __ondemand_* linker sections;
	 * the kernel image itself is never a candidate for eviction.
	 */
	VIRT_FOREACH(K_MEM_KERNEL_VIRT_ਸ਼ੁਰੂ, K_MEM_KERNEL_VIRT_ਮਾਪ, addr)
	{
		pf = k_mem_phys_ਮੰਜ਼ਿਲ_ਪੰਨਾ_ਚੌਖਟਾ(K_MEM_BOOT_VIRT_ਮੰਜ਼ਿਲ_PHYS(addr));
		ਚੌਖਟਾ_mapped_ਸੈੱਟ(pf, addr);
		k_mem_ਪੰਨਾ_ਚੌਖਟਾ_ਸੈੱਟ(pf, K_MEM_ਪੰਨਾ_ਚੌਖਟਾ_PINNED);
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_LINKER_USE_BOOT_SECTION
	/* Pin the boot section to prevent it from being swapped out during
	 * boot process. Will be un-pinned once boot process completes.
	 */
	mark_linker_section_pinned(lnkr_boot_ਸ਼ੁਰੂ, lnkr_boot_ਸਿਰਾ, true);
#ਜੇ_ਅੰਤ /* CONFIG_LINKER_USE_BOOT_SECTION */

	/* Any remaining pages that aren't mapped, reserved, or pinned get
	 * added to the free pages list
	 */
	K_MEM_ਪੰਨਾ_ਚੌਖਟਾ_FOREACH(phys, pf) {
		ਜੇ (k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_ਉਪਲਬਧ(pf)) {
			free_ਪੰਨਾ_ਚੌਖਟਾ_list_put(pf);
		}
	}
	LOG_DBG("free page frames: %zu", z_free_ਪੰਨਾ_ਗਿਣਤੀ);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	z_paging_histogram_init();
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
	k_mem_paging_backing_store_init();
	k_mem_paging_eviction_init();

	ਜੇ (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
		/* start tracking evictable page installed above if any */
		K_MEM_ਪੰਨਾ_ਚੌਖਟਾ_FOREACH(phys, pf) {
			ਜੇ (k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_evictable(pf)) {
				k_mem_paging_eviction_ਜੋੜੋ(pf);
			}
		}
	}
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_LINKER_USE_ONDEMAND_SECTION
	z_paging_ondemand_section_map();
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ASSERT
	ਪੰਨਾ_ਚੌਖਟੇ_initialized = true;
#ਜੇ_ਅੰਤ /* CONFIG_ASSERT */

	k_spin_unlock(&z_mm_ਤਾਲਾ, ਕੁੰਜੀ);
}

ਖਾਲੀ z_mem_manage_boot_ਅੰਤ(ਖਾਲੀ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_LINKER_USE_BOOT_SECTION
	/* At the end of boot process, unpin the boot sections
	 * as they don't need to be in memory all the time anymore.
	 */
	mark_linker_section_pinned(lnkr_boot_ਸ਼ੁਰੂ, lnkr_boot_ਸਿਰਾ, false);
#ਜੇ_ਅੰਤ /* CONFIG_LINKER_USE_BOOT_SECTION */
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_STATS
ਬਣਤਰ k_mem_paging_stats_t paging_stats;
ਬਾਹਰੀ ਬਣਤਰ k_mem_paging_histogram_t z_paging_histogram_eviction;
ਬਾਹਰੀ ਬਣਤਰ k_mem_paging_histogram_t z_paging_histogram_backing_store_ਪੰਨਾ_ਵਿੱਚ;
ਬਾਹਰੀ ਬਣਤਰ k_mem_paging_histogram_t z_paging_histogram_backing_store_ਪੰਨਾ_ਬਾਹਰਲਾ;
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_STATS */

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ do_backing_store_ਪੰਨਾ_ਵਿੱਚ(uintptr_t location)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_MAPPING
	/* Check for special cases */
	ਚੋਣ (location) {
	ਹਾਲਤ ARCH_UNPAGED_ANON_ZERO:
		memset(K_MEM_SCRATCH_ਪੰਨਾ, 0, CONFIG_MMU_ਪੰਨਾ_ਮਾਪ);
		__fallthrough;
	ਹਾਲਤ ARCH_UNPAGED_ANON_UNINIT:
		/* nothing else to do */
		ਵਾਪਸ;
	ਮੂਲ:
		ਰੋਕੋ;
	}
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_MAPPING */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	uint32_t ਸਮਾਂ_diff;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	timing_t ਸਮਾਂ_ਸ਼ੁਰੂ, ਸਮਾਂ_ਸਿਰਾ;

	ਸਮਾਂ_ਸ਼ੁਰੂ = timing_counter_get();
#ਨਹੀਂਤਾਂ
	uint32_t ਸਮਾਂ_ਸ਼ੁਰੂ;

	ਸਮਾਂ_ਸ਼ੁਰੂ = k_ਚੱਕਰ_get_32();
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	k_mem_paging_backing_store_ਪੰਨਾ_ਵਿੱਚ(location);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	ਸਮਾਂ_ਸਿਰਾ = timing_counter_get();
	ਸਮਾਂ_diff = (uint32_t)timing_ਚੱਕਰਾਂ_get(&ਸਮਾਂ_ਸ਼ੁਰੂ, &ਸਮਾਂ_ਸਿਰਾ);
#ਨਹੀਂਤਾਂ
	ਸਮਾਂ_diff = k_ਚੱਕਰ_get_32() - ਸਮਾਂ_ਸ਼ੁਰੂ;
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */

	z_paging_histogram_inc(&z_paging_histogram_backing_store_ਪੰਨਾ_ਵਿੱਚ,
			       ਸਮਾਂ_diff);
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ do_backing_store_ਪੰਨਾ_ਬਾਹਰਲਾ(uintptr_t location)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	uint32_t ਸਮਾਂ_diff;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	timing_t ਸਮਾਂ_ਸ਼ੁਰੂ, ਸਮਾਂ_ਸਿਰਾ;

	ਸਮਾਂ_ਸ਼ੁਰੂ = timing_counter_get();
#ਨਹੀਂਤਾਂ
	uint32_t ਸਮਾਂ_ਸ਼ੁਰੂ;

	ਸਮਾਂ_ਸ਼ੁਰੂ = k_ਚੱਕਰ_get_32();
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	k_mem_paging_backing_store_ਪੰਨਾ_ਬਾਹਰਲਾ(location);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	ਸਮਾਂ_ਸਿਰਾ = timing_counter_get();
	ਸਮਾਂ_diff = (uint32_t)timing_ਚੱਕਰਾਂ_get(&ਸਮਾਂ_ਸ਼ੁਰੂ, &ਸਮਾਂ_ਸਿਰਾ);
#ਨਹੀਂਤਾਂ
	ਸਮਾਂ_diff = k_ਚੱਕਰ_get_32() - ਸਮਾਂ_ਸ਼ੁਰੂ;
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */

	z_paging_histogram_inc(&z_paging_histogram_backing_store_ਪੰਨਾ_ਬਾਹਰਲਾ,
			       ਸਮਾਂ_diff);
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
}

#ਜੇ defined(CONFIG_SMP) && defined(CONFIG_DEMAND_PAGING_ALLOW_IRQ)
/*
 * SMP support is very simple. Some resources such as the scratch page could
 * be made per CPU, backing store driver execution be confined to the faulting
 * CPU, statistics be made to cope with access concurrency, etc. But in the
 * end we're dealing with memory transfer to/from some external storage which
 * is inherently slow and whose access is most likely serialized anyway.
 * So let's simply enforce global demand paging serialization across all CPUs
 * with a mutex as there is no real gain from added parallelism here.
 */
ਸਥਿਰ K_MUTEX_DEFINE(z_mm_paging_ਤਾਲਾ);
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਖਾਲੀ virt_region_foreach(ਖਾਲੀ *addr, size_t ਮਾਪ,
				ਖਾਲੀ (*func)(ਖਾਲੀ *vaddr))
{
	k_mem_assert_virtual_region(addr, ਮਾਪ);

	ਲਈ (size_t ਵਿੱਥ = 0; ਵਿੱਥ < ਮਾਪ; ਵਿੱਥ += CONFIG_MMU_ਪੰਨਾ_ਮਾਪ) {
		func((uint8_t *)addr + ਵਿੱਥ);
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
ਸਥਿਰ ਪੂਰਨ ਪੰਨਾ_ਚੌਖਟਾ_prepare_locked(ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *pf, bool *dirty_ptr,
				     bool ਪੰਨਾ_fault, uintptr_t *location_ptr)
{
	uintptr_t phys;
	ਪੂਰਨ ret;
	bool dirty = *dirty_ptr;

	phys = k_mem_ਪੰਨਾ_ਚੌਖਟਾ_ਮੰਜ਼ਿਲ_phys(pf);
	__ASSERT(!k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_pinned(pf), "page frame 0x%lx is pinned",
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
	ਜੇ (k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_mapped(pf)) {
		dirty = dirty || !k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_backed(pf);
	}

	ਜੇ (dirty || ਪੰਨਾ_fault) {
		arch_mem_scratch(phys);
	}

	ਜੇ (k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_mapped(pf)) {
		ret = k_mem_paging_backing_store_location_get(pf, location_ptr,
							      ਪੰਨਾ_fault);
		ਜੇ (ret != 0) {
			LOG_ERR("out of backing store memory");
			ਵਾਪਸ -ENOMEM;
		}
		arch_mem_ਪੰਨਾ_ਬਾਹਰਲਾ(k_mem_ਪੰਨਾ_ਚੌਖਟਾ_ਮੰਜ਼ਿਲ_virt(pf), *location_ptr);

		ਜੇ (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
			k_mem_paging_eviction_remove(pf);
		}
	} ਨਹੀਂਤਾਂ {
		/* Shouldn't happen unless this function is mis-used */
		__ASSERT(!dirty, "un-mapped page determined to be dirty");
	}
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	/* Mark as busy so that k_mem_page_frame_is_evictable() returns false */
	__ASSERT(!k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_busy(pf), "page frame 0x%lx is already busy",
		 phys);
	k_mem_ਪੰਨਾ_ਚੌਖਟਾ_ਸੈੱਟ(pf, K_MEM_ਪੰਨਾ_ਚੌਖਟਾ_BUSY);
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	/* Update dirty parameter, since we set to true if it wasn't backed
	 * even if otherwise clean
	 */
	*dirty_ptr = dirty;

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ do_mem_evict(ਖਾਲੀ *addr)
{
	bool dirty;
	ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *pf;
	uintptr_t location;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;
	uintptr_t flags, phys;
	ਪੂਰਨ ret;

#ਜੇ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	__ASSERT(!k_is_ਵਿੱਚ_isr(),
		 "%s is unavailable in ISRs with CONFIG_DEMAND_PAGING_ALLOW_IRQ",
		 __func__);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
	k_mutex_ਤਾਲਾ(&z_mm_paging_ਤਾਲਾ, K_FOREVER);
#ਨਹੀਂਤਾਂ
	k_sched_ਤਾਲਾ();
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&z_mm_ਤਾਲਾ);
	flags = arch_ਪੰਨਾ_info_get(addr, &phys, false);
	__ASSERT((flags & ARCH_ਅੰਕੜੇ_ਪੰਨਾ_NOT_MAPPED) == 0,
		 "address %p isn't mapped", addr);
	ਜੇ ((flags & ARCH_ਅੰਕੜੇ_ਪੰਨਾ_LOADED) == 0) {
		/* Un-mapped or already evicted. Nothing to do */
		ret = 0;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	dirty = (flags & ARCH_ਅੰਕੜੇ_ਪੰਨਾ_DIRTY) != 0;
	pf = k_mem_phys_ਮੰਜ਼ਿਲ_ਪੰਨਾ_ਚੌਖਟਾ(phys);
	__ASSERT(k_mem_ਪੰਨਾ_ਚੌਖਟਾ_ਮੰਜ਼ਿਲ_virt(pf) == addr, "page frame address mismatch");
	ret = ਪੰਨਾ_ਚੌਖਟਾ_prepare_locked(pf, &dirty, false, &location);
	ਜੇ (ret != 0) {
		ਜਾਓ ਬਾਹਰਲਾ;
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	k_spin_unlock(&z_mm_ਤਾਲਾ, ਕੁੰਜੀ);
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	ਜੇ (dirty) {
		do_backing_store_ਪੰਨਾ_ਬਾਹਰਲਾ(location);
	}
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&z_mm_ਤਾਲਾ);
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	ਪੰਨਾ_ਚੌਖਟਾ_free_locked(pf);
ਬਾਹਰਲਾ:
	k_spin_unlock(&z_mm_ਤਾਲਾ, ਕੁੰਜੀ);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_ALLOW_IRQ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
	k_mutex_unlock(&z_mm_paging_ਤਾਲਾ);
#ਨਹੀਂਤਾਂ
	k_sched_unlock();
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	ਵਾਪਸ ret;
}

ਪੂਰਨ k_mem_ਪੰਨਾ_ਬਾਹਰਲਾ(ਖਾਲੀ *addr, size_t ਮਾਪ)
{
	__ASSERT(ਪੰਨਾ_ਚੌਖਟੇ_initialized, "%s called on %p too early", __func__,
		 addr);
	k_mem_assert_virtual_region(addr, ਮਾਪ);

	ਲਈ (size_t ਵਿੱਥ = 0; ਵਿੱਥ < ਮਾਪ; ਵਿੱਥ += CONFIG_MMU_ਪੰਨਾ_ਮਾਪ) {
		ਖਾਲੀ *pos = (uint8_t *)addr + ਵਿੱਥ;
		ਪੂਰਨ ret;

		ret = do_mem_evict(pos);
		ਜੇ (ret != 0) {
			ਵਾਪਸ ret;
		}
	}

	ਵਾਪਸ 0;
}

ਪੂਰਨ k_mem_ਪੰਨਾ_ਚੌਖਟਾ_evict(uintptr_t phys)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;
	ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *pf;
	bool dirty;
	uintptr_t flags;
	uintptr_t location;
	ਪੂਰਨ ret;

	__ASSERT(ਪੰਨਾ_ਚੌਖਟੇ_initialized, "%s called on 0x%lx too early",
		 __func__, phys);

	/* Implementation is similar to do_page_fault() except there is no
	 * data page to page-in, see comments in that function.
	 */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	__ASSERT(!k_is_ਵਿੱਚ_isr(),
		 "%s is unavailable in ISRs with CONFIG_DEMAND_PAGING_ALLOW_IRQ",
		 __func__);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
	k_mutex_ਤਾਲਾ(&z_mm_paging_ਤਾਲਾ, K_FOREVER);
#ਨਹੀਂਤਾਂ
	k_sched_ਤਾਲਾ();
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&z_mm_ਤਾਲਾ);
	pf = k_mem_phys_ਮੰਜ਼ਿਲ_ਪੰਨਾ_ਚੌਖਟਾ(phys);
	ਜੇ (!k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_mapped(pf)) {
		/* Nothing to do, free page */
		ret = 0;
		ਜਾਓ ਬਾਹਰਲਾ;
	}
	flags = arch_ਪੰਨਾ_info_get(k_mem_ਪੰਨਾ_ਚੌਖਟਾ_ਮੰਜ਼ਿਲ_virt(pf), ਨਿਰਾ, false);
	/* Shouldn't ever happen */
	__ASSERT((flags & ARCH_ਅੰਕੜੇ_ਪੰਨਾ_LOADED) != 0, "data page not loaded");
	dirty = (flags & ARCH_ਅੰਕੜੇ_ਪੰਨਾ_DIRTY) != 0;
	ret = ਪੰਨਾ_ਚੌਖਟਾ_prepare_locked(pf, &dirty, false, &location);
	ਜੇ (ret != 0) {
		ਜਾਓ ਬਾਹਰਲਾ;
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	k_spin_unlock(&z_mm_ਤਾਲਾ, ਕੁੰਜੀ);
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	ਜੇ (dirty) {
		do_backing_store_ਪੰਨਾ_ਬਾਹਰਲਾ(location);
	}
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&z_mm_ਤਾਲਾ);
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	ਪੰਨਾ_ਚੌਖਟਾ_free_locked(pf);
ਬਾਹਰਲਾ:
	k_spin_unlock(&z_mm_ਤਾਲਾ, ਕੁੰਜੀ);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_ALLOW_IRQ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
	k_mutex_unlock(&z_mm_paging_ਤਾਲਾ);
#ਨਹੀਂਤਾਂ
	k_sched_unlock();
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	ਵਾਪਸ ret;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ paging_stats_ਨੁਕਸ_inc(ਬਣਤਰ k_thread *faulting_thread,
					   ਪੂਰਨ ਕੁੰਜੀ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_STATS
	bool is_irq_unlocked = arch_irq_unlocked(ਕੁੰਜੀ);

	paging_stats.pagefaults.cnt++;

	ਜੇ (is_irq_unlocked) {
		paging_stats.pagefaults.irq_unlocked++;
	} ਨਹੀਂਤਾਂ {
		paging_stats.pagefaults.irq_locked++;
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_THREAD_STATS
	faulting_thread->paging_stats.pagefaults.cnt++;

	ਜੇ (is_irq_unlocked) {
		faulting_thread->paging_stats.pagefaults.irq_unlocked++;
	} ਨਹੀਂਤਾਂ {
		faulting_thread->paging_stats.pagefaults.irq_locked++;
	}
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(faulting_thread);
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_THREAD_STATS */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	ਜੇ (k_is_ਵਿੱਚ_isr()) {
		paging_stats.pagefaults.ਵਿੱਚ_isr++;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_THREAD_STATS
		faulting_thread->paging_stats.pagefaults.ਵਿੱਚ_isr++;
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_THREAD_STATS */
	}
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_STATS */
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ paging_stats_eviction_inc(ਬਣਤਰ k_thread *faulting_thread,
					     bool dirty)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_STATS
	ਜੇ (dirty) {
		paging_stats.eviction.dirty++;
	} ਨਹੀਂਤਾਂ {
		paging_stats.eviction.clean++;
	}
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_THREAD_STATS
	ਜੇ (dirty) {
		faulting_thread->paging_stats.eviction.dirty++;
	} ਨਹੀਂਤਾਂ {
		faulting_thread->paging_stats.eviction.clean++;
	}
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(faulting_thread);
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_THREAD_STATS */
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_STATS */
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *do_eviction_select(bool *dirty)
{
	ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *pf;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	uint32_t ਸਮਾਂ_diff;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	timing_t ਸਮਾਂ_ਸ਼ੁਰੂ, ਸਮਾਂ_ਸਿਰਾ;

	ਸਮਾਂ_ਸ਼ੁਰੂ = timing_counter_get();
#ਨਹੀਂਤਾਂ
	uint32_t ਸਮਾਂ_ਸ਼ੁਰੂ;

	ਸਮਾਂ_ਸ਼ੁਰੂ = k_ਚੱਕਰ_get_32();
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	pf = k_mem_paging_eviction_select(dirty);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	ਸਮਾਂ_ਸਿਰਾ = timing_counter_get();
	ਸਮਾਂ_diff = (uint32_t)timing_ਚੱਕਰਾਂ_get(&ਸਮਾਂ_ਸ਼ੁਰੂ, &ਸਮਾਂ_ਸਿਰਾ);
#ਨਹੀਂਤਾਂ
	ਸਮਾਂ_diff = k_ਚੱਕਰ_get_32() - ਸਮਾਂ_ਸ਼ੁਰੂ;
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */

	z_paging_histogram_inc(&z_paging_histogram_eviction, ਸਮਾਂ_diff);
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	ਵਾਪਸ pf;
}

ਸਥਿਰ bool do_ਪੰਨਾ_fault(ਖਾਲੀ *addr, bool pin)
{
	ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *pf;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;
	uintptr_t ਪੰਨਾ_ਵਿੱਚ_location, ਪੰਨਾ_ਬਾਹਰਲਾ_location;
	ਗਿਣਤੀ arch_ਪੰਨਾ_location ਸਥਿਤੀ;
	bool ਨਤੀਜਾ;
	bool dirty = false;
	ਬਣਤਰ k_thread *faulting_thread;
	ਪੂਰਨ ret;

	__ASSERT(ਪੰਨਾ_ਚੌਖਟੇ_initialized, "page fault at %p happened too early",
		 addr);

	LOG_DBG("page fault at %p", addr);

	/*
	 * TODO: Add performance accounting:
	 * - k_mem_paging_eviction_select() metrics
	 *   * periodic timer execution time histogram (if implemented)
	 */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_ALLOW_IRQ
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
	__ASSERT(!k_is_ਵਿੱਚ_isr(), "ISR page faults are forbidden");
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
	k_mutex_ਤਾਲਾ(&z_mm_paging_ਤਾਲਾ, K_FOREVER);
#ਨਹੀਂਤਾਂ
	k_sched_ਤਾਲਾ();
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&z_mm_ਤਾਲਾ);
	faulting_thread = _current;

	ਸਥਿਤੀ = arch_ਪੰਨਾ_location_get(addr, &ਪੰਨਾ_ਵਿੱਚ_location);
	ਜੇ (ਸਥਿਤੀ == ARCH_ਪੰਨਾ_LOCATION_BAD) {
		/* Return false to treat as a fatal error */
		ਨਤੀਜਾ = false;
		ਜਾਓ ਬਾਹਰਲਾ;
	}
	ਨਤੀਜਾ = true;

	ਜੇ (ਸਥਿਤੀ == ARCH_ਪੰਨਾ_LOCATION_PAGED_ਵਿੱਚ) {
		ਜੇ (pin) {
			/* It's a physical memory address */
			uintptr_t phys = ਪੰਨਾ_ਵਿੱਚ_location;

			pf = k_mem_phys_ਮੰਜ਼ਿਲ_ਪੰਨਾ_ਚੌਖਟਾ(phys);
			ਜੇ (!k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_pinned(pf)) {
				ਜੇ (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
					k_mem_paging_eviction_remove(pf);
				}
				k_mem_ਪੰਨਾ_ਚੌਖਟਾ_ਸੈੱਟ(pf, K_MEM_ਪੰਨਾ_ਚੌਖਟਾ_PINNED);
			}
		}

		/* This if-block is to pin the page if it is
		 * already present in physical memory. There is
		 * no need to go through the following code to
		 * pull in the data pages. So skip to the end.
		 */
		ਜਾਓ ਬਾਹਰਲਾ;
	}
	__ASSERT(ਸਥਿਤੀ == ARCH_ਪੰਨਾ_LOCATION_PAGED_ਬਾਹਰਲਾ,
		 "unexpected status value %d", ਸਥਿਤੀ);

	paging_stats_ਨੁਕਸ_inc(faulting_thread, ਕੁੰਜੀ.ਕੁੰਜੀ);

	pf = free_ਪੰਨਾ_ਚੌਖਟਾ_list_get();
	ਜੇ (pf == ਨਿਰਾ) {
		/* Need to evict a page frame */
		pf = do_eviction_select(&dirty);
		ਜੇ (pf == ਨਿਰਾ) {
			/* Every evictable page frame is pinned or busy.
			 * Fail the fault so it is reported as fatal.
			 */
			ਨਤੀਜਾ = false;
			ਜਾਓ ਬਾਹਰਲਾ;
		}
		LOG_DBG("evicting %p at 0x%lx",
			k_mem_ਪੰਨਾ_ਚੌਖਟਾ_ਮੰਜ਼ਿਲ_virt(pf),
			k_mem_ਪੰਨਾ_ਚੌਖਟਾ_ਮੰਜ਼ਿਲ_phys(pf));

		paging_stats_eviction_inc(faulting_thread, dirty);
	}
	ret = ਪੰਨਾ_ਚੌਖਟਾ_prepare_locked(pf, &dirty, true, &ਪੰਨਾ_ਬਾਹਰਲਾ_location);
	ਜੇ (ret != 0) {
		/* Backing store is full. Fail the fault so it is
		 * reported as fatal.
		 */
		ਨਤੀਜਾ = false;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	k_spin_unlock(&z_mm_ਤਾਲਾ, ਕੁੰਜੀ);
	/* Interrupts are now unlocked if they were not locked when we entered
	 * this function, and we may service ISRs. The scheduler is still
	 * locked.
	 */
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	ਜੇ (dirty) {
		do_backing_store_ਪੰਨਾ_ਬਾਹਰਲਾ(ਪੰਨਾ_ਬਾਹਰਲਾ_location);
	}
	do_backing_store_ਪੰਨਾ_ਵਿੱਚ(ਪੰਨਾ_ਵਿੱਚ_location);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&z_mm_ਤਾਲਾ);
	k_mem_ਪੰਨਾ_ਚੌਖਟਾ_clear(pf, K_MEM_ਪੰਨਾ_ਚੌਖਟਾ_BUSY);
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	k_mem_ਪੰਨਾ_ਚੌਖਟਾ_clear(pf, K_MEM_ਪੰਨਾ_ਚੌਖਟਾ_MAPPED);
	ਚੌਖਟਾ_mapped_ਸੈੱਟ(pf, addr);
	ਜੇ (pin) {
		k_mem_ਪੰਨਾ_ਚੌਖਟਾ_ਸੈੱਟ(pf, K_MEM_ਪੰਨਾ_ਚੌਖਟਾ_PINNED);
	}

	arch_mem_ਪੰਨਾ_ਵਿੱਚ(addr, k_mem_ਪੰਨਾ_ਚੌਖਟਾ_ਮੰਜ਼ਿਲ_phys(pf));
	k_mem_paging_backing_store_ਪੰਨਾ_finalize(pf, ਪੰਨਾ_ਵਿੱਚ_location);
	ਜੇ (IS_ENABLED(CONFIG_EVICTION_TRACKING) && (!pin)) {
		k_mem_paging_eviction_ਜੋੜੋ(pf);
	}
ਬਾਹਰਲਾ:
	k_spin_unlock(&z_mm_ਤਾਲਾ, ਕੁੰਜੀ);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_ALLOW_IRQ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
	k_mutex_unlock(&z_mm_paging_ਤਾਲਾ);
#ਨਹੀਂਤਾਂ
	k_sched_unlock();
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */

	ਵਾਪਸ ਨਤੀਜਾ;
}

ਸਥਿਰ ਖਾਲੀ do_ਪੰਨਾ_ਵਿੱਚ(ਖਾਲੀ *addr)
{
	bool ret;

	ret = do_ਪੰਨਾ_fault(addr, false);
	ਜੇ (!ret) {
		/* There is no error reporting channel to the caller, and
		 * continuing would defer the failure to an arbitrary later
		 * access.
		 */
		LOG_ERR("cannot page in address %p", addr);
		k_panic();
	}
}

ਖਾਲੀ k_mem_ਪੰਨਾ_ਵਿੱਚ(ਖਾਲੀ *addr, size_t ਮਾਪ)
{
	__ASSERT(!IS_ENABLED(CONFIG_DEMAND_PAGING_ALLOW_IRQ) || !k_is_ਵਿੱਚ_isr(),
		 "%s may not be called in ISRs if CONFIG_DEMAND_PAGING_ALLOW_IRQ is enabled",
		 __func__);
	virt_region_foreach(addr, ਮਾਪ, do_ਪੰਨਾ_ਵਿੱਚ);
}

ਸਥਿਰ ਖਾਲੀ do_mem_pin(ਖਾਲੀ *addr)
{
	bool ret;

	ret = do_ਪੰਨਾ_fault(addr, true);
	ਜੇ (!ret) {
		/* There is no error reporting channel to the caller, and
		 * continuing with the memory not actually pinned would
		 * defer the failure to an arbitrary later access, possibly
		 * from a context that cannot handle a page fault.
		 */
		LOG_ERR("cannot pin address %p", addr);
		k_panic();
	}
}

ਖਾਲੀ k_mem_pin(ਖਾਲੀ *addr, size_t ਮਾਪ)
{
	__ASSERT(!IS_ENABLED(CONFIG_DEMAND_PAGING_ALLOW_IRQ) || !k_is_ਵਿੱਚ_isr(),
		 "%s may not be called in ISRs if CONFIG_DEMAND_PAGING_ALLOW_IRQ is enabled",
		 __func__);
	virt_region_foreach(addr, ਮਾਪ, do_mem_pin);
}

bool k_mem_ਪੰਨਾ_fault(ਖਾਲੀ *addr)
{
	ਵਾਪਸ do_ਪੰਨਾ_fault(addr, false);
}

ਸਥਿਰ ਖਾਲੀ do_mem_unpin(ਖਾਲੀ *addr)
{
	ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *pf;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;
	uintptr_t flags, phys;

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&z_mm_ਤਾਲਾ);
	flags = arch_ਪੰਨਾ_info_get(addr, &phys, false);
	__ASSERT((flags & ARCH_ਅੰਕੜੇ_ਪੰਨਾ_NOT_MAPPED) == 0,
		 "invalid data page at %p", addr);
	ਜੇ ((flags & ARCH_ਅੰਕੜੇ_ਪੰਨਾ_LOADED) != 0) {
		pf = k_mem_phys_ਮੰਜ਼ਿਲ_ਪੰਨਾ_ਚੌਖਟਾ(phys);
		ਜੇ (k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_pinned(pf)) {
			k_mem_ਪੰਨਾ_ਚੌਖਟਾ_clear(pf, K_MEM_ਪੰਨਾ_ਚੌਖਟਾ_PINNED);

			ਜੇ (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
				k_mem_paging_eviction_ਜੋੜੋ(pf);
			}
		}
	}
	k_spin_unlock(&z_mm_ਤਾਲਾ, ਕੁੰਜੀ);
}

ਖਾਲੀ k_mem_unpin(ਖਾਲੀ *addr, size_t ਮਾਪ)
{
	__ASSERT(ਪੰਨਾ_ਚੌਖਟੇ_initialized, "%s called on %p too early", __func__,
		 addr);
	virt_region_foreach(addr, ਮਾਪ, do_mem_unpin);
}

#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING */
