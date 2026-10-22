/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Routines for managing virtual address spaces
 */

#ضمن <stdint.h>
#ضمن <kernel_arch_interface.h>
#ضمن <zephyr/spinlock.h>
#ضمن <mmu.h>
#ضمن <zephyr/init.h>
#ضمن <kernel_internal.h>
#ضمن <zephyr/internal/syscall_handler.h>
#ضمن <zephyr/toolchain.h>
#ضمن <zephyr/linker/linker-defs.h>
#ضمن <zephyr/sys/bitarray.h>
#ضمن <zephyr/sys/check.h>
#ضمن <zephyr/sys/math_extras.h>
#ضمن <zephyr/sys/minmax.h>
#ضمن <zephyr/timing/timing.h>
#ضمن <zephyr/arch/common/init.h>
#ضمن <zephyr/logging/log.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#جے_تعریف_شدہ CONFIG_DEMAND_PAGING
#ضمن <zephyr/kernel/mm/demand_paging.h>
#جے_انت /* CONFIG_DEMAND_PAGING */

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
بنية k_spinlock z_mm_جندرا;

/*
 * General page frame management
 */

/* Database of all RAM page frames */
بنية k_mem_ورقہ_چوکھٹا k_mem_ورقہ_چوکھٹے[K_MEM_عدد_ورقہ_چوکھٹے];

#جے_تعریف_شدہ CONFIG_ASSERT
/* Indicator that k_mem_page_frames has been initialized, many of these APIs do
 * not work before POST_KERNEL
 */
ساكن bool ورقہ_چوکھٹے_initialized;
#جے_انت /* CONFIG_ASSERT */

/* Add colors to page table dumps to indicate mapping type */
#تعریف_کرو COLOR_ورقہ_چوکھٹے	1

#إذا COLOR_ورقہ_چوکھٹے
#تعریف_کرو ANSI_DEFAULT "\x1B" "[0m"
#تعریف_کرو ANSI_RED     "\x1B" "[1;31m"
#تعریف_کرو ANSI_GREEN   "\x1B" "[1;32m"
#تعریف_کرو ANSI_YELLOW  "\x1B" "[1;33m"
#تعریف_کرو ANSI_BLUE    "\x1B" "[1;34m"
#تعریف_کرو ANSI_MAGENTA "\x1B" "[1;35m"
#تعریف_کرو ANSI_CYAN    "\x1B" "[1;36m"
#تعریف_کرو ANSI_GREY    "\x1B" "[1;90m"

#تعریف_کرو COLOR(x)	printk(_CONCAT(ANSI_, x))
#وإلا
#تعریف_کرو COLOR(x)	افعل { } طالما (false)
#جے_انت /* COLOR_PAGE_FRAMES */

/* LCOV_EXCL_START */
ساكن فراغ ورقہ_چوکھٹا_dump(بنية k_mem_ورقہ_چوکھٹا *pf)
{
	إذا (k_mem_ورقہ_چوکھٹا_is_free(pf)) {
		COLOR(GREY);
		printk("-");
	} وإلا إذا (k_mem_ورقہ_چوکھٹا_is_reserved(pf)) {
		COLOR(CYAN);
		printk("R");
	} وإلا إذا (k_mem_ورقہ_چوکھٹا_is_busy(pf)) {
		COLOR(MAGENTA);
		printk("B");
	} وإلا إذا (k_mem_ورقہ_چوکھٹا_is_pinned(pf)) {
		COLOR(YELLOW);
		printk("P");
	} وإلا إذا (k_mem_ورقہ_چوکھٹا_is_دستیاب(pf)) {
		COLOR(GREY);
		printk(".");
	} وإلا إذا (k_mem_ورقہ_چوکھٹا_is_mapped(pf)) {
		COLOR(DEFAULT);
		printk("M");
	} وإلا {
		COLOR(RED);
		printk("?");
	}
}

فراغ k_mem_ورقہ_چوکھٹے_dump(فراغ)
{
	صحيح کالم = 0;

	__ASSERT(ورقہ_چوکھٹے_initialized, "%s called too early", __func__);
	printk("Physical memory from 0x%lx to 0x%lx\n",
	       K_MEM_PHYS_RAM_شروع, K_MEM_PHYS_RAM_سرا);

	لكل (صحيح i = 0; i < K_MEM_عدد_ورقہ_چوکھٹے; i++) {
		بنية k_mem_ورقہ_چوکھٹا *pf = &k_mem_ورقہ_چوکھٹے[i];

		ورقہ_چوکھٹا_dump(pf);

		کالم++;
		إذا (کالم == 64) {
			کالم = 0;
			printk("\n");
		}
	}

	COLOR(DEFAULT);
	إذا (کالم != 0) {
		printk("\n");
	}
}
/* LCOV_EXCL_STOP */

#تعریف_کرو VIRT_FOREACH(ادھار_2, ناپ_2, _pos) \
	لكل ((_pos) = (ادھار_2); \
	     (_pos) < ((uint8_t *)(ادھار_2) + (ناپ_2)); (_pos) += CONFIG_MMU_ورقہ_ناپ)

#تعریف_کرو PHYS_FOREACH(ادھار_2, ناپ_2, _pos) \
	لكل ((_pos) = (ادھار_2); \
	     (_pos) < ((uintptr_t)(ادھار_2) + (ناپ_2)); (_pos) += CONFIG_MMU_ورقہ_ناپ)


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
			   CONFIG_KERNEL_VM_ناپ / CONFIG_MMU_ورقہ_ناپ);

ساكن bool virt_region_inited;

#تعریف_کرو Z_VIRT_REGION_شروع_ADDR	K_MEM_VM_FREE_شروع
#تعریف_کرو Z_VIRT_REGION_سرا_ADDR		(K_MEM_VIRT_RAM_سرا - K_MEM_VM_RESERVED)

ساكن مضمن uintptr_t virt_from_bitmap_وتھ(size_t وتھ, size_t ناپ)
{
	أرجع اشارہ_گر_منزل_UINT(K_MEM_VIRT_RAM_سرا)
	       - (وتھ * CONFIG_MMU_ورقہ_ناپ) - ناپ;
}

ساكن مضمن size_t virt_منزل_bitmap_وتھ(فراغ *vaddr, size_t ناپ)
{
	أرجع (اشارہ_گر_منزل_UINT(K_MEM_VIRT_RAM_سرا)
		- اشارہ_گر_منزل_UINT(vaddr) - ناپ) / CONFIG_MMU_ورقہ_ناپ;
}

ساكن فراغ virt_region_init(فراغ)
{
	size_t وتھ, عدد_بٹ;

	/* There are regions where we should never map via
	 * k_mem_map() and k_mem_map_phys_bare(). Mark them as
	 * already allocated so they will never be used.
	 */

	إذا (K_MEM_VM_RESERVED > 0) {
		/* Mark reserved region at end of virtual address space */
		عدد_بٹ = K_MEM_VM_RESERVED / CONFIG_MMU_ورقہ_ناپ;
		(فراغ)sys_bitarray_سیٹ_region(&virt_region_bitmap,
					      عدد_بٹ, 0);
	}

	/* Mark all bits up to Z_FREE_VM_START as allocated */
	عدد_بٹ = اشارہ_گر_منزل_UINT(K_MEM_VM_FREE_شروع)
		   - اشارہ_گر_منزل_UINT(K_MEM_VIRT_RAM_شروع);
	وتھ = virt_منزل_bitmap_وتھ(K_MEM_VIRT_RAM_شروع, عدد_بٹ);
	عدد_بٹ /= CONFIG_MMU_ورقہ_ناپ;
	(فراغ)sys_bitarray_سیٹ_region(&virt_region_bitmap,
				      عدد_بٹ, وتھ);

	virt_region_inited = true;
}

ساكن فراغ virt_region_free(فراغ *vaddr, size_t ناپ)
{
	size_t وتھ, عدد_بٹ;
	uint8_t *vaddr_u8 = (uint8_t *)vaddr;

	إذا (unlikely(!virt_region_inited)) {
		virt_region_init();
	}

#جے_بنا_تعریف CONFIG_KERNEL_سدھا_MAP
	/* Without the need to support K_MEM_DIRECT_MAP, the region must be
	 * able to be represented in the bitmap. So this case is
	 * simple.
	 */

	__ASSERT((vaddr_u8 >= Z_VIRT_REGION_شروع_ADDR)
		 && ((vaddr_u8 + ناپ - 1) < Z_VIRT_REGION_سرا_ADDR),
		 "invalid virtual address region %p (%zu)", vaddr_u8, ناپ);
	إذا (!((vaddr_u8 >= Z_VIRT_REGION_شروع_ADDR)
	      && ((vaddr_u8 + ناپ - 1) < Z_VIRT_REGION_سرا_ADDR))) {
		أرجع;
	}

	وتھ = virt_منزل_bitmap_وتھ(vaddr, ناپ);
	عدد_بٹ = ناپ / CONFIG_MMU_ورقہ_ناپ;
	(فراغ)sys_bitarray_free(&virt_region_bitmap, عدد_بٹ, وتھ);
#وإلا /* !CONFIG_KERNEL_DIRECT_MAP */
	/* With K_MEM_DIRECT_MAP, the region can be outside of the virtual
	 * memory space, wholly within it, or overlap partially.
	 * So additional processing is needed to make sure we only
	 * mark the pages within the bitmap.
	 */
	إذا (((vaddr_u8 >= Z_VIRT_REGION_شروع_ADDR) &&
	     (vaddr_u8 < Z_VIRT_REGION_سرا_ADDR)) ||
	    (((vaddr_u8 + ناپ - 1) >= Z_VIRT_REGION_شروع_ADDR) &&
	     ((vaddr_u8 + ناپ - 1) < Z_VIRT_REGION_سرا_ADDR))) {
		uint8_t *adjusted_شروع = max(vaddr_u8, Z_VIRT_REGION_شروع_ADDR);
		uint8_t *adjusted_سرا = min(vaddr_u8 + ناپ,
					    Z_VIRT_REGION_سرا_ADDR);
		size_t adjusted_sz = adjusted_سرا - adjusted_شروع;

		وتھ = virt_منزل_bitmap_وتھ(adjusted_شروع, adjusted_sz);
		عدد_بٹ = adjusted_sz / CONFIG_MMU_ورقہ_ناپ;
		(فراغ)sys_bitarray_free(&virt_region_bitmap, عدد_بٹ, وتھ);
	}
#جے_انت /* !CONFIG_KERNEL_DIRECT_MAP */
}

