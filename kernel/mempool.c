/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <zephyr/sys/math_extras.h>
#అన్తర్భావ <zephyr/sys/util.h>

రకనిర్వచనం శూన్య * (sys_కుప్ప_allocator_t)(నిర్మాణం sys_కుప్ప *కుప్ప, size_t align, size_t బైట్లు);

స్థిర శూన్య *z_alloc_helper(నిర్మాణం k_కుప్ప *కుప్ప, size_t align, size_t కొలత,
			    sys_కుప్ప_allocator_t sys_కుప్ప_allocator)
{
	శూన్య *mem;
	నిర్మాణం k_కుప్ప **కుప్ప_ref;
	size_t __align;
	k_spinlock_కీలకం_t కీలకం;

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		"align must be a power of 2");

	/*
	 * Adjust the size to make room for our heap reference.
	 * Merge a rewind bit with align value (see sys_heap_aligned_alloc()).
	 * This allows for storing the heap pointer right below the aligned
	 * boundary without wasting any memory.
	 */
	ఒకవేళ (కొలత_కూడు_అధికప్రవాహం(కొలత, పరిమాణం(కుప్ప_ref), &కొలత)) {
		తిరిగి ఖాళీ;
	}
	__align = align | పరిమాణం(కుప్ప_ref);

	/*
	 * No point calling k_heap_malloc/k_heap_aligned_alloc with K_NO_WAIT.
	 * Better bypass them and go directly to sys_heap_*() instead.
	 */
	కీలకం = k_spin_తాళం(&కుప్ప->తాళం);
	mem = sys_కుప్ప_allocator(&కుప్ప->కుప్ప, __align, కొలత);
	k_spin_unlock(&కుప్ప->తాళం, కీలకం);

	ఒకవేళ (mem == ఖాళీ) {
		తిరిగి ఖాళీ;
	}

	కుప్ప_ref = mem;
	*కుప్ప_ref = కుప్ప;
	mem = ++కుప్ప_ref;
	__ASSERT(align == 0 || ((uintptr_t)mem & (align - 1)) == 0,
		 "misaligned memory at %p (align = %zu)", mem, align);

	తిరిగి mem;
}

శూన్య k_free(శూన్య *ptr)
{
	నిర్మాణం k_కుప్ప **కుప్ప_ref;

	ఒకవేళ (ptr != ఖాళీ) {
		కుప్ప_ref = ptr;
		--కుప్ప_ref;
		ptr = కుప్ప_ref;

		SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_కుప్ప_sys, k_free, *కుప్ప_ref, కుప్ప_ref);

		/*
		 * Bypass k_heap_free() as z_unpend_all() and its scheduler
		 * locking is unnecessary here, similar to z_alloc_helper()
		 * bypassing k_heap_alloc() to go directly to sys_heap_*().
		 */
		నిర్మాణం k_కుప్ప *కుప్ప = *కుప్ప_ref;
		k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&కుప్ప->తాళం);

		sys_కుప్ప_free(&కుప్ప->కుప్ప, ptr);
		k_spin_unlock(&కుప్ప->తాళం, కీలకం);

		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_కుప్ప_sys, k_free, కుప్ప, ptr);
	}
}

#ఒకవేళ (K_కుప్ప_MEM_POOL_కొలత > 0)

K_కుప్ప_DEFINE(system_కుప్ప, Z_కుప్ప_MIN_కొలత_FOR(K_కుప్ప_MEM_POOL_కొలత));
#నిర్వచించు SYSTEM_కుప్ప (&system_కుప్ప)

#ఒకవేళ defined(CONFIG_SYS_కుప్ప_KASAN_SYSTEM)
#అన్తర్భావ <zephyr/sys/heap_kasan.h>
K_కుప్ప_KASAN_ENABLE(system_కుప్ప, Z_కుప్ప_MIN_కొలత_FOR(K_కుప్ప_MEM_POOL_కొలత));
#అయితే_ముగింపు

శూన్య *k_aligned_alloc(size_t align, size_t కొలత)
{
	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_కుప్ప_sys, k_aligned_alloc, SYSTEM_కుప్ప);

	శూన్య *ret = z_alloc_helper(SYSTEM_కుప్ప, align, కొలత, sys_కుప్ప_aligned_alloc);

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_కుప్ప_sys, k_aligned_alloc, SYSTEM_కుప్ప, ret);

	తిరిగి ret;
}

