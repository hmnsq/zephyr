/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Routines for managing virtual address spaces
 */

#અન્તર્ભાવ <stdint.h>
#અન્તર્ભાવ <kernel_arch_interface.h>
#અન્તર્ભાવ <zephyr/spinlock.h>
#અન્તર્ભાવ <mmu.h>
#અન્તર્ભાવ <zephyr/init.h>
#અન્તર્ભાવ <kernel_internal.h>
#અન્તર્ભાવ <zephyr/internal/syscall_handler.h>
#અન્તર્ભાવ <zephyr/toolchain.h>
#અન્તર્ભાવ <zephyr/linker/linker-defs.h>
#અન્તર્ભાવ <zephyr/sys/bitarray.h>
#અન્તર્ભાવ <zephyr/sys/check.h>
#અન્તર્ભાવ <zephyr/sys/math_extras.h>
#અન્તર્ભાવ <zephyr/sys/minmax.h>
#અન્તર્ભાવ <zephyr/timing/timing.h>
#અન્તર્ભાવ <zephyr/arch/common/init.h>
#અન્તર્ભાવ <zephyr/logging/log.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#ifdef CONFIG_DEMAND_PAGING
#અન્તર્ભાવ <zephyr/kernel/mm/demand_paging.h>
#endif /* CONFIG_DEMAND_PAGING */

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
સંરચના k_spinlock z_mm_lock;

/*
 * General page frame management
 */

/* Database of all RAM page frames */
સંરચના k_mem_page_frame k_mem_page_frames[K_MEM_અંક_PAGE_FRAMES];

#ifdef CONFIG_ASSERT
/* Indicator that k_mem_page_frames has been initialized, many of these APIs do
 * not work before POST_KERNEL
 */
સ્થિર bool page_frames_initialized;
#endif /* CONFIG_ASSERT */

/* Add colors to page table dumps to indicate mapping type */
#define COLOR_PAGE_FRAMES	1

#જો COLOR_PAGE_FRAMES
#define ANSI_DEFAULT "\x1B" "[0m"
#define ANSI_RED     "\x1B" "[1;31m"
#define ANSI_GREEN   "\x1B" "[1;32m"
#define ANSI_YELLOW  "\x1B" "[1;33m"
#define ANSI_BLUE    "\x1B" "[1;34m"
#define ANSI_MAGENTA "\x1B" "[1;35m"
#define ANSI_CYAN    "\x1B" "[1;36m"
#define ANSI_GREY    "\x1B" "[1;90m"

#define COLOR(x)	printk(_CONCAT(ANSI_, x))
#નહીંતર
#define COLOR(x)	કરો { } જ્યારે (false)
#endif /* COLOR_PAGE_FRAMES */

/* LCOV_EXCL_START */
સ્થિર શૂન્ય page_frame_dump(સંરચના k_mem_page_frame *pf)
{
	જો (k_mem_page_frame_is_free(pf)) {
		COLOR(GREY);
		printk("-");
	} નહીંતર જો (k_mem_page_frame_is_reserved(pf)) {
		COLOR(CYAN);
		printk("R");
	} નહીંતર જો (k_mem_page_frame_is_busy(pf)) {
		COLOR(MAGENTA);
		printk("B");
	} નહીંતર જો (k_mem_page_frame_is_pinned(pf)) {
		COLOR(YELLOW);
		printk("P");
	} નહીંતર જો (k_mem_page_frame_is_available(pf)) {
		COLOR(GREY);
		printk(".");
	} નહીંતર જો (k_mem_page_frame_is_mapped(pf)) {
		COLOR(DEFAULT);
		printk("M");
	} નહીંતર {
		COLOR(RED);
		printk("?");
	}
}

શૂન્ય k_mem_page_frames_dump(શૂન્ય)
{
	પૂર્ણાંક column = 0;

	__ASSERT(page_frames_initialized, "%s called too early", __func__);
	printk("Physical memory from 0x%lx to 0x%lx\n",
	       K_MEM_PHYS_RAM_START, K_MEM_PHYS_RAM_END);

	માટે (પૂર્ણાંક i = 0; i < K_MEM_અંક_PAGE_FRAMES; i++) {
		સંરચના k_mem_page_frame *pf = &k_mem_page_frames[i];

		page_frame_dump(pf);

		column++;
		જો (column == 64) {
			column = 0;
			printk("\n");
		}
	}

	COLOR(DEFAULT);
	જો (column != 0) {
		printk("\n");
	}
}
/* LCOV_EXCL_STOP */

#define VIRT_FOREACH(_base, _size, _pos) \
	માટે ((_pos) = (_base); \
	     (_pos) < ((uint8_t *)(_base) + (_size)); (_pos) += CONFIG_MMU_PAGE_SIZE)

#define PHYS_FOREACH(_base, _size, _pos) \
	માટે ((_pos) = (_base); \
	     (_pos) < ((uintptr_t)(_base) + (_size)); (_pos) += CONFIG_MMU_PAGE_SIZE)


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
			   CONFIG_KERNEL_VM_SIZE / CONFIG_MMU_PAGE_SIZE);

સ્થિર bool virt_region_inited;

#define Z_VIRT_REGION_START_ADDR	K_MEM_VM_FREE_START
#define Z_VIRT_REGION_END_ADDR		(K_MEM_VIRT_RAM_END - K_MEM_VM_RESERVED)

સ્થિર અંતઃરેખ uintptr_t virt_from_bitmap_offset(size_t offset, size_t size)
{
	પાછા POINTER_TO_UINT(K_MEM_VIRT_RAM_END)
	       - (offset * CONFIG_MMU_PAGE_SIZE) - size;
}

સ્થિર અંતઃરેખ size_t virt_to_bitmap_offset(શૂન્ય *vaddr, size_t size)
{
	પાછા (POINTER_TO_UINT(K_MEM_VIRT_RAM_END)
		- POINTER_TO_UINT(vaddr) - size) / CONFIG_MMU_PAGE_SIZE;
}

સ્થિર શૂન્ય virt_region_init(શૂન્ય)
{
	size_t offset, અંક_bits;

	/* There are regions where we should never map via
	 * k_mem_map() and k_mem_map_phys_bare(). Mark them as
	 * already allocated so they will never be used.
	 */

	જો (K_MEM_VM_RESERVED > 0) {
		/* Mark reserved region at end of virtual address space */
		અંક_bits = K_MEM_VM_RESERVED / CONFIG_MMU_PAGE_SIZE;
		(શૂન્ય)sys_bitarray_set_region(&virt_region_bitmap,
					      અંક_bits, 0);
	}

	/* Mark all bits up to Z_FREE_VM_START as allocated */
	અંક_bits = POINTER_TO_UINT(K_MEM_VM_FREE_START)
		   - POINTER_TO_UINT(K_MEM_VIRT_RAM_START);
	offset = virt_to_bitmap_offset(K_MEM_VIRT_RAM_START, અંક_bits);
	અંક_bits /= CONFIG_MMU_PAGE_SIZE;
	(શૂન્ય)sys_bitarray_set_region(&virt_region_bitmap,
				      અંક_bits, offset);

	virt_region_inited = true;
}

સ્થિર શૂન્ય virt_region_free(શૂન્ય *vaddr, size_t size)
{
	size_t offset, અંક_bits;
	uint8_t *vaddr_u8 = (uint8_t *)vaddr;

	જો (unlikely(!virt_region_inited)) {
		virt_region_init();
	}

#ifndef CONFIG_KERNEL_DIRECT_MAP
	/* Without the need to support K_MEM_DIRECT_MAP, the region must be
	 * able to be represented in the bitmap. So this case is
	 * simple.
	 */

	__ASSERT((vaddr_u8 >= Z_VIRT_REGION_START_ADDR)
		 && ((vaddr_u8 + size - 1) < Z_VIRT_REGION_END_ADDR),
		 "invalid virtual address region %p (%zu)", vaddr_u8, size);
	જો (!((vaddr_u8 >= Z_VIRT_REGION_START_ADDR)
	      && ((vaddr_u8 + size - 1) < Z_VIRT_REGION_END_ADDR))) {
		પાછા;
	}

	offset = virt_to_bitmap_offset(vaddr, size);
	અંક_bits = size / CONFIG_MMU_PAGE_SIZE;
	(શૂન્ય)sys_bitarray_free(&virt_region_bitmap, અંક_bits, offset);
#નહીંતર /* !CONFIG_KERNEL_DIRECT_MAP */
	/* With K_MEM_DIRECT_MAP, the region can be outside of the virtual
	 * memory space, wholly within it, or overlap partially.
	 * So additional processing is needed to make sure we only
	 * mark the pages within the bitmap.
	 */
	જો (((vaddr_u8 >= Z_VIRT_REGION_START_ADDR) &&
	     (vaddr_u8 < Z_VIRT_REGION_END_ADDR)) ||
	    (((vaddr_u8 + size - 1) >= Z_VIRT_REGION_START_ADDR) &&
	     ((vaddr_u8 + size - 1) < Z_VIRT_REGION_END_ADDR))) {
		uint8_t *adjusted_start = max(vaddr_u8, Z_VIRT_REGION_START_ADDR);
		uint8_t *adjusted_end = min(vaddr_u8 + size,
					    Z_VIRT_REGION_END_ADDR);
		size_t adjusted_sz = adjusted_end - adjusted_start;

		offset = virt_to_bitmap_offset(adjusted_start, adjusted_sz);
		અંક_bits = adjusted_sz / CONFIG_MMU_PAGE_SIZE;
		(શૂન્ય)sys_bitarray_free(&virt_region_bitmap, અંક_bits, offset);
	}
#endif /* !CONFIG_KERNEL_DIRECT_MAP */
}