ساكن فراغ *virt_region_alloc(size_t ناپ, size_t align)
{
	uintptr_t dest_addr;
	size_t alloc_ناپ;
	size_t وتھ;
	size_t عدد_بٹ;
	صحيح ret;

	إذا (unlikely(!virt_region_inited)) {
		virt_region_init();
	}

	/* Possibly request more pages to ensure we can get an aligned virtual address */
	عدد_بٹ = (ناپ + align - CONFIG_MMU_ورقہ_ناپ) / CONFIG_MMU_ورقہ_ناپ;
	alloc_ناپ = عدد_بٹ * CONFIG_MMU_ورقہ_ناپ;
	ret = sys_bitarray_alloc(&virt_region_bitmap, عدد_بٹ, &وتھ);
	إذا (ret != 0) {
		LOG_ERR("insufficient virtual address space (requested %zu)",
			ناپ);
		أرجع عدم;
	}

	/* Remember that bit #0 in bitmap corresponds to the highest
	 * virtual address. So here we need to go downwards (backwards?)
	 * to get the starting address of the allocated region.
	 */
	dest_addr = virt_from_bitmap_وتھ(وتھ, alloc_ناپ);

	إذا (alloc_ناپ > ناپ) {
		uintptr_t aligned_dest_addr = گیڑا_اتے(dest_addr, align);

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
		virt_region_free(UINT_منزل_اشارہ_گر(dest_addr),
				 aligned_dest_addr - dest_addr);
		إذا (((dest_addr + alloc_ناپ) - (aligned_dest_addr + ناپ)) > 0) {
			virt_region_free(UINT_منزل_اشارہ_گر(aligned_dest_addr + ناپ),
					 (dest_addr + alloc_ناپ) - (aligned_dest_addr + ناپ));
		}

		dest_addr = aligned_dest_addr;
	}

	/* Need to make sure this does not step into kernel memory */
	إذا (dest_addr < اشارہ_گر_منزل_UINT(Z_VIRT_REGION_شروع_ADDR)) {
		(فراغ)sys_bitarray_free(&virt_region_bitmap, عدد_بٹ, وتھ);
		أرجع عدم;
	}

	أرجع UINT_منزل_اشارہ_گر(dest_addr);
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
BUILD_ASSERT(SYS_SFLIST_FLAG_بٹ >= 1, "MMU free-page list needs one sflist flag bit");

ساكن sys_sflist_t free_ورقہ_چوکھٹا_list;

/* Number of unused and available free page frames.
 * This information may go stale immediately.
 */
ساكن size_t z_free_ورقہ_گنتی;

#تعریف_کرو PF_ASSERT(pf, اظہاریہ, fmt, ...) \
	__ASSERT(اظہاریہ, "page frame 0x%lx: " fmt, k_mem_ورقہ_چوکھٹا_منزل_phys(pf), \
		 ##__VA_ARGS__)

/* Get an unused page frame. don't care which one, or NULL if there are none */
ساكن بنية k_mem_ورقہ_چوکھٹا *free_ورقہ_چوکھٹا_list_get(فراغ)
{
	sys_sfnode_t *گرہ;
	بنية k_mem_ورقہ_چوکھٹا *pf = عدم;

	گرہ = sys_sflist_get(&free_ورقہ_چوکھٹا_list);
	إذا (گرہ != عدم) {
		z_free_ورقہ_گنتی--;
		pf = CONTAINER_دا(گرہ, بنية k_mem_ورقہ_چوکھٹا, گرہ);
		PF_ASSERT(pf, k_mem_ورقہ_چوکھٹا_is_free(pf),
			 "on free list but not free");
		pf->va_and_flags = 0;
	}

	أرجع pf;
}

/* Release a page frame back into the list of free pages */
ساكن فراغ free_ورقہ_چوکھٹا_list_put(بنية k_mem_ورقہ_چوکھٹا *pf)
{
	PF_ASSERT(pf, k_mem_ورقہ_چوکھٹا_is_دستیاب(pf),
		 "unavailable page put on free list");

	sys_sfnode_init(&pf->گرہ, K_MEM_ورقہ_چوکھٹا_FREE);
	sys_sflist_append(&free_ورقہ_چوکھٹا_list, &pf->گرہ);
	z_free_ورقہ_گنتی++;
}

ساكن فراغ free_ورقہ_چوکھٹا_list_init(فراغ)
{
	sys_sflist_init(&free_ورقہ_چوکھٹا_list);
}

ساكن فراغ ورقہ_چوکھٹا_free_locked(بنية k_mem_ورقہ_چوکھٹا *pf)
{
	pf->va_and_flags = 0;
	free_ورقہ_چوکھٹا_list_put(pf);
}

/*
 * Memory Mapping
 */

/* Called after the frame is mapped in the arch layer, to update our
 * local ontology (and do some assertions while we're at it)
 */
ساكن فراغ چوکھٹا_mapped_سیٹ(بنية k_mem_ورقہ_چوکھٹا *pf, فراغ *addr)
{
	PF_ASSERT(pf, !k_mem_ورقہ_چوکھٹا_is_free(pf),
		  "attempted to map a page frame on the free list");
	PF_ASSERT(pf, !k_mem_ورقہ_چوکھٹا_is_reserved(pf),
		  "attempted to map a reserved page frame");

	/* We do allow multiple mappings for pinned page frames
	 * since we will never need to reverse map them.
	 * This is uncommon, use-cases are for things like the
	 * Zephyr equivalent of VSDOs
	 */
	PF_ASSERT(pf, !k_mem_ورقہ_چوکھٹا_is_mapped(pf) || k_mem_ورقہ_چوکھٹا_is_pinned(pf),
		 "non-pinned and already mapped to %p",
		 k_mem_ورقہ_چوکھٹا_منزل_virt(pf));

	uintptr_t flags_نقاب = CONFIG_MMU_ورقہ_ناپ - 1;
	uintptr_t va = (uintptr_t)addr & ~flags_نقاب;

	pf->va_and_flags &= flags_نقاب;
	pf->va_and_flags |= va | K_MEM_ورقہ_چوکھٹا_MAPPED;
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
ساكن صحيح virt_منزل_ورقہ_چوکھٹا(فراغ *virt, uintptr_t *phys)
{
	uintptr_t paddr;
	بنية k_mem_ورقہ_چوکھٹا *pf;
	صحيح ret = -EFAULT;

	K_MEM_ورقہ_چوکھٹا_FOREACH(paddr, pf) {
		إذا (k_mem_ورقہ_چوکھٹا_is_mapped(pf)) {
			إذا (virt == k_mem_ورقہ_چوکھٹا_منزل_virt(pf)) {
				ret = 0;
				إذا (phys != عدم) {
					*phys = k_mem_ورقہ_چوکھٹا_منزل_phys(pf);
				}
				اكسر;
			}
		}
	}

	أرجع ret;
}
/* LCOV_EXCL_STOP */

__weak FUNC_ALIAS(virt_منزل_ورقہ_چوکھٹا, arch_ورقہ_phys_get, صحيح);

#جے_تعریف_شدہ CONFIG_DEMAND_PAGING
ساكن صحيح ورقہ_چوکھٹا_prepare_locked(بنية k_mem_ورقہ_چوکھٹا *pf, bool *dirty_ptr,
				     bool ورقہ_وچ, uintptr_t *location_ptr);

ساكن مضمن فراغ do_backing_store_ورقہ_وچ(uintptr_t location);
ساكن مضمن فراغ do_backing_store_ورقہ_باہرلا(uintptr_t location);
#جے_انت /* CONFIG_DEMAND_PAGING */

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
ساكن صحيح map_anon_ورقہ(فراغ *addr, uint32_t flags, bool *recovered)
{
	بنية k_mem_ورقہ_چوکھٹا *pf;
	uintptr_t phys;
	bool جندرا = (flags & K_MEM_MAP_جندرا) != 0U;
	صحيح ret;

	*recovered = true;

	pf = free_ورقہ_چوکھٹا_list_get();
	إذا (pf == عدم) {
#جے_تعریف_شدہ CONFIG_DEMAND_PAGING
		uintptr_t location;
		bool dirty;

		pf = k_mem_paging_eviction_select(&dirty);
		إذا (pf == عدم) {
			/* Every evictable page frame is pinned or busy */
			أرجع -ENOMEM;
		}
		LOG_DBG("evicting %p at 0x%lx",
			k_mem_ورقہ_چوکھٹا_منزل_virt(pf),
			k_mem_ورقہ_چوکھٹا_منزل_phys(pf));
		ret = ورقہ_چوکھٹا_prepare_locked(pf, &dirty, false, &location);
		إذا (ret != 0) {
			أرجع -ENOMEM;
		}
		إذا (dirty) {
			do_backing_store_ورقہ_باہرلا(location);
		}
		pf->va_and_flags = 0;
#وإلا
		أرجع -ENOMEM;
#جے_انت /* CONFIG_DEMAND_PAGING */
	}

	phys = k_mem_ورقہ_چوکھٹا_منزل_phys(pf);
	ret = arch_mem_map(addr, phys, CONFIG_MMU_ورقہ_ناپ, flags);
	__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
	إذا (ret != 0) {
#جے_تعریف_شدہ CONFIG_USERSPACE
		/* The mapping above may have already succeeded in some
		 * memory domains but failing in the middle. So we need to
		 * unmap those first before returning the page frame back
		 * to the list.
		 */
		صحيح unmap_ret = arch_mem_unmap(addr, CONFIG_MMU_ورقہ_ناپ);

		__ASSERT(unmap_ret == 0, "%s: cannot undo failed mapping %d\n", __func__,
			 unmap_ret);

		إذا (unmap_ret != 0) {
			/* The page may still be mapped in some memory domains,
			 * so the page frame cannot be given back to the free
			 * list for reuse.
			 */
			*recovered = false;
			أرجع ret;
		}
#جے_انت

		/* Mapping failed: give the page frame back before bailing out. */
		ورقہ_چوکھٹا_free_locked(pf);
		أرجع ret;
	}

	إذا (جندرا) {
		k_mem_ورقہ_چوکھٹا_سیٹ(pf, K_MEM_ورقہ_چوکھٹا_PINNED);
	}
	چوکھٹا_mapped_سیٹ(pf, addr);
#جے_تعریف_شدہ CONFIG_DEMAND_PAGING
	إذا (IS_ENABLED(CONFIG_EVICTION_TRACKING) && (!جندرا)) {
		k_mem_paging_eviction_جوڑو(pf);
	}
#جے_انت

	LOG_DBG("memory mapping anon page %p -> 0x%lx", addr, phys);

	أرجع 0;
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
ساكن صحيح unmap_anon_ورقے(فراغ *virt, size_t sz)
{
	uint8_t *pos;
	uintptr_t phys;
	بنية k_mem_ورقہ_چوکھٹا *pf;
	صحيح ret = 0;

	VIRT_FOREACH(virt, sz, pos) {
#جے_تعریف_شدہ CONFIG_DEMAND_PAGING
		تعداد arch_ورقہ_location حالت;
		uintptr_t location;

		حالت = arch_ورقہ_location_get(pos, &location);
		بدل (حالت) {
		حالة ARCH_ورقہ_LOCATION_PAGED_باہرلا:
			/*
			 * No pf is associated with this mapping.
			 * Simply get rid of the MMU entry and free
			 * corresponding backing store.
			 */
			ret = arch_mem_unmap(pos, CONFIG_MMU_ورقہ_ناپ);
			__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__,
				 ret);

			k_mem_paging_backing_store_location_free(location);

			إذا (ret != 0) {
				اذهب unmap_anon_باہرلا;
			}

			استمر;
		حالة ARCH_ورقہ_LOCATION_PAGED_وچ:
			/*
			 * The page is in memory but it may not be
			 * accessible in order to manage tracking
			 * of the ARCH_DATA_PAGE_ACCESSED flag
			 * meaning arch_page_phys_get() could fail.
			 * Still, we know the actual phys address.
			 */
			phys = location;
			ret = 0;
			اكسر;
		افتراضي:
			ret = arch_ورقہ_phys_get(pos, &phys);
			اكسر;
		}
#وإلا
		ret = arch_ورقہ_phys_get(pos, &phys);
#جے_انت
		__ASSERT(ret == 0, "%s: cannot unmap an unmapped address %p", __func__, pos);
		إذا (ret != 0) {
			/* Found an address not mapped. Do not continue. */
			اذهب unmap_anon_باہرلا;
		}

		__ASSERT(k_mem_is_ورقہ_چوکھٹا(phys), "%s: 0x%lx is not a page frame", __func__,
			 phys);
		إذا (!k_mem_is_ورقہ_چوکھٹا(phys)) {
			/* Physical address has no corresponding page frame
			 * description in the page frame array.
			 * This should not happen. Do not continue.
			 */
			اذهب unmap_anon_باہرلا;
		}

		/* Grab the corresponding page frame from physical address */
		pf = k_mem_phys_منزل_ورقہ_چوکھٹا(phys);

		__ASSERT(k_mem_ورقہ_چوکھٹا_is_mapped(pf), "%s: 0x%lx is not a mapped page frame",
			 __func__, phys);
		إذا (!k_mem_ورقہ_چوکھٹا_is_mapped(pf)) {
			/* Page frame is not marked mapped.
			 * This should not happen. Do not continue.
			 */
			اذهب unmap_anon_باہرلا;
		}

		ret = arch_mem_unmap(pos, CONFIG_MMU_ورقہ_ناپ);
		__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
		إذا (ret != 0) {
			/* Fail to unmap. Must bail without releasing any resources
			 * as the page is still mapped.
			 */
			اذهب unmap_anon_باہرلا;
		}

#جے_تعریف_شدہ CONFIG_DEMAND_PAGING
		إذا (IS_ENABLED(CONFIG_EVICTION_TRACKING) &&
		    (!k_mem_ورقہ_چوکھٹا_is_pinned(pf))) {
			k_mem_paging_eviction_remove(pf);
		}
#جے_انت

		/* Put the page frame back into free list */
		ورقہ_چوکھٹا_free_locked(pf);
	}

unmap_anon_باہرلا:
	أرجع ret;
}

#جے_تعریف_شدہ CONFIG_DEMAND_MAPPING
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
ساكن bool undo_demand_mapped_ورقے(فراغ *virt, size_t sz)
{
	uint8_t *undo_pos;
	bool success = true;
	صحيح ret;

	/* Undo the pages mapped before the failure. */
	VIRT_FOREACH(virt, sz, undo_pos) {
		ret = arch_mem_unmap(undo_pos, CONFIG_MMU_ورقہ_ناپ);
		__ASSERT(ret == 0, "%s: cannot undo mapped pages %d", __func__, ret);

		/* Failed to undo one mapped page. Though we still
		 * want to continue unmapping the remaining mapped
		 * pages to reclaim them.
		 */
		إذا (ret != 0) {
			success = false;
		}
	}

	أرجع success;
}
#جے_انت

فراغ *k_mem_map_phys_guard(uintptr_t phys, size_t ناپ, uint32_t flags, bool is_anon)
{
	uint8_t *dst;
	uint8_t *region;
	size_t کل_ناپ;
	صحيح ret;
	k_spinlock_کلید_t کلید;
	uint8_t *pos;
	bool uninit = (flags & K_MEM_MAP_UNINIT) != 0U;

	__ASSERT(!is_anon || (is_anon && ورقہ_چوکھٹے_initialized),
		 "%s called too early", __func__);
	__ASSERT((flags & K_MEM_CACHE_نقاب) == 0U,
		 "%s does not support explicit cache settings", __func__);

	إذا (((flags & K_MEM_PERM_USER) != 0U) &&
	    ((flags & K_MEM_MAP_UNINIT) != 0U)) {
		LOG_ERR("user access to anonymous uninitialized pages is forbidden");
		أرجع عدم;
	}
	إذا ((ناپ % CONFIG_MMU_ورقہ_ناپ) != 0U) {
		LOG_ERR("unaligned size %zu passed to %s", ناپ, __func__);
		أرجع عدم;
	}
	إذا (ناپ == 0) {
		LOG_ERR("zero sized memory mapping");
		أرجع عدم;
	}

	/* Need extra for the guard pages (before and after) which we
	 * won't map.
	 */
	إذا (ناپ_جوڑو_حد_توں_ودھ(ناپ, CONFIG_MMU_ورقہ_ناپ * 2, &کل_ناپ)) {
		LOG_ERR("too large size %zu passed to %s", ناپ, __func__);
		أرجع عدم;
	}

	کلید = k_spin_جندرا(&z_mm_جندرا);

	region = virt_region_alloc(کل_ناپ, CONFIG_MMU_ورقہ_ناپ);
	إذا (region == عدم) {
		/* Address space has no free region */
		dst = عدم;
		اذهب باہرلا;
	}

	/* Unmap both guard pages to make sure accessing them
	 * will generate fault.
	 *
	 * Note that a failed unmap may have left the guard page mapped in
	 * some memory domains, so the virtual region cannot be freed for
	 * reuse.
	 */
	ret = arch_mem_unmap(region, CONFIG_MMU_ورقہ_ناپ);
	__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
	إذا (ret != 0) {
		dst = عدم;
		اذهب باہرلا;
	}

	ret = arch_mem_unmap(region + CONFIG_MMU_ورقہ_ناپ + ناپ, CONFIG_MMU_ورقہ_ناپ);
	__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
	إذا (ret != 0) {
		dst = عدم;
		اذهب باہرلا;
	}

	/* Skip over the "before" guard page in returned address. */
	dst = region + CONFIG_MMU_ورقہ_ناپ;

	إذا (is_anon) {
		/* Mapping from anonymous memory */
		flags |= K_MEM_CACHE_WB;
#جے_تعریف_شدہ CONFIG_DEMAND_MAPPING
		إذا ((flags & K_MEM_MAP_جندرا) == 0) {
			flags |= K_MEM_MAP_UNPAGED;
			VIRT_FOREACH(dst, ناپ, pos) {
				ret = arch_mem_map(pos,
						   uninit ? ARCH_UNPAGED_ANON_UNINIT
							  : ARCH_UNPAGED_ANON_ZERO,
						   CONFIG_MMU_ورقہ_ناپ, flags);
				__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
				إذا (ret != 0) {
					size_t unmap_sz = pos - dst;

					إذا (IS_ENABLED(CONFIG_USERSPACE)) {
						/* Note that it is unmapping an extra page due to
						 * the possibility that the mapping above has
						 * already updated some memory domains but fail in
						 * the middle. So we need to include the failed
						 * page to unmap those that have been mapped so
						 * far.
						 */
						unmap_sz += CONFIG_MMU_ورقہ_ناپ;
					}

					bool success = undo_demand_mapped_ورقے(dst, unmap_sz);

					dst = عدم;

					إذا (success) {
						اذهب fail;
					} وإلا {
						/* Cannot fully recover from mapping
						 * failure. We cannot free the allocated
						 * virtual memory region since some of
						 * the page may still be mapped.
						 */
						اذهب باہرلا;
					}
				}
			}
			LOG_DBG("memory mapping anon pages %p to %p unpaged", dst, pos-1);
			/* skip the memset() below */
			uninit = true;
		} وإلا
#جے_انت
		{
			VIRT_FOREACH(dst, ناپ, pos) {
				bool recovered;

				ret = map_anon_ورقہ(pos, flags, &recovered);

				إذا (ret != 0) {
					صحيح unmap_ret = unmap_anon_ورقے(dst, pos - dst);

					dst = عدم;

					إذا ((unmap_ret == 0) && recovered) {
						اذهب fail;
					} وإلا {
						/* Cannot fully recover from mapping
						 * failure. We cannot free the allocated
						 * virtual memory region since some of
						 * the page may still be mapped.
						 */
						اذهب باہرلا;
					}
				}
			}
		}
	} وإلا {
		/* Mapping known physical memory. */
		ret = arch_mem_map(dst, phys, ناپ, flags);
		__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
		إذا (ret != 0) {
			/* Clean up any partial mapping left behind. */
			ret = arch_mem_unmap(dst, ناپ);
			__ASSERT(ret == 0, "%s: cannot undo mapped pages %d", __func__, ret);

			dst = عدم;

			إذا (ret != 0) {
				/* Cannot fully undo all the mappings. There might still be pages
				 * mapped. So we cannot free the allocated virtual memory region.
				 */
				اذهب باہرلا;
			} وإلا {
				اذهب fail;
			}
		}
	}

باہرلا:
	k_spin_unlock(&z_mm_جندرا, کلید);

	إذا (dst != عدم && !uninit) {
		/* If we later implement mappings to a copy-on-write
		 * zero page, won't need this step
		 */
		memset(dst, 0, ناپ);
	}

	أرجع dst;

fail:
	/* Release the whole region, including the guard pages. */
	virt_region_free(region, کل_ناپ);

	اذهب باہرلا;
}

فراغ k_mem_unmap_phys_guard(فراغ *addr, size_t ناپ, bool is_anon)
{
	uint8_t *pos;
	k_spinlock_کلید_t کلید;
	size_t کل_ناپ;
	صحيح ret;

	/* Need space for the "before" guard page */
	__ASSERT_NO_MSG(اشارہ_گر_منزل_UINT(addr) >= CONFIG_MMU_ورقہ_ناپ);

	/* Make sure address range is still valid after accounting
	 * for two guard pages.
	 */
	pos = (uint8_t *)addr - CONFIG_MMU_ورقہ_ناپ;
	k_mem_assert_virtual_region(pos, ناپ + (CONFIG_MMU_ورقہ_ناپ * 2));

	کلید = k_spin_جندرا(&z_mm_جندرا);

	/* Check if both guard pages are unmapped.
	 * Bail if not, as this is probably a region not mapped
	 * using k_mem_map().
	 */
	pos = addr;
	ret = arch_ورقہ_phys_get(pos - CONFIG_MMU_ورقہ_ناپ, عدم);
	__ASSERT(ret != 0,
		 "%s: cannot find preceding guard page for (%p, %zu)",
		 __func__, addr, ناپ);
	إذا (ret == 0) {
		اذهب باہرلا;
	}

	ret = arch_ورقہ_phys_get(pos + ناپ, عدم);
	__ASSERT(ret != 0,
		 "%s: cannot find succeeding guard page for (%p, %zu)",
		 __func__, addr, ناپ);
	إذا (ret == 0) {
		اذهب باہرلا;
	}

	إذا (is_anon) {
		/* Unmapping anonymous memory */
		ret = unmap_anon_ورقے(addr, ناپ);
		__ASSERT(ret == 0, "%s: unmap_anon_pages() returned %d", __func__, ret);
		إذا (ret != 0) {
			/* Fail to unmap. Must bail without releasing any resources
			 * as some pages are still be mapped.
			 */
			اذهب باہرلا;
		}
	} وإلا {
		/*
		 * Unmapping previous mapped memory with specific physical address.
		 *
		 * Note that we don't have to unmap the guard pages, as they should
		 * have been unmapped. We just need to unmapped the in-between
		 * region [addr, (addr + size)).
		 */
		ret = arch_mem_unmap(addr, ناپ);
		__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
		إذا (ret != 0) {
			/* Fail to unmap. Must bail without releasing any resources
			 * as some pages are still be mapped.
			 */
			اذهب باہرلا;
		}
	}

	/* There are guard pages just before and after the mapped
	 * region. So we also need to free them from the bitmap.
	 */
	pos = (uint8_t *)addr - CONFIG_MMU_ورقہ_ناپ;
	کل_ناپ = ناپ + (CONFIG_MMU_ورقہ_ناپ * 2);
	virt_region_free(pos, کل_ناپ);

باہرلا:
	k_spin_unlock(&z_mm_جندرا, کلید);
}

صحيح k_mem_update_flags(فراغ *addr, size_t ناپ, uint32_t flags)
{
	uintptr_t phys;
	k_spinlock_کلید_t کلید;
	صحيح ret;

	k_mem_assert_virtual_region(addr, ناپ);

	کلید = k_spin_جندرا(&z_mm_جندرا);

	/*
	 * We can achieve desired result without explicit architecture support
	 * by unmapping and remapping the same physical memory using new flags.
	 */

	ret = arch_ورقہ_phys_get(addr, &phys);
	إذا (ret < 0) {
		اذهب باہرلا;
	}

	/* TODO: detect and handle paged-out memory as well */

	ret = arch_mem_unmap(addr, ناپ);
	__ASSERT(ret == 0, "%s: arch_mem_unmap failed %d\n", __func__, ret);
	إذا (ret != 0) {
		اذهب باہرلا;
	}

	ret = arch_mem_map(addr, phys, ناپ, flags);
	__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);

باہرلا:
	k_spin_unlock(&z_mm_جندرا, کلید);
	أرجع ret;
}

size_t k_mem_free_get(فراغ)
{
	size_t ret;
	k_spinlock_کلید_t کلید;

	__ASSERT(ورقہ_چوکھٹے_initialized, "%s called too early", __func__);

	کلید = k_spin_جندرا(&z_mm_جندرا);
#جے_تعریف_شدہ CONFIG_DEMAND_PAGING
	إذا (z_free_ورقہ_گنتی > CONFIG_DEMAND_PAGING_ورقہ_چوکھٹے_RESERVE) {
		ret = z_free_ورقہ_گنتی - CONFIG_DEMAND_PAGING_ورقہ_چوکھٹے_RESERVE;
	} وإلا {
		ret = 0;
	}
#وإلا
	ret = z_free_ورقہ_گنتی;
#جے_انت /* CONFIG_DEMAND_PAGING */
	k_spin_unlock(&z_mm_جندرا, کلید);

	أرجع ret * (size_t)CONFIG_MMU_ورقہ_ناپ;
}

/* Get the default virtual region alignment, here the default MMU page size
 *
 * @param[in] phys Physical address of region to be mapped, aligned to MMU_PAGE_SIZE
 * @param[in] size Size of region to be mapped, aligned to MMU_PAGE_SIZE
 *
 * @return alignment to apply on the virtual address of this region
 */
ساكن size_t virt_region_align(uintptr_t phys, size_t ناپ)
{
	ARG_بن_ورتیا(phys);
	ARG_بن_ورتیا(ناپ);

	أرجع CONFIG_MMU_ورقہ_ناپ;
}

__weak FUNC_ALIAS(virt_region_align, arch_virt_region_align, size_t);

/* This may be called from arch early boot code before z_cstart() is invoked.
 * Data will be copied and BSS zeroed, but this must not rely on any
 * initialization functions being called prior to work correctly.
 */
فراغ k_mem_map_phys_bare(uint8_t **virt_ptr, uintptr_t phys, size_t ناپ, uint32_t flags)
{
	uintptr_t aligned_phys, addr_وتھ;
	size_t aligned_ناپ, align_boundary;
	k_spinlock_کلید_t کلید;
	uint8_t *dest_addr;
	size_t عدد_بٹ;
	size_t وتھ;
	صحيح ret;

#جے_بنا_تعریف CONFIG_KERNEL_سدھا_MAP
	__ASSERT(!(flags & K_MEM_سدھا_MAP), "The direct-map is not enabled");
#جے_انت /* CONFIG_KERNEL_DIRECT_MAP */
	addr_وتھ = k_mem_region_align(&aligned_phys, &aligned_ناپ,
					 phys, ناپ,
					 CONFIG_MMU_ورقہ_ناپ);
	__ASSERT(aligned_ناپ != 0U, "0-length mapping at 0x%lx", aligned_phys);
	__ASSERT(aligned_ناپ - 1 <= (UINTPTR_MAX - aligned_phys),
		 "wraparound for physical address 0x%lx (size %zu)",
		 aligned_phys, aligned_ناپ);

	align_boundary = arch_virt_region_align(aligned_phys, aligned_ناپ);

	کلید = k_spin_جندرا(&z_mm_جندرا);

	إذا (IS_ENABLED(CONFIG_KERNEL_سدھا_MAP) &&
	    (flags & K_MEM_سدھا_MAP)) {
		dest_addr = (uint8_t *)aligned_phys;

		/* Mark the region of virtual memory bitmap as used
		 * if the region overlaps the virtual memory space.
		 *
		 * Basically if either end of region is within
		 * virtual memory space, we need to mark the bits.
		 */

		إذا (وچ_RANGE(aligned_phys,
			      (uintptr_t)K_MEM_VIRT_RAM_شروع,
			      (uintptr_t)(K_MEM_VIRT_RAM_سرا - 1)) ||
		    وچ_RANGE(aligned_phys + aligned_ناپ - 1,
			      (uintptr_t)K_MEM_VIRT_RAM_شروع,
			      (uintptr_t)(K_MEM_VIRT_RAM_سرا - 1))) {
			uint8_t *adjusted_شروع = max(dest_addr, K_MEM_VIRT_RAM_شروع);
			uint8_t *adjusted_سرا = min(dest_addr + aligned_ناپ,
						    K_MEM_VIRT_RAM_سرا);
			size_t adjusted_sz = adjusted_سرا - adjusted_شروع;

			عدد_بٹ = adjusted_sz / CONFIG_MMU_ورقہ_ناپ;
			وتھ = virt_منزل_bitmap_وتھ(adjusted_شروع, adjusted_sz);
			إذا (sys_bitarray_test_and_سیٹ_region(
			    &virt_region_bitmap, عدد_بٹ, وتھ, true)) {
				اذهب fail;
			}
		}
	} وإلا {
		/* Obtain an appropriately sized chunk of virtual memory */
		dest_addr = virt_region_alloc(aligned_ناپ, align_boundary);
		إذا (!dest_addr) {
			اذهب fail;
		}
	}

	/* If this fails there's something amiss with virt_region_get */
	__ASSERT((uintptr_t)dest_addr <
		 ((uintptr_t)dest_addr + (ناپ - 1)),
		 "wraparound for virtual address %p (size %zu)",
		 dest_addr, ناپ);

	LOG_DBG("arch_mem_map(%p, 0x%lx, %zu, %x) offset %lu", (فراغ *)dest_addr,
		aligned_phys, aligned_ناپ, flags, addr_وتھ);

	ret = arch_mem_map(dest_addr, aligned_phys, aligned_ناپ, flags);
	__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
	إذا (ret != 0) {
		اذهب fail;
	}
	k_spin_unlock(&z_mm_جندرا, کلید);

	*virt_ptr = dest_addr + addr_وتھ;
	أرجع;
fail:
	/* May re-visit this in the future, but for now running out of
	 * virtual address space or failing the arch_mem_map() call is
	 * an unrecoverable situation.
	 *
	 * Other problems not related to resource exhaustion we leave as
	 * assertions since they are clearly programming mistakes.
	 */
	LOG_ERR("memory mapping 0x%lx (size %zu, flags 0x%x) failed",
		phys, ناپ, flags);
	k_panic();
}

فراغ k_mem_unmap_phys_bare(uint8_t *virt, size_t ناپ)
{
	uintptr_t aligned_virt, addr_وتھ;
	size_t aligned_ناپ;
	k_spinlock_کلید_t کلید;
	صحيح ret;

	addr_وتھ = k_mem_region_align(&aligned_virt, &aligned_ناپ,
					 اشارہ_گر_منزل_UINT(virt), ناپ,
					 CONFIG_MMU_ورقہ_ناپ);
	__ASSERT(aligned_ناپ != 0U, "0-length mapping at 0x%lx", aligned_virt);
	__ASSERT(aligned_ناپ - 1 <= (UINTPTR_MAX - aligned_virt),
		 "wraparound for virtual address 0x%lx (size %zu)",
		 aligned_virt, aligned_ناپ);

	کلید = k_spin_جندرا(&z_mm_جندرا);

	LOG_DBG("arch_mem_unmap(0x%lx, %zu) offset %lu",
		aligned_virt, aligned_ناپ, addr_وتھ);

	ret = arch_mem_unmap(UINT_منزل_اشارہ_گر(aligned_virt), aligned_ناپ);
	__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
	إذا (ret != 0) {
		/* Fail to unmap. We cannot free the virtual memory region
		 * as some pages are still be mapped.
		 */
		اذهب باہرلا;
	}

	virt_region_free(UINT_منزل_اشارہ_گر(aligned_virt), aligned_ناپ);

باہرلا:
	k_spin_unlock(&z_mm_جندرا, کلید);
}

/*
 * Miscellaneous
 */

size_t k_mem_region_align(uintptr_t *aligned_addr, size_t *aligned_ناپ,
			  uintptr_t addr, size_t ناپ, size_t align)
{
	size_t addr_وتھ;

	/* The actual mapped region must be page-aligned. Round down the
	 * physical address and pad the region size appropriately
	 */
	*aligned_addr = گیڑا_تھلے(addr, align);
	addr_وتھ = addr - *aligned_addr;
	*aligned_ناپ = گیڑا_اتے(ناپ + addr_وتھ, align);

	أرجع addr_وتھ;
}

#جے_تعریف_شدہ CONFIG_LINKER_USE_BOOT_SECTION
ساكن فراغ mark_linker_section_pinned(فراغ *شروع_addr, فراغ *سرا_addr,
				       bool pin)
{
	بنية k_mem_ورقہ_چوکھٹا *pf;
	uint8_t *addr;

	uintptr_t pinned_شروع = گیڑا_تھلے(اشارہ_گر_منزل_UINT(شروع_addr),
					    CONFIG_MMU_ورقہ_ناپ);
	uintptr_t pinned_سرا = گیڑا_اتے(اشارہ_گر_منزل_UINT(سرا_addr),
					CONFIG_MMU_ورقہ_ناپ);
	size_t pinned_ناپ = pinned_سرا - pinned_شروع;

	VIRT_FOREACH(UINT_منزل_اشارہ_گر(pinned_شروع), pinned_ناپ, addr)
	{
		pf = k_mem_phys_منزل_ورقہ_چوکھٹا(K_MEM_BOOT_VIRT_منزل_PHYS(addr));
		چوکھٹا_mapped_سیٹ(pf, addr);

		إذا (pin) {
			k_mem_ورقہ_چوکھٹا_سیٹ(pf, K_MEM_ورقہ_چوکھٹا_PINNED);
		} وإلا {
			k_mem_ورقہ_چوکھٹا_clear(pf, K_MEM_ورقہ_چوکھٹا_PINNED);
#جے_تعریف_شدہ CONFIG_DEMAND_PAGING
			إذا (IS_ENABLED(CONFIG_EVICTION_TRACKING) &&
			    k_mem_ورقہ_چوکھٹا_is_evictable(pf)) {
				k_mem_paging_eviction_جوڑو(pf);
			}
#جے_انت
		}
	}
}
#جے_انت /* CONFIG_LINKER_USE_BOOT_SECTION */

#جے_تعریف_شدہ CONFIG_LINKER_USE_ONDEMAND_SECTION
ساكن فراغ z_paging_ondemand_section_map(فراغ)
{
	uint8_t *addr;
	size_t ناپ;
	uintptr_t location;
	uint32_t flags;
	صحيح ret;

	ناپ = (uintptr_t)lnkr_ondemand_متن_ناپ;
	flags = K_MEM_MAP_UNPAGED | K_MEM_PERM_EXEC | K_MEM_CACHE_WB;
	VIRT_FOREACH(lnkr_ondemand_متن_شروع, ناپ, addr) {
		k_mem_paging_backing_store_location_query(addr, &location);

		ret = arch_mem_map(addr, location, CONFIG_MMU_ورقہ_ناپ, flags);
		__ASSERT(ret == 0, "arch_mem_map() returned %d", ret);
		إذا (ret != 0) {
			k_panic();
		}

		sys_bitarray_سیٹ_region(&virt_region_bitmap, 1,
					virt_منزل_bitmap_وتھ(addr, CONFIG_MMU_ورقہ_ناپ));
	}

	ناپ = (uintptr_t)lnkr_ondemand_rodata_ناپ;
	flags = K_MEM_MAP_UNPAGED | K_MEM_CACHE_WB;
	VIRT_FOREACH(lnkr_ondemand_rodata_شروع, ناپ, addr) {
		k_mem_paging_backing_store_location_query(addr, &location);

		ret = arch_mem_map(addr, location, CONFIG_MMU_ورقہ_ناپ, flags);
		__ASSERT(ret == 0, "arch_mem_map() returned %d", ret);
		إذا (ret != 0) {
			k_panic();
		}

		sys_bitarray_سیٹ_region(&virt_region_bitmap, 1,
					virt_منزل_bitmap_وتھ(addr, CONFIG_MMU_ورقہ_ناپ));
	}
}
#جے_انت /* CONFIG_LINKER_USE_ONDEMAND_SECTION */

فراغ z_mem_manage_init(فراغ)
{
	uintptr_t phys;
	uint8_t *addr;
	بنية k_mem_ورقہ_چوکھٹا *pf;
	k_spinlock_کلید_t کلید = k_spin_جندرا(&z_mm_جندرا);

	free_ورقہ_چوکھٹا_list_init();

	ARG_بن_ورتیا(addr);

#جے_تعریف_شدہ CONFIG_ARCH_HAS_RESERVED_ورقہ_چوکھٹے
	/* If some page frames are unavailable for use as memory, arch
	 * code will mark K_MEM_PAGE_FRAME_RESERVED in their flags
	 */
	arch_reserved_ورقے_update();
#جے_انت /* CONFIG_ARCH_HAS_RESERVED_PAGE_FRAMES */

	/* The entire Zephyr image is mapped and pinned at boot. Demand
	 * paging applies only to anonymous mappings created via k_mem_map()
	 * and to memory placed in explicit __ondemand_* linker sections;
	 * the kernel image itself is never a candidate for eviction.
	 */
	VIRT_FOREACH(K_MEM_KERNEL_VIRT_شروع, K_MEM_KERNEL_VIRT_ناپ, addr)
	{
		pf = k_mem_phys_منزل_ورقہ_چوکھٹا(K_MEM_BOOT_VIRT_منزل_PHYS(addr));
		چوکھٹا_mapped_سیٹ(pf, addr);
		k_mem_ورقہ_چوکھٹا_سیٹ(pf, K_MEM_ورقہ_چوکھٹا_PINNED);
	}

#جے_تعریف_شدہ CONFIG_LINKER_USE_BOOT_SECTION
	/* Pin the boot section to prevent it from being swapped out during
	 * boot process. Will be un-pinned once boot process completes.
	 */
	mark_linker_section_pinned(lnkr_boot_شروع, lnkr_boot_سرا, true);
#جے_انت /* CONFIG_LINKER_USE_BOOT_SECTION */

	/* Any remaining pages that aren't mapped, reserved, or pinned get
	 * added to the free pages list
	 */
	K_MEM_ورقہ_چوکھٹا_FOREACH(phys, pf) {
		إذا (k_mem_ورقہ_چوکھٹا_is_دستیاب(pf)) {
			free_ورقہ_چوکھٹا_list_put(pf);
		}
	}
	LOG_DBG("free page frames: %zu", z_free_ورقہ_گنتی);

#جے_تعریف_شدہ CONFIG_DEMAND_PAGING
#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	z_paging_histogram_init();
#جے_انت /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
	k_mem_paging_backing_store_init();
	k_mem_paging_eviction_init();

	إذا (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
		/* start tracking evictable page installed above if any */
		K_MEM_ورقہ_چوکھٹا_FOREACH(phys, pf) {
			إذا (k_mem_ورقہ_چوکھٹا_is_evictable(pf)) {
				k_mem_paging_eviction_جوڑو(pf);
			}
		}
	}
#جے_انت /* CONFIG_DEMAND_PAGING */

#جے_تعریف_شدہ CONFIG_LINKER_USE_ONDEMAND_SECTION
	z_paging_ondemand_section_map();
#جے_انت

#جے_تعریف_شدہ CONFIG_ASSERT
	ورقہ_چوکھٹے_initialized = true;
#جے_انت /* CONFIG_ASSERT */

	k_spin_unlock(&z_mm_جندرا, کلید);
}

فراغ z_mem_manage_boot_انت(فراغ)
{
#جے_تعریف_شدہ CONFIG_LINKER_USE_BOOT_SECTION
	/* At the end of boot process, unpin the boot sections
	 * as they don't need to be in memory all the time anymore.
	 */
	mark_linker_section_pinned(lnkr_boot_شروع, lnkr_boot_سرا, false);
#جے_انت /* CONFIG_LINKER_USE_BOOT_SECTION */
}

#جے_تعریف_شدہ CONFIG_DEMAND_PAGING

#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_STATS
بنية k_mem_paging_stats_t paging_stats;
خارجي بنية k_mem_paging_histogram_t z_paging_histogram_eviction;
خارجي بنية k_mem_paging_histogram_t z_paging_histogram_backing_store_ورقہ_وچ;
خارجي بنية k_mem_paging_histogram_t z_paging_histogram_backing_store_ورقہ_باہرلا;
#جے_انت /* CONFIG_DEMAND_PAGING_STATS */

ساكن مضمن فراغ do_backing_store_ورقہ_وچ(uintptr_t location)
{
#جے_تعریف_شدہ CONFIG_DEMAND_MAPPING
	/* Check for special cases */
	بدل (location) {
	حالة ARCH_UNPAGED_ANON_ZERO:
		memset(K_MEM_SCRATCH_ورقہ, 0, CONFIG_MMU_ورقہ_ناپ);
		__fallthrough;
	حالة ARCH_UNPAGED_ANON_UNINIT:
		/* nothing else to do */
		أرجع;
	افتراضي:
		اكسر;
	}
#جے_انت /* CONFIG_DEMAND_MAPPING */

#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	uint32_t ویلا_diff;

#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	timing_t ویلا_شروع, ویلا_سرا;

	ویلا_شروع = timing_counter_get();
#وإلا
	uint32_t ویلا_شروع;

	ویلا_شروع = k_چکر_get_32();
#جے_انت /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#جے_انت /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	k_mem_paging_backing_store_ورقہ_وچ(location);

#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	ویلا_سرا = timing_counter_get();
	ویلا_diff = (uint32_t)timing_چکراں_get(&ویلا_شروع, &ویلا_سرا);
#وإلا
	ویلا_diff = k_چکر_get_32() - ویلا_شروع;
#جے_انت /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */

	z_paging_histogram_inc(&z_paging_histogram_backing_store_ورقہ_وچ,
			       ویلا_diff);
#جے_انت /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
}

ساكن مضمن فراغ do_backing_store_ورقہ_باہرلا(uintptr_t location)
{
#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	uint32_t ویلا_diff;

#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	timing_t ویلا_شروع, ویلا_سرا;

	ویلا_شروع = timing_counter_get();
#وإلا
	uint32_t ویلا_شروع;

	ویلا_شروع = k_چکر_get_32();
#جے_انت /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#جے_انت /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	k_mem_paging_backing_store_ورقہ_باہرلا(location);

#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	ویلا_سرا = timing_counter_get();
	ویلا_diff = (uint32_t)timing_چکراں_get(&ویلا_شروع, &ویلا_سرا);
#وإلا
	ویلا_diff = k_چکر_get_32() - ویلا_شروع;
#جے_انت /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */

	z_paging_histogram_inc(&z_paging_histogram_backing_store_ورقہ_باہرلا,
			       ویلا_diff);
#جے_انت /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
}

#إذا defined(CONFIG_SMP) && defined(CONFIG_DEMAND_PAGING_ALLOW_IRQ)
/*
 * SMP support is very simple. Some resources such as the scratch page could
 * be made per CPU, backing store driver execution be confined to the faulting
 * CPU, statistics be made to cope with access concurrency, etc. But in the
 * end we're dealing with memory transfer to/from some external storage which
 * is inherently slow and whose access is most likely serialized anyway.
 * So let's simply enforce global demand paging serialization across all CPUs
 * with a mutex as there is no real gain from added parallelism here.
 */
ساكن K_MUTEX_DEFINE(z_mm_paging_جندرا);
#جے_انت

ساكن فراغ virt_region_foreach(فراغ *addr, size_t ناپ,
				فراغ (*func)(فراغ *vaddr))
{
	k_mem_assert_virtual_region(addr, ناپ);

	لكل (size_t وتھ = 0; وتھ < ناپ; وتھ += CONFIG_MMU_ورقہ_ناپ) {
		func((uint8_t *)addr + وتھ);
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
ساكن صحيح ورقہ_چوکھٹا_prepare_locked(بنية k_mem_ورقہ_چوکھٹا *pf, bool *dirty_ptr,
				     bool ورقہ_fault, uintptr_t *location_ptr)
{
	uintptr_t phys;
	صحيح ret;
	bool dirty = *dirty_ptr;

	phys = k_mem_ورقہ_چوکھٹا_منزل_phys(pf);
	__ASSERT(!k_mem_ورقہ_چوکھٹا_is_pinned(pf), "page frame 0x%lx is pinned",
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
	إذا (k_mem_ورقہ_چوکھٹا_is_mapped(pf)) {
		dirty = dirty || !k_mem_ورقہ_چوکھٹا_is_backed(pf);
	}

	إذا (dirty || ورقہ_fault) {
		arch_mem_scratch(phys);
	}

	إذا (k_mem_ورقہ_چوکھٹا_is_mapped(pf)) {
		ret = k_mem_paging_backing_store_location_get(pf, location_ptr,
							      ورقہ_fault);
		إذا (ret != 0) {
			LOG_ERR("out of backing store memory");
			أرجع -ENOMEM;
		}
		arch_mem_ورقہ_باہرلا(k_mem_ورقہ_چوکھٹا_منزل_virt(pf), *location_ptr);

		إذا (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
			k_mem_paging_eviction_remove(pf);
		}
	} وإلا {
		/* Shouldn't happen unless this function is mis-used */
		__ASSERT(!dirty, "un-mapped page determined to be dirty");
	}
#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	/* Mark as busy so that k_mem_page_frame_is_evictable() returns false */
	__ASSERT(!k_mem_ورقہ_چوکھٹا_is_busy(pf), "page frame 0x%lx is already busy",
		 phys);
	k_mem_ورقہ_چوکھٹا_سیٹ(pf, K_MEM_ورقہ_چوکھٹا_BUSY);
#جے_انت /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	/* Update dirty parameter, since we set to true if it wasn't backed
	 * even if otherwise clean
	 */
	*dirty_ptr = dirty;

	أرجع 0;
}

ساكن صحيح do_mem_evict(فراغ *addr)
{
	bool dirty;
	بنية k_mem_ورقہ_چوکھٹا *pf;
	uintptr_t location;
	k_spinlock_کلید_t کلید;
	uintptr_t flags, phys;
	صحيح ret;

#إذا CONFIG_DEMAND_PAGING_ALLOW_IRQ
	__ASSERT(!k_is_وچ_isr(),
		 "%s is unavailable in ISRs with CONFIG_DEMAND_PAGING_ALLOW_IRQ",
		 __func__);
#جے_تعریف_شدہ CONFIG_SMP
	k_mutex_جندرا(&z_mm_paging_جندرا, K_FOREVER);
#وإلا
	k_sched_جندرا();
#جے_انت
#جے_انت /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	کلید = k_spin_جندرا(&z_mm_جندرا);
	flags = arch_ورقہ_info_get(addr, &phys, false);
	__ASSERT((flags & ARCH_انکڑے_ورقہ_NOT_MAPPED) == 0,
		 "address %p isn't mapped", addr);
	إذا ((flags & ARCH_انکڑے_ورقہ_LOADED) == 0) {
		/* Un-mapped or already evicted. Nothing to do */
		ret = 0;
		اذهب باہرلا;
	}

	dirty = (flags & ARCH_انکڑے_ورقہ_DIRTY) != 0;
	pf = k_mem_phys_منزل_ورقہ_چوکھٹا(phys);
	__ASSERT(k_mem_ورقہ_چوکھٹا_منزل_virt(pf) == addr, "page frame address mismatch");
	ret = ورقہ_چوکھٹا_prepare_locked(pf, &dirty, false, &location);
	إذا (ret != 0) {
		اذهب باہرلا;
	}

#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	k_spin_unlock(&z_mm_جندرا, کلید);
#جے_انت /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	إذا (dirty) {
		do_backing_store_ورقہ_باہرلا(location);
	}
#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	کلید = k_spin_جندرا(&z_mm_جندرا);
#جے_انت /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	ورقہ_چوکھٹا_free_locked(pf);
باہرلا:
	k_spin_unlock(&z_mm_جندرا, کلید);
#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_ALLOW_IRQ
#جے_تعریف_شدہ CONFIG_SMP
	k_mutex_unlock(&z_mm_paging_جندرا);
#وإلا
	k_sched_unlock();
#جے_انت
#جے_انت /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	أرجع ret;
}

صحيح k_mem_ورقہ_باہرلا(فراغ *addr, size_t ناپ)
{
	__ASSERT(ورقہ_چوکھٹے_initialized, "%s called on %p too early", __func__,
		 addr);
	k_mem_assert_virtual_region(addr, ناپ);

	لكل (size_t وتھ = 0; وتھ < ناپ; وتھ += CONFIG_MMU_ورقہ_ناپ) {
		فراغ *pos = (uint8_t *)addr + وتھ;
		صحيح ret;

		ret = do_mem_evict(pos);
		إذا (ret != 0) {
			أرجع ret;
		}
	}

	أرجع 0;
}

صحيح k_mem_ورقہ_چوکھٹا_evict(uintptr_t phys)
{
	k_spinlock_کلید_t کلید;
	بنية k_mem_ورقہ_چوکھٹا *pf;
	bool dirty;
	uintptr_t flags;
	uintptr_t location;
	صحيح ret;

	__ASSERT(ورقہ_چوکھٹے_initialized, "%s called on 0x%lx too early",
		 __func__, phys);

	/* Implementation is similar to do_page_fault() except there is no
	 * data page to page-in, see comments in that function.
	 */

#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	__ASSERT(!k_is_وچ_isr(),
		 "%s is unavailable in ISRs with CONFIG_DEMAND_PAGING_ALLOW_IRQ",
		 __func__);
#جے_تعریف_شدہ CONFIG_SMP
	k_mutex_جندرا(&z_mm_paging_جندرا, K_FOREVER);
#وإلا
	k_sched_جندرا();
#جے_انت
#جے_انت /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	کلید = k_spin_جندرا(&z_mm_جندرا);
	pf = k_mem_phys_منزل_ورقہ_چوکھٹا(phys);
	إذا (!k_mem_ورقہ_چوکھٹا_is_mapped(pf)) {
		/* Nothing to do, free page */
		ret = 0;
		اذهب باہرلا;
	}
	flags = arch_ورقہ_info_get(k_mem_ورقہ_چوکھٹا_منزل_virt(pf), عدم, false);
	/* Shouldn't ever happen */
	__ASSERT((flags & ARCH_انکڑے_ورقہ_LOADED) != 0, "data page not loaded");
	dirty = (flags & ARCH_انکڑے_ورقہ_DIRTY) != 0;
	ret = ورقہ_چوکھٹا_prepare_locked(pf, &dirty, false, &location);
	إذا (ret != 0) {
		اذهب باہرلا;
	}

#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	k_spin_unlock(&z_mm_جندرا, کلید);
#جے_انت /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	إذا (dirty) {
		do_backing_store_ورقہ_باہرلا(location);
	}
#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	کلید = k_spin_جندرا(&z_mm_جندرا);
#جے_انت /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	ورقہ_چوکھٹا_free_locked(pf);
باہرلا:
	k_spin_unlock(&z_mm_جندرا, کلید);
#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_ALLOW_IRQ
#جے_تعریف_شدہ CONFIG_SMP
	k_mutex_unlock(&z_mm_paging_جندرا);
#وإلا
	k_sched_unlock();
#جے_انت
#جے_انت /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	أرجع ret;
}

ساكن مضمن فراغ paging_stats_نقص_inc(بنية k_thread *faulting_thread,
					   صحيح کلید)
{
#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_STATS
	bool is_irq_unlocked = arch_irq_unlocked(کلید);

	paging_stats.pagefaults.cnt++;

	إذا (is_irq_unlocked) {
		paging_stats.pagefaults.irq_unlocked++;
	} وإلا {
		paging_stats.pagefaults.irq_locked++;
	}

#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_THREAD_STATS
	faulting_thread->paging_stats.pagefaults.cnt++;

	إذا (is_irq_unlocked) {
		faulting_thread->paging_stats.pagefaults.irq_unlocked++;
	} وإلا {
		faulting_thread->paging_stats.pagefaults.irq_locked++;
	}
#وإلا
	ARG_بن_ورتیا(faulting_thread);
#جے_انت /* CONFIG_DEMAND_PAGING_THREAD_STATS */

#جے_بنا_تعریف CONFIG_DEMAND_PAGING_ALLOW_IRQ
	إذا (k_is_وچ_isr()) {
		paging_stats.pagefaults.وچ_isr++;

#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_THREAD_STATS
		faulting_thread->paging_stats.pagefaults.وچ_isr++;
#جے_انت /* CONFIG_DEMAND_PAGING_THREAD_STATS */
	}
#جے_انت /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
#جے_انت /* CONFIG_DEMAND_PAGING_STATS */
}

ساكن مضمن فراغ paging_stats_eviction_inc(بنية k_thread *faulting_thread,
					     bool dirty)
{
#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_STATS
	إذا (dirty) {
		paging_stats.eviction.dirty++;
	} وإلا {
		paging_stats.eviction.clean++;
	}
#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_THREAD_STATS
	إذا (dirty) {
		faulting_thread->paging_stats.eviction.dirty++;
	} وإلا {
		faulting_thread->paging_stats.eviction.clean++;
	}
#وإلا
	ARG_بن_ورتیا(faulting_thread);
#جے_انت /* CONFIG_DEMAND_PAGING_THREAD_STATS */
#جے_انت /* CONFIG_DEMAND_PAGING_STATS */
}

ساكن مضمن بنية k_mem_ورقہ_چوکھٹا *do_eviction_select(bool *dirty)
{
	بنية k_mem_ورقہ_چوکھٹا *pf;

#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	uint32_t ویلا_diff;

#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	timing_t ویلا_شروع, ویلا_سرا;

	ویلا_شروع = timing_counter_get();
#وإلا
	uint32_t ویلا_شروع;

	ویلا_شروع = k_چکر_get_32();
#جے_انت /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#جے_انت /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	pf = k_mem_paging_eviction_select(dirty);

#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	ویلا_سرا = timing_counter_get();
	ویلا_diff = (uint32_t)timing_چکراں_get(&ویلا_شروع, &ویلا_سرا);
#وإلا
	ویلا_diff = k_چکر_get_32() - ویلا_شروع;
#جے_انت /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */

	z_paging_histogram_inc(&z_paging_histogram_eviction, ویلا_diff);
#جے_انت /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	أرجع pf;
}

ساكن bool do_ورقہ_fault(فراغ *addr, bool pin)
{
	بنية k_mem_ورقہ_چوکھٹا *pf;
	k_spinlock_کلید_t کلید;
	uintptr_t ورقہ_وچ_location, ورقہ_باہرلا_location;
	تعداد arch_ورقہ_location حالت;
	bool نتیجہ;
	bool dirty = false;
	بنية k_thread *faulting_thread;
	صحيح ret;

	__ASSERT(ورقہ_چوکھٹے_initialized, "page fault at %p happened too early",
		 addr);

	LOG_DBG("page fault at %p", addr);

	/*
	 * TODO: Add performance accounting:
	 * - k_mem_paging_eviction_select() metrics
	 *   * periodic timer execution time histogram (if implemented)
	 */

#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_ALLOW_IRQ
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
	__ASSERT(!k_is_وچ_isr(), "ISR page faults are forbidden");
#جے_تعریف_شدہ CONFIG_SMP
	k_mutex_جندرا(&z_mm_paging_جندرا, K_FOREVER);
#وإلا
	k_sched_جندرا();
#جے_انت
#جے_انت /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */

	کلید = k_spin_جندرا(&z_mm_جندرا);
	faulting_thread = _current;

	حالت = arch_ورقہ_location_get(addr, &ورقہ_وچ_location);
	إذا (حالت == ARCH_ورقہ_LOCATION_BAD) {
		/* Return false to treat as a fatal error */
		نتیجہ = false;
		اذهب باہرلا;
	}
	نتیجہ = true;

	إذا (حالت == ARCH_ورقہ_LOCATION_PAGED_وچ) {
		إذا (pin) {
			/* It's a physical memory address */
			uintptr_t phys = ورقہ_وچ_location;

			pf = k_mem_phys_منزل_ورقہ_چوکھٹا(phys);
			إذا (!k_mem_ورقہ_چوکھٹا_is_pinned(pf)) {
				إذا (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
					k_mem_paging_eviction_remove(pf);
				}
				k_mem_ورقہ_چوکھٹا_سیٹ(pf, K_MEM_ورقہ_چوکھٹا_PINNED);
			}
		}

		/* This if-block is to pin the page if it is
		 * already present in physical memory. There is
		 * no need to go through the following code to
		 * pull in the data pages. So skip to the end.
		 */
		اذهب باہرلا;
	}
	__ASSERT(حالت == ARCH_ورقہ_LOCATION_PAGED_باہرلا,
		 "unexpected status value %d", حالت);

	paging_stats_نقص_inc(faulting_thread, کلید.کلید);

	pf = free_ورقہ_چوکھٹا_list_get();
	إذا (pf == عدم) {
		/* Need to evict a page frame */
		pf = do_eviction_select(&dirty);
		إذا (pf == عدم) {
			/* Every evictable page frame is pinned or busy.
			 * Fail the fault so it is reported as fatal.
			 */
			نتیجہ = false;
			اذهب باہرلا;
		}
		LOG_DBG("evicting %p at 0x%lx",
			k_mem_ورقہ_چوکھٹا_منزل_virt(pf),
			k_mem_ورقہ_چوکھٹا_منزل_phys(pf));

		paging_stats_eviction_inc(faulting_thread, dirty);
	}
	ret = ورقہ_چوکھٹا_prepare_locked(pf, &dirty, true, &ورقہ_باہرلا_location);
	إذا (ret != 0) {
		/* Backing store is full. Fail the fault so it is
		 * reported as fatal.
		 */
		نتیجہ = false;
		اذهب باہرلا;
	}

#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	k_spin_unlock(&z_mm_جندرا, کلید);
	/* Interrupts are now unlocked if they were not locked when we entered
	 * this function, and we may service ISRs. The scheduler is still
	 * locked.
	 */
#جے_انت /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	إذا (dirty) {
		do_backing_store_ورقہ_باہرلا(ورقہ_باہرلا_location);
	}
	do_backing_store_ورقہ_وچ(ورقہ_وچ_location);

#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	کلید = k_spin_جندرا(&z_mm_جندرا);
	k_mem_ورقہ_چوکھٹا_clear(pf, K_MEM_ورقہ_چوکھٹا_BUSY);
#جے_انت /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	k_mem_ورقہ_چوکھٹا_clear(pf, K_MEM_ورقہ_چوکھٹا_MAPPED);
	چوکھٹا_mapped_سیٹ(pf, addr);
	إذا (pin) {
		k_mem_ورقہ_چوکھٹا_سیٹ(pf, K_MEM_ورقہ_چوکھٹا_PINNED);
	}

	arch_mem_ورقہ_وچ(addr, k_mem_ورقہ_چوکھٹا_منزل_phys(pf));
	k_mem_paging_backing_store_ورقہ_finalize(pf, ورقہ_وچ_location);
	إذا (IS_ENABLED(CONFIG_EVICTION_TRACKING) && (!pin)) {
		k_mem_paging_eviction_جوڑو(pf);
	}
باہرلا:
	k_spin_unlock(&z_mm_جندرا, کلید);
#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_ALLOW_IRQ
#جے_تعریف_شدہ CONFIG_SMP
	k_mutex_unlock(&z_mm_paging_جندرا);
#وإلا
	k_sched_unlock();
#جے_انت
#جے_انت /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */

	أرجع نتیجہ;
}

ساكن فراغ do_ورقہ_وچ(فراغ *addr)
{
	bool ret;

	ret = do_ورقہ_fault(addr, false);
	إذا (!ret) {
		/* There is no error reporting channel to the caller, and
		 * continuing would defer the failure to an arbitrary later
		 * access.
		 */
		LOG_ERR("cannot page in address %p", addr);
		k_panic();
	}
}

فراغ k_mem_ورقہ_وچ(فراغ *addr, size_t ناپ)
{
	__ASSERT(!IS_ENABLED(CONFIG_DEMAND_PAGING_ALLOW_IRQ) || !k_is_وچ_isr(),
		 "%s may not be called in ISRs if CONFIG_DEMAND_PAGING_ALLOW_IRQ is enabled",
		 __func__);
	virt_region_foreach(addr, ناپ, do_ورقہ_وچ);
}

ساكن فراغ do_mem_pin(فراغ *addr)
{
	bool ret;

	ret = do_ورقہ_fault(addr, true);
	إذا (!ret) {
		/* There is no error reporting channel to the caller, and
		 * continuing with the memory not actually pinned would
		 * defer the failure to an arbitrary later access, possibly
		 * from a context that cannot handle a page fault.
		 */
		LOG_ERR("cannot pin address %p", addr);
		k_panic();
	}
}

فراغ k_mem_pin(فراغ *addr, size_t ناپ)
{
	__ASSERT(!IS_ENABLED(CONFIG_DEMAND_PAGING_ALLOW_IRQ) || !k_is_وچ_isr(),
		 "%s may not be called in ISRs if CONFIG_DEMAND_PAGING_ALLOW_IRQ is enabled",
		 __func__);
	virt_region_foreach(addr, ناپ, do_mem_pin);
}

bool k_mem_ورقہ_fault(فراغ *addr)
{
	أرجع do_ورقہ_fault(addr, false);
}

ساكن فراغ do_mem_unpin(فراغ *addr)
{
	بنية k_mem_ورقہ_چوکھٹا *pf;
	k_spinlock_کلید_t کلید;
	uintptr_t flags, phys;

	کلید = k_spin_جندرا(&z_mm_جندرا);
	flags = arch_ورقہ_info_get(addr, &phys, false);
	__ASSERT((flags & ARCH_انکڑے_ورقہ_NOT_MAPPED) == 0,
		 "invalid data page at %p", addr);
	إذا ((flags & ARCH_انکڑے_ورقہ_LOADED) != 0) {
		pf = k_mem_phys_منزل_ورقہ_چوکھٹا(phys);
		إذا (k_mem_ورقہ_چوکھٹا_is_pinned(pf)) {
			k_mem_ورقہ_چوکھٹا_clear(pf, K_MEM_ورقہ_چوکھٹا_PINNED);

			إذا (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
				k_mem_paging_eviction_جوڑو(pf);
			}
		}
	}
	k_spin_unlock(&z_mm_جندرا, کلید);
}

فراغ k_mem_unpin(فراغ *addr, size_t ناپ)
{
	__ASSERT(ورقہ_چوکھٹے_initialized, "%s called on %p too early", __func__,
		 addr);
	virt_region_foreach(addr, ناپ, do_mem_unpin);
}

#جے_انت /* CONFIG_DEMAND_PAGING */