శూన్య *k_malloc(size_t కొలత)
{
	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_కుప్ప_sys, k_malloc, SYSTEM_కుప్ప);

	శూన్య *ret = z_alloc_helper(SYSTEM_కుప్ప, 0, కొలత, sys_కుప్ప_noalign_alloc);

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_కుప్ప_sys, k_malloc, SYSTEM_కుప్ప, ret);

	తిరిగి ret;
}

శూన్య *k_calloc(size_t nmemb, size_t కొలత)
{
	శూన్య *ret;
	size_t bounds;

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_కుప్ప_sys, k_calloc, SYSTEM_కుప్ప);

	ఒకవేళ (కొలత_mul_అధికప్రవాహం(nmemb, కొలత, &bounds)) {
		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_కుప్ప_sys, k_calloc, SYSTEM_కుప్ప, ఖాళీ);

		తిరిగి ఖాళీ;
	}

	ret = k_malloc(bounds);
	ఒకవేళ (ret != ఖాళీ) {
		(శూన్య)memset(ret, 0, bounds);
	}

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_కుప్ప_sys, k_calloc, SYSTEM_కుప్ప, ret);

	తిరిగి ret;
}

శూన్య *k_realloc(శూన్య *ptr, size_t కొలత)
{
	నిర్మాణం k_కుప్ప *కుప్ప, **కుప్ప_ref;
	k_spinlock_కీలకం_t కీలకం;
	శూన్య *ret;

	ఒకవేళ (కొలత == 0) {
		k_free(ptr);
		తిరిగి ఖాళీ;
	}
	ఒకవేళ (ptr == ఖాళీ) {
		తిరిగి k_malloc(కొలత);
	}
	కుప్ప_ref = ptr;
	ptr = --కుప్ప_ref;
	కుప్ప = *కుప్ప_ref;

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_కుప్ప_sys, k_realloc, కుప్ప, ptr);

	ఒకవేళ (కొలత_కూడు_అధికప్రవాహం(కొలత, పరిమాణం(కుప్ప_ref), &కొలత)) {
		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_కుప్ప_sys, k_realloc, కుప్ప, ptr, ఖాళీ);
		తిరిగి ఖాళీ;
	}

	/*
	 * No point calling k_heap_realloc() with K_NO_WAIT here.
	 * Better bypass it and go directly to sys_heap_realloc() instead.
	 */
	కీలకం = k_spin_తాళం(&కుప్ప->తాళం);
	ret = sys_కుప్ప_realloc(&కుప్ప->కుప్ప, ptr, కొలత);
	k_spin_unlock(&కుప్ప->తాళం, కీలకం);

	ఒకవేళ (ret != ఖాళీ) {
		కుప్ప_ref = ret;
		ret = ++కుప్ప_ref;
	}

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_కుప్ప_sys, k_realloc, కుప్ప, ptr, ret);

	తిరిగి ret;
}

శూన్య k_thread_system_pool_కేటాయించు(నిర్మాణం k_thread *thread)
{
	thread->resource_pool = SYSTEM_కుప్ప;
}
#లేకపోతే
#నిర్వచించు SYSTEM_కుప్ప	ఖాళీ
#అయితే_ముగింపు /* K_HEAP_MEM_POOL_SIZE */

స్థిర శూన్య *z_thread_alloc_helper(size_t align, size_t కొలత,
				   sys_కుప్ప_allocator_t sys_కుప్ప_allocator)
{
	శూన్య *ret;
	నిర్మాణం k_కుప్ప *కుప్ప;

	ఒకవేళ (k_is_లోపల_isr()) {
		కుప్ప = SYSTEM_కుప్ప;
	} లేకపోతే {
		కుప్ప = _current->resource_pool;
	}

	ఒకవేళ (కుప్ప != ఖాళీ) {
		ret = z_alloc_helper(కుప్ప, align, కొలత, sys_కుప్ప_allocator);
	} లేకపోతే {
		ret = ఖాళీ;
	}

	తిరిగి ret;
}

శూన్య *z_thread_aligned_alloc(size_t align, size_t కొలత)
{
	తిరిగి z_thread_alloc_helper(align, కొలత, sys_కుప్ప_aligned_alloc);
}

శూన్య *z_thread_malloc(size_t కొలత)
{
	తిరిగి z_thread_alloc_helper(0, కొలత, sys_కుప్ప_noalign_alloc);
}

/*
 * There is no guarantee that functions in kheap.c are being used,
 * meaning the linker may not pull it in at all. Yet we need the
 * static heaps to be initialized and that is done in that file.
 * Make sure it is pulled in at least for that; the rest will be GC'd.
 */
స్థిర శూన్య *స్థిరరాశి __used kheap_ref = k_కుప్ప_init;