સ્થિર શૂન્ય *virt_region_alloc(size_t size, size_t align)
{
	uintptr_t dest_addr;
	size_t alloc_size;
	size_t offset;
	size_t અંક_bits;
	પૂર્ણાંક ret;

	જો (unlikely(!virt_region_inited)) {
		virt_region_init();
	}

	/* Possibly request more pages to ensure we can get an aligned virtual address */
	અંક_bits = (size + align - CONFIG_MMU_PAGE_SIZE) / CONFIG_MMU_PAGE_SIZE;
	alloc_size = અંક_bits * CONFIG_MMU_PAGE_SIZE;
	ret = sys_bitarray_alloc(&virt_region_bitmap, અંક_bits, &offset);
	જો (ret != 0) {
		LOG_ERR("insufficient virtual address space (requested %zu)",
			size);
		પાછા ખાલી;
	}

	/* Remember that bit #0 in bitmap corresponds to the highest
	 * virtual address. So here we need to go downwards (backwards?)
	 * to get the starting address of the allocated region.
	 */
	dest_addr = virt_from_bitmap_offset(offset, alloc_size);

	જો (alloc_size > size) {
		uintptr_t aligned_dest_addr = ROUND_UP(dest_addr, align);

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
		virt_region_free(UINT_TO_POINTER(dest_addr),
				 aligned_dest_addr - dest_addr);
		જો (((dest_addr + alloc_size) - (aligned_dest_addr + size)) > 0) {
			virt_region_free(UINT_TO_POINTER(aligned_dest_addr + size),
					 (dest_addr + alloc_size) - (aligned_dest_addr + size));
		}

		dest_addr = aligned_dest_addr;
	}

	/* Need to make sure this does not step into kernel memory */
	જો (dest_addr < POINTER_TO_UINT(Z_VIRT_REGION_START_ADDR)) {
		(શૂન્ય)sys_bitarray_free(&virt_region_bitmap, અંક_bits, offset);
		પાછા ખાલી;
	}

	પાછા UINT_TO_POINTER(dest_addr);
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
BUILD_ASSERT(SYS_SFLIST_FLAG_BITS >= 1, "MMU free-page list needs one sflist flag bit");

સ્થિર sys_sflist_t free_page_frame_list;

/* Number of unused and available free page frames.
 * This information may go stale immediately.
 */
સ્થિર size_t z_free_page_ગણતરી;

#define PF_ASSERT(pf, પદાવલિ, fmt, ...) \
	__ASSERT(પદાવલિ, "page frame 0x%lx: " fmt, k_mem_page_frame_to_phys(pf), \
		 ##__VA_ARGS__)

/* Get an unused page frame. don't care which one, or NULL if there are none */
સ્થિર સંરચના k_mem_page_frame *free_page_frame_list_get(શૂન્ય)
{
	sys_sfnode_t *ગાંઠ;
	સંરચના k_mem_page_frame *pf = ખાલી;

	ગાંઠ = sys_sflist_get(&free_page_frame_list);
	જો (ગાંઠ != ખાલી) {
		z_free_page_ગણતરી--;
		pf = CONTAINER_OF(ગાંઠ, સંરચના k_mem_page_frame, ગાંઠ);
		PF_ASSERT(pf, k_mem_page_frame_is_free(pf),
			 "on free list but not free");
		pf->va_and_flags = 0;
	}

	પાછા pf;
}

/* Release a page frame back into the list of free pages */
સ્થિર શૂન્ય free_page_frame_list_put(સંરચના k_mem_page_frame *pf)
{
	PF_ASSERT(pf, k_mem_page_frame_is_available(pf),
		 "unavailable page put on free list");

	sys_sfnode_init(&pf->ગાંઠ, K_MEM_PAGE_FRAME_FREE);
	sys_sflist_append(&free_page_frame_list, &pf->ગાંઠ);
	z_free_page_ગણતરી++;
}

સ્થિર શૂન્ય free_page_frame_list_init(શૂન્ય)
{
	sys_sflist_init(&free_page_frame_list);
}

સ્થિર શૂન્ય page_frame_free_locked(સંરચના k_mem_page_frame *pf)
{
	pf->va_and_flags = 0;
	free_page_frame_list_put(pf);
}

/*
 * Memory Mapping
 */

/* Called after the frame is mapped in the arch layer, to update our
 * local ontology (and do some assertions while we're at it)
 */
સ્થિર શૂન્ય frame_mapped_set(સંરચના k_mem_page_frame *pf, શૂન્ય *addr)
{
	PF_ASSERT(pf, !k_mem_page_frame_is_free(pf),
		  "attempted to map a page frame on the free list");
	PF_ASSERT(pf, !k_mem_page_frame_is_reserved(pf),
		  "attempted to map a reserved page frame");

	/* We do allow multiple mappings for pinned page frames
	 * since we will never need to reverse map them.
	 * This is uncommon, use-cases are for things like the
	 * Zephyr equivalent of VSDOs
	 */
	PF_ASSERT(pf, !k_mem_page_frame_is_mapped(pf) || k_mem_page_frame_is_pinned(pf),
		 "non-pinned and already mapped to %p",
		 k_mem_page_frame_to_virt(pf));

	uintptr_t flags_mask = CONFIG_MMU_PAGE_SIZE - 1;
	uintptr_t va = (uintptr_t)addr & ~flags_mask;

	pf->va_and_flags &= flags_mask;
	pf->va_and_flags |= va | K_MEM_PAGE_FRAME_MAPPED;
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
સ્થિર પૂર્ણાંક virt_to_page_frame(શૂન્ય *virt, uintptr_t *phys)
{
	uintptr_t paddr;
	સંરચના k_mem_page_frame *pf;
	પૂર્ણાંક ret = -EFAULT;

	K_MEM_PAGE_FRAME_FOREACH(paddr, pf) {
		જો (k_mem_page_frame_is_mapped(pf)) {
			જો (virt == k_mem_page_frame_to_virt(pf)) {
				ret = 0;
				જો (phys != ખાલી) {
					*phys = k_mem_page_frame_to_phys(pf);
				}
				તોડો;
			}
		}
	}

	પાછા ret;
}
/* LCOV_EXCL_STOP */

__weak FUNC_ALIAS(virt_to_page_frame, arch_page_phys_get, પૂર્ણાંક);

#ifdef CONFIG_DEMAND_PAGING
સ્થિર પૂર્ણાંક page_frame_prepare_locked(સંરચના k_mem_page_frame *pf, bool *dirty_ptr,
				     bool page_in, uintptr_t *location_ptr);

સ્થિર અંતઃરેખ શૂન્ય do_backing_store_page_in(uintptr_t location);
સ્થિર અંતઃરેખ શૂન્ય do_backing_store_page_out(uintptr_t location);
#endif /* CONFIG_DEMAND_PAGING */

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
સ્થિર પૂર્ણાંક map_anon_page(શૂન્ય *addr, uint32_t flags, bool *recovered)
{
	સંરચના k_mem_page_frame *pf;
	uintptr_t phys;
	bool lock = (flags & K_MEM_MAP_LOCK) != 0U;
	પૂર્ણાંક ret;

	*recovered = true;

	pf = free_page_frame_list_get();
	જો (pf == ખાલી) {
#ifdef CONFIG_DEMAND_PAGING
		uintptr_t location;
		bool dirty;

		pf = k_mem_paging_eviction_select(&dirty);
		જો (pf == ખાલી) {
			/* Every evictable page frame is pinned or busy */
			પાછા -ENOMEM;
		}
		LOG_DBG("evicting %p at 0x%lx",
			k_mem_page_frame_to_virt(pf),
			k_mem_page_frame_to_phys(pf));
		ret = page_frame_prepare_locked(pf, &dirty, false, &location);
		જો (ret != 0) {
			પાછા -ENOMEM;
		}
		જો (dirty) {
			do_backing_store_page_out(location);
		}
		pf->va_and_flags = 0;
#નહીંતર
		પાછા -ENOMEM;
#endif /* CONFIG_DEMAND_PAGING */
	}

	phys = k_mem_page_frame_to_phys(pf);
	ret = arch_mem_map(addr, phys, CONFIG_MMU_PAGE_SIZE, flags);
	__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
	જો (ret != 0) {
#ifdef CONFIG_USERSPACE
		/* The mapping above may have already succeeded in some
		 * memory domains but failing in the middle. So we need to
		 * unmap those first before returning the page frame back
		 * to the list.
		 */
		પૂર્ણાંક unmap_ret = arch_mem_unmap(addr, CONFIG_MMU_PAGE_SIZE);

		__ASSERT(unmap_ret == 0, "%s: cannot undo failed mapping %d\n", __func__,
			 unmap_ret);

		જો (unmap_ret != 0) {
			/* The page may still be mapped in some memory domains,
			 * so the page frame cannot be given back to the free
			 * list for reuse.
			 */
			*recovered = false;
			પાછા ret;
		}
#endif

		/* Mapping failed: give the page frame back before bailing out. */
		page_frame_free_locked(pf);
		પાછા ret;
	}

	જો (lock) {
		k_mem_page_frame_set(pf, K_MEM_PAGE_FRAME_PINNED);
	}
	frame_mapped_set(pf, addr);
#ifdef CONFIG_DEMAND_PAGING
	જો (IS_ENABLED(CONFIG_EVICTION_TRACKING) && (!lock)) {
		k_mem_paging_eviction_add(pf);
	}
#endif

	LOG_DBG("memory mapping anon page %p -> 0x%lx", addr, phys);

	પાછા 0;
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
સ્થિર પૂર્ણાંક unmap_anon_pages(શૂન્ય *virt, size_t sz)
{
	uint8_t *pos;
	uintptr_t phys;
	સંરચના k_mem_page_frame *pf;
	પૂર્ણાંક ret = 0;

	VIRT_FOREACH(virt, sz, pos) {
#ifdef CONFIG_DEMAND_PAGING
		ગણના arch_page_location status;
		uintptr_t location;

		status = arch_page_location_get(pos, &location);
		પસંદગી (status) {
		સ્થિતિ ARCH_PAGE_LOCATION_PAGED_OUT:
			/*
			 * No pf is associated with this mapping.
			 * Simply get rid of the MMU entry and free
			 * corresponding backing store.
			 */
			ret = arch_mem_unmap(pos, CONFIG_MMU_PAGE_SIZE);
			__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__,
				 ret);

			k_mem_paging_backing_store_location_free(location);

			જો (ret != 0) {
				જાઓ unmap_anon_out;
			}

			ચાલુ;
		સ્થિતિ ARCH_PAGE_LOCATION_PAGED_IN:
			/*
			 * The page is in memory but it may not be
			 * accessible in order to manage tracking
			 * of the ARCH_DATA_PAGE_ACCESSED flag
			 * meaning arch_page_phys_get() could fail.
			 * Still, we know the actual phys address.
			 */
			phys = location;
			ret = 0;
			તોડો;
		મૂળભૂત:
			ret = arch_page_phys_get(pos, &phys);
			તોડો;
		}
#નહીંતર
		ret = arch_page_phys_get(pos, &phys);
#endif
		__ASSERT(ret == 0, "%s: cannot unmap an unmapped address %p", __func__, pos);
		જો (ret != 0) {
			/* Found an address not mapped. Do not continue. */
			જાઓ unmap_anon_out;
		}

		__ASSERT(k_mem_is_page_frame(phys), "%s: 0x%lx is not a page frame", __func__,
			 phys);
		જો (!k_mem_is_page_frame(phys)) {
			/* Physical address has no corresponding page frame
			 * description in the page frame array.
			 * This should not happen. Do not continue.
			 */
			જાઓ unmap_anon_out;
		}

		/* Grab the corresponding page frame from physical address */
		pf = k_mem_phys_to_page_frame(phys);

		__ASSERT(k_mem_page_frame_is_mapped(pf), "%s: 0x%lx is not a mapped page frame",
			 __func__, phys);
		જો (!k_mem_page_frame_is_mapped(pf)) {
			/* Page frame is not marked mapped.
			 * This should not happen. Do not continue.
			 */
			જાઓ unmap_anon_out;
		}

		ret = arch_mem_unmap(pos, CONFIG_MMU_PAGE_SIZE);
		__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
		જો (ret != 0) {
			/* Fail to unmap. Must bail without releasing any resources
			 * as the page is still mapped.
			 */
			જાઓ unmap_anon_out;
		}

#ifdef CONFIG_DEMAND_PAGING
		જો (IS_ENABLED(CONFIG_EVICTION_TRACKING) &&
		    (!k_mem_page_frame_is_pinned(pf))) {
			k_mem_paging_eviction_remove(pf);
		}
#endif

		/* Put the page frame back into free list */
		page_frame_free_locked(pf);
	}

unmap_anon_out:
	પાછા ret;
}

#ifdef CONFIG_DEMAND_MAPPING
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
સ્થિર bool undo_demand_mapped_pages(શૂન્ય *virt, size_t sz)
{
	uint8_t *undo_pos;
	bool success = true;
	પૂર્ણાંક ret;

	/* Undo the pages mapped before the failure. */
	VIRT_FOREACH(virt, sz, undo_pos) {
		ret = arch_mem_unmap(undo_pos, CONFIG_MMU_PAGE_SIZE);
		__ASSERT(ret == 0, "%s: cannot undo mapped pages %d", __func__, ret);

		/* Failed to undo one mapped page. Though we still
		 * want to continue unmapping the remaining mapped
		 * pages to reclaim them.
		 */
		જો (ret != 0) {
			success = false;
		}
	}

	પાછા success;
}
#endif

શૂન્ય *k_mem_map_phys_guard(uintptr_t phys, size_t size, uint32_t flags, bool is_anon)
{
	uint8_t *dst;
	uint8_t *region;
	size_t total_size;
	પૂર્ણાંક ret;
	k_spinlock_ચાવી_t ચાવી;
	uint8_t *pos;
	bool uninit = (flags & K_MEM_MAP_UNINIT) != 0U;

	__ASSERT(!is_anon || (is_anon && page_frames_initialized),
		 "%s called too early", __func__);
	__ASSERT((flags & K_MEM_CACHE_MASK) == 0U,
		 "%s does not support explicit cache settings", __func__);

	જો (((flags & K_MEM_PERM_USER) != 0U) &&
	    ((flags & K_MEM_MAP_UNINIT) != 0U)) {
		LOG_ERR("user access to anonymous uninitialized pages is forbidden");
		પાછા ખાલી;
	}
	જો ((size % CONFIG_MMU_PAGE_SIZE) != 0U) {
		LOG_ERR("unaligned size %zu passed to %s", size, __func__);
		પાછા ખાલી;
	}
	જો (size == 0) {
		LOG_ERR("zero sized memory mapping");
		પાછા ખાલી;
	}

	/* Need extra for the guard pages (before and after) which we
	 * won't map.
	 */
	જો (size_add_overflow(size, CONFIG_MMU_PAGE_SIZE * 2, &total_size)) {
		LOG_ERR("too large size %zu passed to %s", size, __func__);
		પાછા ખાલી;
	}

	ચાવી = k_spin_lock(&z_mm_lock);

	region = virt_region_alloc(total_size, CONFIG_MMU_PAGE_SIZE);
	જો (region == ખાલી) {
		/* Address space has no free region */
		dst = ખાલી;
		જાઓ out;
	}

	/* Unmap both guard pages to make sure accessing them
	 * will generate fault.
	 *
	 * Note that a failed unmap may have left the guard page mapped in
	 * some memory domains, so the virtual region cannot be freed for
	 * reuse.
	 */
	ret = arch_mem_unmap(region, CONFIG_MMU_PAGE_SIZE);
	__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
	જો (ret != 0) {
		dst = ખાલી;
		જાઓ out;
	}

	ret = arch_mem_unmap(region + CONFIG_MMU_PAGE_SIZE + size, CONFIG_MMU_PAGE_SIZE);
	__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
	જો (ret != 0) {
		dst = ખાલી;
		જાઓ out;
	}

	/* Skip over the "before" guard page in returned address. */
	dst = region + CONFIG_MMU_PAGE_SIZE;

	જો (is_anon) {
		/* Mapping from anonymous memory */
		flags |= K_MEM_CACHE_WB;
#ifdef CONFIG_DEMAND_MAPPING
		જો ((flags & K_MEM_MAP_LOCK) == 0) {
			flags |= K_MEM_MAP_UNPAGED;
			VIRT_FOREACH(dst, size, pos) {
				ret = arch_mem_map(pos,
						   uninit ? ARCH_UNPAGED_ANON_UNINIT
							  : ARCH_UNPAGED_ANON_ZERO,
						   CONFIG_MMU_PAGE_SIZE, flags);
				__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
				જો (ret != 0) {
					size_t unmap_sz = pos - dst;

					જો (IS_ENABLED(CONFIG_USERSPACE)) {
						/* Note that it is unmapping an extra page due to
						 * the possibility that the mapping above has
						 * already updated some memory domains but fail in
						 * the middle. So we need to include the failed
						 * page to unmap those that have been mapped so
						 * far.
						 */
						unmap_sz += CONFIG_MMU_PAGE_SIZE;
					}

					bool success = undo_demand_mapped_pages(dst, unmap_sz);

					dst = ખાલી;

					જો (success) {
						જાઓ fail;
					} નહીંતર {
						/* Cannot fully recover from mapping
						 * failure. We cannot free the allocated
						 * virtual memory region since some of
						 * the page may still be mapped.
						 */
						જાઓ out;
					}
				}
			}
			LOG_DBG("memory mapping anon pages %p to %p unpaged", dst, pos-1);
			/* skip the memset() below */
			uninit = true;
		} નહીંતર
#endif
		{
			VIRT_FOREACH(dst, size, pos) {
				bool recovered;

				ret = map_anon_page(pos, flags, &recovered);

				જો (ret != 0) {
					પૂર્ણાંક unmap_ret = unmap_anon_pages(dst, pos - dst);

					dst = ખાલી;

					જો ((unmap_ret == 0) && recovered) {
						જાઓ fail;
					} નહીંતર {
						/* Cannot fully recover from mapping
						 * failure. We cannot free the allocated
						 * virtual memory region since some of
						 * the page may still be mapped.
						 */
						જાઓ out;
					}
				}
			}
		}
	} નહીંતર {
		/* Mapping known physical memory. */
		ret = arch_mem_map(dst, phys, size, flags);
		__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
		જો (ret != 0) {
			/* Clean up any partial mapping left behind. */
			ret = arch_mem_unmap(dst, size);
			__ASSERT(ret == 0, "%s: cannot undo mapped pages %d", __func__, ret);

			dst = ખાલી;

			જો (ret != 0) {
				/* Cannot fully undo all the mappings. There might still be pages
				 * mapped. So we cannot free the allocated virtual memory region.
				 */
				જાઓ out;
			} નહીંતર {
				જાઓ fail;
			}
		}
	}

out:
	k_spin_unlock(&z_mm_lock, ચાવી);

	જો (dst != ખાલી && !uninit) {
		/* If we later implement mappings to a copy-on-write
		 * zero page, won't need this step
		 */
		memset(dst, 0, size);
	}

	પાછા dst;

fail:
	/* Release the whole region, including the guard pages. */
	virt_region_free(region, total_size);

	જાઓ out;
}

શૂન્ય k_mem_unmap_phys_guard(શૂન્ય *addr, size_t size, bool is_anon)
{
	uint8_t *pos;
	k_spinlock_ચાવી_t ચાવી;
	size_t total_size;
	પૂર્ણાંક ret;

	/* Need space for the "before" guard page */
	__ASSERT_NO_MSG(POINTER_TO_UINT(addr) >= CONFIG_MMU_PAGE_SIZE);

	/* Make sure address range is still valid after accounting
	 * for two guard pages.
	 */
	pos = (uint8_t *)addr - CONFIG_MMU_PAGE_SIZE;
	k_mem_assert_virtual_region(pos, size + (CONFIG_MMU_PAGE_SIZE * 2));

	ચાવી = k_spin_lock(&z_mm_lock);

	/* Check if both guard pages are unmapped.
	 * Bail if not, as this is probably a region not mapped
	 * using k_mem_map().
	 */
	pos = addr;
	ret = arch_page_phys_get(pos - CONFIG_MMU_PAGE_SIZE, ખાલી);
	__ASSERT(ret != 0,
		 "%s: cannot find preceding guard page for (%p, %zu)",
		 __func__, addr, size);
	જો (ret == 0) {
		જાઓ out;
	}

	ret = arch_page_phys_get(pos + size, ખાલી);
	__ASSERT(ret != 0,
		 "%s: cannot find succeeding guard page for (%p, %zu)",
		 __func__, addr, size);
	જો (ret == 0) {
		જાઓ out;
	}

	જો (is_anon) {
		/* Unmapping anonymous memory */
		ret = unmap_anon_pages(addr, size);
		__ASSERT(ret == 0, "%s: unmap_anon_pages() returned %d", __func__, ret);
		જો (ret != 0) {
			/* Fail to unmap. Must bail without releasing any resources
			 * as some pages are still be mapped.
			 */
			જાઓ out;
		}
	} નહીંતર {
		/*
		 * Unmapping previous mapped memory with specific physical address.
		 *
		 * Note that we don't have to unmap the guard pages, as they should
		 * have been unmapped. We just need to unmapped the in-between
		 * region [addr, (addr + size)).
		 */
		ret = arch_mem_unmap(addr, size);
		__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
		જો (ret != 0) {
			/* Fail to unmap. Must bail without releasing any resources
			 * as some pages are still be mapped.
			 */
			જાઓ out;
		}
	}

	/* There are guard pages just before and after the mapped
	 * region. So we also need to free them from the bitmap.
	 */
	pos = (uint8_t *)addr - CONFIG_MMU_PAGE_SIZE;
	total_size = size + (CONFIG_MMU_PAGE_SIZE * 2);
	virt_region_free(pos, total_size);

out:
	k_spin_unlock(&z_mm_lock, ચાવી);
}

પૂર્ણાંક k_mem_update_flags(શૂન્ય *addr, size_t size, uint32_t flags)
{
	uintptr_t phys;
	k_spinlock_ચાવી_t ચાવી;
	પૂર્ણાંક ret;

	k_mem_assert_virtual_region(addr, size);

	ચાવી = k_spin_lock(&z_mm_lock);

	/*
	 * We can achieve desired result without explicit architecture support
	 * by unmapping and remapping the same physical memory using new flags.
	 */

	ret = arch_page_phys_get(addr, &phys);
	જો (ret < 0) {
		જાઓ out;
	}

	/* TODO: detect and handle paged-out memory as well */

	ret = arch_mem_unmap(addr, size);
	__ASSERT(ret == 0, "%s: arch_mem_unmap failed %d\n", __func__, ret);
	જો (ret != 0) {
		જાઓ out;
	}

	ret = arch_mem_map(addr, phys, size, flags);
	__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);

out:
	k_spin_unlock(&z_mm_lock, ચાવી);
	પાછા ret;
}

size_t k_mem_free_get(શૂન્ય)
{
	size_t ret;
	k_spinlock_ચાવી_t ચાવી;

	__ASSERT(page_frames_initialized, "%s called too early", __func__);

	ચાવી = k_spin_lock(&z_mm_lock);
#ifdef CONFIG_DEMAND_PAGING
	જો (z_free_page_ગણતરી > CONFIG_DEMAND_PAGING_PAGE_FRAMES_RESERVE) {
		ret = z_free_page_ગણતરી - CONFIG_DEMAND_PAGING_PAGE_FRAMES_RESERVE;
	} નહીંતર {
		ret = 0;
	}
#નહીંતર
	ret = z_free_page_ગણતરી;
#endif /* CONFIG_DEMAND_PAGING */
	k_spin_unlock(&z_mm_lock, ચાવી);

	પાછા ret * (size_t)CONFIG_MMU_PAGE_SIZE;
}

/* Get the default virtual region alignment, here the default MMU page size
 *
 * @param[in] phys Physical address of region to be mapped, aligned to MMU_PAGE_SIZE
 * @param[in] size Size of region to be mapped, aligned to MMU_PAGE_SIZE
 *
 * @return alignment to apply on the virtual address of this region
 */
સ્થિર size_t virt_region_align(uintptr_t phys, size_t size)
{
	ARG_UNUSED(phys);
	ARG_UNUSED(size);

	પાછા CONFIG_MMU_PAGE_SIZE;
}

__weak FUNC_ALIAS(virt_region_align, arch_virt_region_align, size_t);

/* This may be called from arch early boot code before z_cstart() is invoked.
 * Data will be copied and BSS zeroed, but this must not rely on any
 * initialization functions being called prior to work correctly.
 */
શૂન્ય k_mem_map_phys_bare(uint8_t **virt_ptr, uintptr_t phys, size_t size, uint32_t flags)
{
	uintptr_t aligned_phys, addr_offset;
	size_t aligned_size, align_boundary;
	k_spinlock_ચાવી_t ચાવી;
	uint8_t *dest_addr;
	size_t અંક_bits;
	size_t offset;
	પૂર્ણાંક ret;

#ifndef CONFIG_KERNEL_DIRECT_MAP
	__ASSERT(!(flags & K_MEM_DIRECT_MAP), "The direct-map is not enabled");
#endif /* CONFIG_KERNEL_DIRECT_MAP */
	addr_offset = k_mem_region_align(&aligned_phys, &aligned_size,
					 phys, size,
					 CONFIG_MMU_PAGE_SIZE);
	__ASSERT(aligned_size != 0U, "0-length mapping at 0x%lx", aligned_phys);
	__ASSERT(aligned_size - 1 <= (UINTPTR_MAX - aligned_phys),
		 "wraparound for physical address 0x%lx (size %zu)",
		 aligned_phys, aligned_size);

	align_boundary = arch_virt_region_align(aligned_phys, aligned_size);

	ચાવી = k_spin_lock(&z_mm_lock);

	જો (IS_ENABLED(CONFIG_KERNEL_DIRECT_MAP) &&
	    (flags & K_MEM_DIRECT_MAP)) {
		dest_addr = (uint8_t *)aligned_phys;

		/* Mark the region of virtual memory bitmap as used
		 * if the region overlaps the virtual memory space.
		 *
		 * Basically if either end of region is within
		 * virtual memory space, we need to mark the bits.
		 */

		જો (IN_RANGE(aligned_phys,
			      (uintptr_t)K_MEM_VIRT_RAM_START,
			      (uintptr_t)(K_MEM_VIRT_RAM_END - 1)) ||
		    IN_RANGE(aligned_phys + aligned_size - 1,
			      (uintptr_t)K_MEM_VIRT_RAM_START,
			      (uintptr_t)(K_MEM_VIRT_RAM_END - 1))) {
			uint8_t *adjusted_start = max(dest_addr, K_MEM_VIRT_RAM_START);
			uint8_t *adjusted_end = min(dest_addr + aligned_size,
						    K_MEM_VIRT_RAM_END);
			size_t adjusted_sz = adjusted_end - adjusted_start;

			અંક_bits = adjusted_sz / CONFIG_MMU_PAGE_SIZE;
			offset = virt_to_bitmap_offset(adjusted_start, adjusted_sz);
			જો (sys_bitarray_test_and_set_region(
			    &virt_region_bitmap, અંક_bits, offset, true)) {
				જાઓ fail;
			}
		}
	} નહીંતર {
		/* Obtain an appropriately sized chunk of virtual memory */
		dest_addr = virt_region_alloc(aligned_size, align_boundary);
		જો (!dest_addr) {
			જાઓ fail;
		}
	}

	/* If this fails there's something amiss with virt_region_get */
	__ASSERT((uintptr_t)dest_addr <
		 ((uintptr_t)dest_addr + (size - 1)),
		 "wraparound for virtual address %p (size %zu)",
		 dest_addr, size);

	LOG_DBG("arch_mem_map(%p, 0x%lx, %zu, %x) offset %lu", (શૂન્ય *)dest_addr,
		aligned_phys, aligned_size, flags, addr_offset);

	ret = arch_mem_map(dest_addr, aligned_phys, aligned_size, flags);
	__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
	જો (ret != 0) {
		જાઓ fail;
	}
	k_spin_unlock(&z_mm_lock, ચાવી);

	*virt_ptr = dest_addr + addr_offset;
	પાછા;
fail:
	/* May re-visit this in the future, but for now running out of
	 * virtual address space or failing the arch_mem_map() call is
	 * an unrecoverable situation.
	 *
	 * Other problems not related to resource exhaustion we leave as
	 * assertions since they are clearly programming mistakes.
	 */
	LOG_ERR("memory mapping 0x%lx (size %zu, flags 0x%x) failed",
		phys, size, flags);
	k_panic();
}

શૂન્ય k_mem_unmap_phys_bare(uint8_t *virt, size_t size)
{
	uintptr_t aligned_virt, addr_offset;
	size_t aligned_size;
	k_spinlock_ચાવી_t ચાવી;
	પૂર્ણાંક ret;

	addr_offset = k_mem_region_align(&aligned_virt, &aligned_size,
					 POINTER_TO_UINT(virt), size,
					 CONFIG_MMU_PAGE_SIZE);
	__ASSERT(aligned_size != 0U, "0-length mapping at 0x%lx", aligned_virt);
	__ASSERT(aligned_size - 1 <= (UINTPTR_MAX - aligned_virt),
		 "wraparound for virtual address 0x%lx (size %zu)",
		 aligned_virt, aligned_size);

	ચાવી = k_spin_lock(&z_mm_lock);

	LOG_DBG("arch_mem_unmap(0x%lx, %zu) offset %lu",
		aligned_virt, aligned_size, addr_offset);

	ret = arch_mem_unmap(UINT_TO_POINTER(aligned_virt), aligned_size);
	__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
	જો (ret != 0) {
		/* Fail to unmap. We cannot free the virtual memory region
		 * as some pages are still be mapped.
		 */
		જાઓ out;
	}

	virt_region_free(UINT_TO_POINTER(aligned_virt), aligned_size);

out:
	k_spin_unlock(&z_mm_lock, ચાવી);
}

/*
 * Miscellaneous
 */

size_t k_mem_region_align(uintptr_t *aligned_addr, size_t *aligned_size,
			  uintptr_t addr, size_t size, size_t align)
{
	size_t addr_offset;

	/* The actual mapped region must be page-aligned. Round down the
	 * physical address and pad the region size appropriately
	 */
	*aligned_addr = ROUND_DOWN(addr, align);
	addr_offset = addr - *aligned_addr;
	*aligned_size = ROUND_UP(size + addr_offset, align);

	પાછા addr_offset;
}

#ifdef CONFIG_LINKER_USE_BOOT_SECTION
સ્થિર શૂન્ય mark_linker_section_pinned(શૂન્ય *start_addr, શૂન્ય *end_addr,
				       bool pin)
{
	સંરચના k_mem_page_frame *pf;
	uint8_t *addr;

	uintptr_t pinned_start = ROUND_DOWN(POINTER_TO_UINT(start_addr),
					    CONFIG_MMU_PAGE_SIZE);
	uintptr_t pinned_end = ROUND_UP(POINTER_TO_UINT(end_addr),
					CONFIG_MMU_PAGE_SIZE);
	size_t pinned_size = pinned_end - pinned_start;

	VIRT_FOREACH(UINT_TO_POINTER(pinned_start), pinned_size, addr)
	{
		pf = k_mem_phys_to_page_frame(K_MEM_BOOT_VIRT_TO_PHYS(addr));
		frame_mapped_set(pf, addr);

		જો (pin) {
			k_mem_page_frame_set(pf, K_MEM_PAGE_FRAME_PINNED);
		} નહીંતર {
			k_mem_page_frame_clear(pf, K_MEM_PAGE_FRAME_PINNED);
#ifdef CONFIG_DEMAND_PAGING
			જો (IS_ENABLED(CONFIG_EVICTION_TRACKING) &&
			    k_mem_page_frame_is_evictable(pf)) {
				k_mem_paging_eviction_add(pf);
			}
#endif
		}
	}
}
#endif /* CONFIG_LINKER_USE_BOOT_SECTION */

#ifdef CONFIG_LINKER_USE_ONDEMAND_SECTION
સ્થિર શૂન્ય z_paging_ondemand_section_map(શૂન્ય)
{
	uint8_t *addr;
	size_t size;
	uintptr_t location;
	uint32_t flags;
	પૂર્ણાંક ret;

	size = (uintptr_t)lnkr_ondemand_text_size;
	flags = K_MEM_MAP_UNPAGED | K_MEM_PERM_EXEC | K_MEM_CACHE_WB;
	VIRT_FOREACH(lnkr_ondemand_text_start, size, addr) {
		k_mem_paging_backing_store_location_query(addr, &location);

		ret = arch_mem_map(addr, location, CONFIG_MMU_PAGE_SIZE, flags);
		__ASSERT(ret == 0, "arch_mem_map() returned %d", ret);
		જો (ret != 0) {
			k_panic();
		}

		sys_bitarray_set_region(&virt_region_bitmap, 1,
					virt_to_bitmap_offset(addr, CONFIG_MMU_PAGE_SIZE));
	}

	size = (uintptr_t)lnkr_ondemand_rodata_size;
	flags = K_MEM_MAP_UNPAGED | K_MEM_CACHE_WB;
	VIRT_FOREACH(lnkr_ondemand_rodata_start, size, addr) {
		k_mem_paging_backing_store_location_query(addr, &location);

		ret = arch_mem_map(addr, location, CONFIG_MMU_PAGE_SIZE, flags);
		__ASSERT(ret == 0, "arch_mem_map() returned %d", ret);
		જો (ret != 0) {
			k_panic();
		}

		sys_bitarray_set_region(&virt_region_bitmap, 1,
					virt_to_bitmap_offset(addr, CONFIG_MMU_PAGE_SIZE));
	}
}
#endif /* CONFIG_LINKER_USE_ONDEMAND_SECTION */

શૂન્ય z_mem_manage_init(શૂન્ય)
{
	uintptr_t phys;
	uint8_t *addr;
	સંરચના k_mem_page_frame *pf;
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&z_mm_lock);

	free_page_frame_list_init();

	ARG_UNUSED(addr);

#ifdef CONFIG_ARCH_HAS_RESERVED_PAGE_FRAMES
	/* If some page frames are unavailable for use as memory, arch
	 * code will mark K_MEM_PAGE_FRAME_RESERVED in their flags
	 */
	arch_reserved_pages_update();
#endif /* CONFIG_ARCH_HAS_RESERVED_PAGE_FRAMES */

	/* The entire Zephyr image is mapped and pinned at boot. Demand
	 * paging applies only to anonymous mappings created via k_mem_map()
	 * and to memory placed in explicit __ondemand_* linker sections;
	 * the kernel image itself is never a candidate for eviction.
	 */
	VIRT_FOREACH(K_MEM_KERNEL_VIRT_START, K_MEM_KERNEL_VIRT_SIZE, addr)
	{
		pf = k_mem_phys_to_page_frame(K_MEM_BOOT_VIRT_TO_PHYS(addr));
		frame_mapped_set(pf, addr);
		k_mem_page_frame_set(pf, K_MEM_PAGE_FRAME_PINNED);
	}

#ifdef CONFIG_LINKER_USE_BOOT_SECTION
	/* Pin the boot section to prevent it from being swapped out during
	 * boot process. Will be un-pinned once boot process completes.
	 */
	mark_linker_section_pinned(lnkr_boot_start, lnkr_boot_end, true);
#endif /* CONFIG_LINKER_USE_BOOT_SECTION */

	/* Any remaining pages that aren't mapped, reserved, or pinned get
	 * added to the free pages list
	 */
	K_MEM_PAGE_FRAME_FOREACH(phys, pf) {
		જો (k_mem_page_frame_is_available(pf)) {
			free_page_frame_list_put(pf);
		}
	}
	LOG_DBG("free page frames: %zu", z_free_page_ગણતરી);

#ifdef CONFIG_DEMAND_PAGING
#ifdef CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	z_paging_histogram_init();
#endif /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
	k_mem_paging_backing_store_init();
	k_mem_paging_eviction_init();

	જો (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
		/* start tracking evictable page installed above if any */
		K_MEM_PAGE_FRAME_FOREACH(phys, pf) {
			જો (k_mem_page_frame_is_evictable(pf)) {
				k_mem_paging_eviction_add(pf);
			}
		}
	}
#endif /* CONFIG_DEMAND_PAGING */

#ifdef CONFIG_LINKER_USE_ONDEMAND_SECTION
	z_paging_ondemand_section_map();
#endif

#ifdef CONFIG_ASSERT
	page_frames_initialized = true;
#endif /* CONFIG_ASSERT */

	k_spin_unlock(&z_mm_lock, ચાવી);
}

શૂન્ય z_mem_manage_boot_finish(શૂન્ય)
{
#ifdef CONFIG_LINKER_USE_BOOT_SECTION
	/* At the end of boot process, unpin the boot sections
	 * as they don't need to be in memory all the time anymore.
	 */
	mark_linker_section_pinned(lnkr_boot_start, lnkr_boot_end, false);
#endif /* CONFIG_LINKER_USE_BOOT_SECTION */
}

#ifdef CONFIG_DEMAND_PAGING

#ifdef CONFIG_DEMAND_PAGING_STATS
સંરચના k_mem_paging_stats_t paging_stats;
બાહ્ય સંરચના k_mem_paging_histogram_t z_paging_histogram_eviction;
બાહ્ય સંરચના k_mem_paging_histogram_t z_paging_histogram_backing_store_page_in;
બાહ્ય સંરચના k_mem_paging_histogram_t z_paging_histogram_backing_store_page_out;
#endif /* CONFIG_DEMAND_PAGING_STATS */

સ્થિર અંતઃરેખ શૂન્ય do_backing_store_page_in(uintptr_t location)
{
#ifdef CONFIG_DEMAND_MAPPING
	/* Check for special cases */
	પસંદગી (location) {
	સ્થિતિ ARCH_UNPAGED_ANON_ZERO:
		memset(K_MEM_SCRATCH_PAGE, 0, CONFIG_MMU_PAGE_SIZE);
		__fallthrough;
	સ્થિતિ ARCH_UNPAGED_ANON_UNINIT:
		/* nothing else to do */
		પાછા;
	મૂળભૂત:
		તોડો;
	}
#endif /* CONFIG_DEMAND_MAPPING */

#ifdef CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	uint32_t time_diff;

#ifdef CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	timing_t time_start, time_end;

	time_start = timing_counter_get();
#નહીંતર
	uint32_t time_start;

	time_start = k_cycle_get_32();
#endif /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#endif /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	k_mem_paging_backing_store_page_in(location);

#ifdef CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
#ifdef CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	time_end = timing_counter_get();
	time_diff = (uint32_t)timing_cycles_get(&time_start, &time_end);
#નહીંતર
	time_diff = k_cycle_get_32() - time_start;
#endif /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */

	z_paging_histogram_inc(&z_paging_histogram_backing_store_page_in,
			       time_diff);
#endif /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
}

સ્થિર અંતઃરેખ શૂન્ય do_backing_store_page_out(uintptr_t location)
{
#ifdef CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	uint32_t time_diff;

#ifdef CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	timing_t time_start, time_end;

	time_start = timing_counter_get();
#નહીંતર
	uint32_t time_start;

	time_start = k_cycle_get_32();
#endif /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#endif /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	k_mem_paging_backing_store_page_out(location);

#ifdef CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
#ifdef CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	time_end = timing_counter_get();
	time_diff = (uint32_t)timing_cycles_get(&time_start, &time_end);
#નહીંતર
	time_diff = k_cycle_get_32() - time_start;
#endif /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */

	z_paging_histogram_inc(&z_paging_histogram_backing_store_page_out,
			       time_diff);
#endif /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
}

#જો defined(CONFIG_SMP) && defined(CONFIG_DEMAND_PAGING_ALLOW_IRQ)
/*
 * SMP support is very simple. Some resources such as the scratch page could
 * be made per CPU, backing store driver execution be confined to the faulting
 * CPU, statistics be made to cope with access concurrency, etc. But in the
 * end we're dealing with memory transfer to/from some external storage which
 * is inherently slow and whose access is most likely serialized anyway.
 * So let's simply enforce global demand paging serialization across all CPUs
 * with a mutex as there is no real gain from added parallelism here.
 */
સ્થિર K_MUTEX_DEFINE(z_mm_paging_lock);
#endif

સ્થિર શૂન્ય virt_region_foreach(શૂન્ય *addr, size_t size,
				શૂન્ય (*func)(શૂન્ય *vaddr))
{
	k_mem_assert_virtual_region(addr, size);

	માટે (size_t offset = 0; offset < size; offset += CONFIG_MMU_PAGE_SIZE) {
		func((uint8_t *)addr + offset);
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
સ્થિર પૂર્ણાંક page_frame_prepare_locked(સંરચના k_mem_page_frame *pf, bool *dirty_ptr,
				     bool page_fault, uintptr_t *location_ptr)
{
	uintptr_t phys;
	પૂર્ણાંક ret;
	bool dirty = *dirty_ptr;

	phys = k_mem_page_frame_to_phys(pf);
	__ASSERT(!k_mem_page_frame_is_pinned(pf), "page frame 0x%lx is pinned",
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
	જો (k_mem_page_frame_is_mapped(pf)) {
		dirty = dirty || !k_mem_page_frame_is_backed(pf);
	}

	જો (dirty || page_fault) {
		arch_mem_scratch(phys);
	}

	જો (k_mem_page_frame_is_mapped(pf)) {
		ret = k_mem_paging_backing_store_location_get(pf, location_ptr,
							      page_fault);
		જો (ret != 0) {
			LOG_ERR("out of backing store memory");
			પાછા -ENOMEM;
		}
		arch_mem_page_out(k_mem_page_frame_to_virt(pf), *location_ptr);

		જો (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
			k_mem_paging_eviction_remove(pf);
		}
	} નહીંતર {
		/* Shouldn't happen unless this function is mis-used */
		__ASSERT(!dirty, "un-mapped page determined to be dirty");
	}
#ifdef CONFIG_DEMAND_PAGING_ALLOW_IRQ
	/* Mark as busy so that k_mem_page_frame_is_evictable() returns false */
	__ASSERT(!k_mem_page_frame_is_busy(pf), "page frame 0x%lx is already busy",
		 phys);
	k_mem_page_frame_set(pf, K_MEM_PAGE_FRAME_BUSY);
#endif /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	/* Update dirty parameter, since we set to true if it wasn't backed
	 * even if otherwise clean
	 */
	*dirty_ptr = dirty;

	પાછા 0;
}

સ્થિર પૂર્ણાંક do_mem_evict(શૂન્ય *addr)
{
	bool dirty;
	સંરચના k_mem_page_frame *pf;
	uintptr_t location;
	k_spinlock_ચાવી_t ચાવી;
	uintptr_t flags, phys;
	પૂર્ણાંક ret;

#જો CONFIG_DEMAND_PAGING_ALLOW_IRQ
	__ASSERT(!k_is_in_isr(),
		 "%s is unavailable in ISRs with CONFIG_DEMAND_PAGING_ALLOW_IRQ",
		 __func__);
#ifdef CONFIG_SMP
	k_mutex_lock(&z_mm_paging_lock, K_FOREVER);
#નહીંતર
	k_sched_lock();
#endif
#endif /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	ચાવી = k_spin_lock(&z_mm_lock);
	flags = arch_page_info_get(addr, &phys, false);
	__ASSERT((flags & ARCH_DATA_PAGE_NOT_MAPPED) == 0,
		 "address %p isn't mapped", addr);
	જો ((flags & ARCH_DATA_PAGE_LOADED) == 0) {
		/* Un-mapped or already evicted. Nothing to do */
		ret = 0;
		જાઓ out;
	}

	dirty = (flags & ARCH_DATA_PAGE_DIRTY) != 0;
	pf = k_mem_phys_to_page_frame(phys);
	__ASSERT(k_mem_page_frame_to_virt(pf) == addr, "page frame address mismatch");
	ret = page_frame_prepare_locked(pf, &dirty, false, &location);
	જો (ret != 0) {
		જાઓ out;
	}

#ifdef CONFIG_DEMAND_PAGING_ALLOW_IRQ
	k_spin_unlock(&z_mm_lock, ચાવી);
#endif /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	જો (dirty) {
		do_backing_store_page_out(location);
	}
#ifdef CONFIG_DEMAND_PAGING_ALLOW_IRQ
	ચાવી = k_spin_lock(&z_mm_lock);
#endif /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	page_frame_free_locked(pf);
out:
	k_spin_unlock(&z_mm_lock, ચાવી);
#ifdef CONFIG_DEMAND_PAGING_ALLOW_IRQ
#ifdef CONFIG_SMP
	k_mutex_unlock(&z_mm_paging_lock);
#નહીંતર
	k_sched_unlock();
#endif
#endif /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	પાછા ret;
}

પૂર્ણાંક k_mem_page_out(શૂન્ય *addr, size_t size)
{
	__ASSERT(page_frames_initialized, "%s called on %p too early", __func__,
		 addr);
	k_mem_assert_virtual_region(addr, size);

	માટે (size_t offset = 0; offset < size; offset += CONFIG_MMU_PAGE_SIZE) {
		શૂન્ય *pos = (uint8_t *)addr + offset;
		પૂર્ણાંક ret;

		ret = do_mem_evict(pos);
		જો (ret != 0) {
			પાછા ret;
		}
	}

	પાછા 0;
}

પૂર્ણાંક k_mem_page_frame_evict(uintptr_t phys)
{
	k_spinlock_ચાવી_t ચાવી;
	સંરચના k_mem_page_frame *pf;
	bool dirty;
	uintptr_t flags;
	uintptr_t location;
	પૂર્ણાંક ret;

	__ASSERT(page_frames_initialized, "%s called on 0x%lx too early",
		 __func__, phys);

	/* Implementation is similar to do_page_fault() except there is no
	 * data page to page-in, see comments in that function.
	 */

#ifdef CONFIG_DEMAND_PAGING_ALLOW_IRQ
	__ASSERT(!k_is_in_isr(),
		 "%s is unavailable in ISRs with CONFIG_DEMAND_PAGING_ALLOW_IRQ",
		 __func__);
#ifdef CONFIG_SMP
	k_mutex_lock(&z_mm_paging_lock, K_FOREVER);
#નહીંતર
	k_sched_lock();
#endif
#endif /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	ચાવી = k_spin_lock(&z_mm_lock);
	pf = k_mem_phys_to_page_frame(phys);
	જો (!k_mem_page_frame_is_mapped(pf)) {
		/* Nothing to do, free page */
		ret = 0;
		જાઓ out;
	}
	flags = arch_page_info_get(k_mem_page_frame_to_virt(pf), ખાલી, false);
	/* Shouldn't ever happen */
	__ASSERT((flags & ARCH_DATA_PAGE_LOADED) != 0, "data page not loaded");
	dirty = (flags & ARCH_DATA_PAGE_DIRTY) != 0;
	ret = page_frame_prepare_locked(pf, &dirty, false, &location);
	જો (ret != 0) {
		જાઓ out;
	}

#ifdef CONFIG_DEMAND_PAGING_ALLOW_IRQ
	k_spin_unlock(&z_mm_lock, ચાવી);
#endif /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	જો (dirty) {
		do_backing_store_page_out(location);
	}
#ifdef CONFIG_DEMAND_PAGING_ALLOW_IRQ
	ચાવી = k_spin_lock(&z_mm_lock);
#endif /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	page_frame_free_locked(pf);
out:
	k_spin_unlock(&z_mm_lock, ચાવી);
#ifdef CONFIG_DEMAND_PAGING_ALLOW_IRQ
#ifdef CONFIG_SMP
	k_mutex_unlock(&z_mm_paging_lock);
#નહીંતર
	k_sched_unlock();
#endif
#endif /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	પાછા ret;
}

સ્થિર અંતઃરેખ શૂન્ય paging_stats_faults_inc(સંરચના k_thread *faulting_thread,
					   પૂર્ણાંક ચાવી)
{
#ifdef CONFIG_DEMAND_PAGING_STATS
	bool is_irq_unlocked = arch_irq_unlocked(ચાવી);

	paging_stats.pagefaults.cnt++;

	જો (is_irq_unlocked) {
		paging_stats.pagefaults.irq_unlocked++;
	} નહીંતર {
		paging_stats.pagefaults.irq_locked++;
	}

#ifdef CONFIG_DEMAND_PAGING_THREAD_STATS
	faulting_thread->paging_stats.pagefaults.cnt++;

	જો (is_irq_unlocked) {
		faulting_thread->paging_stats.pagefaults.irq_unlocked++;
	} નહીંતર {
		faulting_thread->paging_stats.pagefaults.irq_locked++;
	}
#નહીંતર
	ARG_UNUSED(faulting_thread);
#endif /* CONFIG_DEMAND_PAGING_THREAD_STATS */

#ifndef CONFIG_DEMAND_PAGING_ALLOW_IRQ
	જો (k_is_in_isr()) {
		paging_stats.pagefaults.in_isr++;

#ifdef CONFIG_DEMAND_PAGING_THREAD_STATS
		faulting_thread->paging_stats.pagefaults.in_isr++;
#endif /* CONFIG_DEMAND_PAGING_THREAD_STATS */
	}
#endif /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
#endif /* CONFIG_DEMAND_PAGING_STATS */
}

સ્થિર અંતઃરેખ શૂન્ય paging_stats_eviction_inc(સંરચના k_thread *faulting_thread,
					     bool dirty)
{
#ifdef CONFIG_DEMAND_PAGING_STATS
	જો (dirty) {
		paging_stats.eviction.dirty++;
	} નહીંતર {
		paging_stats.eviction.clean++;
	}
#ifdef CONFIG_DEMAND_PAGING_THREAD_STATS
	જો (dirty) {
		faulting_thread->paging_stats.eviction.dirty++;
	} નહીંતર {
		faulting_thread->paging_stats.eviction.clean++;
	}
#નહીંતર
	ARG_UNUSED(faulting_thread);
#endif /* CONFIG_DEMAND_PAGING_THREAD_STATS */
#endif /* CONFIG_DEMAND_PAGING_STATS */
}

સ્થિર અંતઃરેખ સંરચના k_mem_page_frame *do_eviction_select(bool *dirty)
{
	સંરચના k_mem_page_frame *pf;

#ifdef CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	uint32_t time_diff;

#ifdef CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	timing_t time_start, time_end;

	time_start = timing_counter_get();
#નહીંતર
	uint32_t time_start;

	time_start = k_cycle_get_32();
#endif /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#endif /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	pf = k_mem_paging_eviction_select(dirty);

#ifdef CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
#ifdef CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	time_end = timing_counter_get();
	time_diff = (uint32_t)timing_cycles_get(&time_start, &time_end);
#નહીંતર
	time_diff = k_cycle_get_32() - time_start;
#endif /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */

	z_paging_histogram_inc(&z_paging_histogram_eviction, time_diff);
#endif /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	પાછા pf;
}

સ્થિર bool do_page_fault(શૂન્ય *addr, bool pin)
{
	સંરચના k_mem_page_frame *pf;
	k_spinlock_ચાવી_t ચાવી;
	uintptr_t page_in_location, page_out_location;
	ગણના arch_page_location status;
	bool result;
	bool dirty = false;
	સંરચના k_thread *faulting_thread;
	પૂર્ણાંક ret;

	__ASSERT(page_frames_initialized, "page fault at %p happened too early",
		 addr);

	LOG_DBG("page fault at %p", addr);

	/*
	 * TODO: Add performance accounting:
	 * - k_mem_paging_eviction_select() metrics
	 *   * periodic timer execution time histogram (if implemented)
	 */

#ifdef CONFIG_DEMAND_PAGING_ALLOW_IRQ
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
	__ASSERT(!k_is_in_isr(), "ISR page faults are forbidden");
#ifdef CONFIG_SMP
	k_mutex_lock(&z_mm_paging_lock, K_FOREVER);
#નહીંતર
	k_sched_lock();
#endif
#endif /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */

	ચાવી = k_spin_lock(&z_mm_lock);
	faulting_thread = _current;

	status = arch_page_location_get(addr, &page_in_location);
	જો (status == ARCH_PAGE_LOCATION_BAD) {
		/* Return false to treat as a fatal error */
		result = false;
		જાઓ out;
	}
	result = true;

	જો (status == ARCH_PAGE_LOCATION_PAGED_IN) {
		જો (pin) {
			/* It's a physical memory address */
			uintptr_t phys = page_in_location;

			pf = k_mem_phys_to_page_frame(phys);
			જો (!k_mem_page_frame_is_pinned(pf)) {
				જો (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
					k_mem_paging_eviction_remove(pf);
				}
				k_mem_page_frame_set(pf, K_MEM_PAGE_FRAME_PINNED);
			}
		}

		/* This if-block is to pin the page if it is
		 * already present in physical memory. There is
		 * no need to go through the following code to
		 * pull in the data pages. So skip to the end.
		 */
		જાઓ out;
	}
	__ASSERT(status == ARCH_PAGE_LOCATION_PAGED_OUT,
		 "unexpected status value %d", status);

	paging_stats_faults_inc(faulting_thread, ચાવી.ચાવી);

	pf = free_page_frame_list_get();
	જો (pf == ખાલી) {
		/* Need to evict a page frame */
		pf = do_eviction_select(&dirty);
		જો (pf == ખાલી) {
			/* Every evictable page frame is pinned or busy.
			 * Fail the fault so it is reported as fatal.
			 */
			result = false;
			જાઓ out;
		}
		LOG_DBG("evicting %p at 0x%lx",
			k_mem_page_frame_to_virt(pf),
			k_mem_page_frame_to_phys(pf));

		paging_stats_eviction_inc(faulting_thread, dirty);
	}
	ret = page_frame_prepare_locked(pf, &dirty, true, &page_out_location);
	જો (ret != 0) {
		/* Backing store is full. Fail the fault so it is
		 * reported as fatal.
		 */
		result = false;
		જાઓ out;
	}

#ifdef CONFIG_DEMAND_PAGING_ALLOW_IRQ
	k_spin_unlock(&z_mm_lock, ચાવી);
	/* Interrupts are now unlocked if they were not locked when we entered
	 * this function, and we may service ISRs. The scheduler is still
	 * locked.
	 */
#endif /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	જો (dirty) {
		do_backing_store_page_out(page_out_location);
	}
	do_backing_store_page_in(page_in_location);

#ifdef CONFIG_DEMAND_PAGING_ALLOW_IRQ
	ચાવી = k_spin_lock(&z_mm_lock);
	k_mem_page_frame_clear(pf, K_MEM_PAGE_FRAME_BUSY);
#endif /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	k_mem_page_frame_clear(pf, K_MEM_PAGE_FRAME_MAPPED);
	frame_mapped_set(pf, addr);
	જો (pin) {
		k_mem_page_frame_set(pf, K_MEM_PAGE_FRAME_PINNED);
	}

	arch_mem_page_in(addr, k_mem_page_frame_to_phys(pf));
	k_mem_paging_backing_store_page_finalize(pf, page_in_location);
	જો (IS_ENABLED(CONFIG_EVICTION_TRACKING) && (!pin)) {
		k_mem_paging_eviction_add(pf);
	}
out:
	k_spin_unlock(&z_mm_lock, ચાવી);
#ifdef CONFIG_DEMAND_PAGING_ALLOW_IRQ
#ifdef CONFIG_SMP
	k_mutex_unlock(&z_mm_paging_lock);
#નહીંતર
	k_sched_unlock();
#endif
#endif /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */

	પાછા result;
}

સ્થિર શૂન્ય do_page_in(શૂન્ય *addr)
{
	bool ret;

	ret = do_page_fault(addr, false);
	જો (!ret) {
		/* There is no error reporting channel to the caller, and
		 * continuing would defer the failure to an arbitrary later
		 * access.
		 */
		LOG_ERR("cannot page in address %p", addr);
		k_panic();
	}
}

શૂન્ય k_mem_page_in(શૂન્ય *addr, size_t size)
{
	__ASSERT(!IS_ENABLED(CONFIG_DEMAND_PAGING_ALLOW_IRQ) || !k_is_in_isr(),
		 "%s may not be called in ISRs if CONFIG_DEMAND_PAGING_ALLOW_IRQ is enabled",
		 __func__);
	virt_region_foreach(addr, size, do_page_in);
}

સ્થિર શૂન્ય do_mem_pin(શૂન્ય *addr)
{
	bool ret;

	ret = do_page_fault(addr, true);
	જો (!ret) {
		/* There is no error reporting channel to the caller, and
		 * continuing with the memory not actually pinned would
		 * defer the failure to an arbitrary later access, possibly
		 * from a context that cannot handle a page fault.
		 */
		LOG_ERR("cannot pin address %p", addr);
		k_panic();
	}
}

શૂન્ય k_mem_pin(શૂન્ય *addr, size_t size)
{
	__ASSERT(!IS_ENABLED(CONFIG_DEMAND_PAGING_ALLOW_IRQ) || !k_is_in_isr(),
		 "%s may not be called in ISRs if CONFIG_DEMAND_PAGING_ALLOW_IRQ is enabled",
		 __func__);
	virt_region_foreach(addr, size, do_mem_pin);
}

bool k_mem_page_fault(શૂન્ય *addr)
{
	પાછા do_page_fault(addr, false);
}

સ્થિર શૂન્ય do_mem_unpin(શૂન્ય *addr)
{
	સંરચના k_mem_page_frame *pf;
	k_spinlock_ચાવી_t ચાવી;
	uintptr_t flags, phys;

	ચાવી = k_spin_lock(&z_mm_lock);
	flags = arch_page_info_get(addr, &phys, false);
	__ASSERT((flags & ARCH_DATA_PAGE_NOT_MAPPED) == 0,
		 "invalid data page at %p", addr);
	જો ((flags & ARCH_DATA_PAGE_LOADED) != 0) {
		pf = k_mem_phys_to_page_frame(phys);
		જો (k_mem_page_frame_is_pinned(pf)) {
			k_mem_page_frame_clear(pf, K_MEM_PAGE_FRAME_PINNED);

			જો (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
				k_mem_paging_eviction_add(pf);
			}
		}
	}
	k_spin_unlock(&z_mm_lock, ચાવી);
}

શૂન્ય k_mem_unpin(શૂન્ય *addr, size_t size)
{
	__ASSERT(page_frames_initialized, "%s called on %p too early", __func__,
		 addr);
	virt_region_foreach(addr, size, do_mem_unpin);
}

#endif /* CONFIG_DEMAND_PAGING */
