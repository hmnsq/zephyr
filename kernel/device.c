/*
 * Copyright (c) 2015-2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <stddef.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <zephyr/device.h>
#অন্তর্ভাব <zephyr/sys/atomic.h>
#অন্তর্ভাব <zephyr/sys/iterable_sections.h>
#অন্তর্ভাব <zephyr/sys/kobject.h>
#অন্তর্ভাব <zephyr/internal/syscall_handler.h>
#অন্তর্ভাব <zephyr/toolchain.h>
#অন্তর্ভাব <zephyr/pm/device_runtime.h>

পূর্ণাঙ্ক do_device_init(ধ্রুব সংরচনা device *dev)
{
	পূর্ণাঙ্ক rc = 0;

	যদি (dev->ops.init != রিক্ত) {
		rc = dev->ops.init(dev);
		/* If initialization failed, record in dev->state->init_res
		 * the POSITIVE value of the resulting errno
		 */
		যদি (rc != 0) {
			/* device's init function should return:
			 *   0 on success
			 *   a negative value on failure (-errno)
			 * errno value maps to an uint8_t range as of now.
			 */
			__ASSERT(rc >= -UINT8_MAX && rc < 0, "device %s init: invalid error (%d)",
				 dev->name, rc);

			যদি (rc < 0) {
				rc = -rc;
			}
			/* handle error value overflow in production
			 * this is likely a bug in the device's init function. Signals it
			 */
			যদি (rc > UINT8_MAX) {
				rc = UINT8_MAX;
			}
		}

		/* Always record the result: a device that failed to initialize
		 * earlier must not keep that error once it initializes
		 * successfully.
		 */
		dev->state->init_res = rc;
	}

	যদি (rc == 0) {
		/* A failed init leaves the device uninitialized, so it can be
		 * retried by device_init() and nothing can be released by
		 * device_deinit().
		 */
		dev->state->initialized = true;

		/* Run automatic device runtime enablement */
		(শূন্য)pm_device_runtime_auto_enable(dev);
	}

	/* here, the value of rc is either 0 or +errno
	 * flip the sign to return a negative value on failure as expected
	 */
	প্রত্যাগম -rc;
}

পূর্ণাঙ্ক z_impl_device_init(ধ্রুব সংরচনা device *dev)
{
	যদি (dev->state->initialized) {
		প্রত্যাগম -EALREADY;
	}

	প্রত্যাগম do_device_init(dev);
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ পূর্ণাঙ্ক z_vrfy_device_init(ধ্রুব সংরচনা device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	প্রত্যাগম z_impl_device_init(dev);
}
#অন্তর্ভাব <zephyr/syscalls/device_init_mrsh.c>
#endif


ধ্রুব সংরচনা device *z_impl_device_get_binding(ধ্রুব বর্ণ *name)
{
	/* A null string identifies no device.  So does an empty
	 * string.
	 */
	যদি ((name == রিক্ত) || (name[0] == '\0')) {
		প্রত্যাগম রিক্ত;
	}

	/* Return NULL if the device matching 'name' is not ready. */
	STRUCT_SECTION_FOREACH(device, dev) {
		যদি ((dev->name == name) || (strcmp(name, dev->name) == 0)) {
			প্রত্যাগম z_impl_device_is_ready(dev) ? dev : রিক্ত;
		}
	}

	প্রত্যাগম রিক্ত;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ ধ্রুব সংরচনা device *z_vrfy_device_get_binding(ধ্রুব বর্ণ *name)
{
	বর্ণ name_copy[Z_DEVICE_MAX_NAME_LEN];

	যদি (k_usermode_string_copy(name_copy, name, পরিমাণ(name_copy))
	    != 0) {
		প্রত্যাগম রিক্ত;
	}

	প্রত্যাগম z_impl_device_get_binding(name_copy);
}
#অন্তর্ভাব <zephyr/syscalls/device_get_binding_mrsh.c>

স্থির অন্তঃস্থ bool z_vrfy_device_is_ready(ধ্রুব সংরচনা device *dev)
{
	/* NULL is a documented argument, not an invalid kernel object */
	যদি (dev == রিক্ত) {
		প্রত্যাগম false;
	}

	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	প্রত্যাগম z_impl_device_is_ready(dev);
}
#অন্তর্ভাব <zephyr/syscalls/device_is_ready_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_DEVICE_DT_METADATA
ধ্রুব সংরচনা device *z_impl_device_get_by_dt_nodelabel(ধ্রুব বর্ণ *nodelabel)
{
	/* For consistency with device_get_binding(). */
	যদি ((nodelabel == রিক্ত) || (nodelabel[0] == '\0')) {
		প্রত্যাগম রিক্ত;
	}

	/* Unlike device_get_binding(), which has a history of being
	 * used in application code, we don't expect
	 * device_get_by_dt_nodelabel() to be used outside of
	 * scenarios where a human is in the loop. The shell is the
	 * main expected use case. Therefore, nodelabel is probably
	 * not the same pointer as any of the entry->nodelabel
	 * elements. We therefore skip the pointer comparison that
	 * device_get_binding() does.
	 */
	STRUCT_SECTION_FOREACH(device, dev) {
		ধ্রুব সংরচনা device_dt_nodelabels *nl = device_get_dt_nodelabels(dev);

		যদি (!z_impl_device_is_ready(dev) || nl == রিক্ত) {
			অনুবর্ত;
		}

		প্রতি (size_t i = 0; i < nl->অঙ্ক_nodelabels; i++) {
			ধ্রুব বর্ণ *dev_nodelabel = nl->nodelabels[i];

			যদি (strcmp(nodelabel, dev_nodelabel) == 0) {
				প্রত্যাগম dev;
			}
		}
	}

	প্রত্যাগম রিক্ত;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ ধ্রুব সংরচনা device *z_vrfy_device_get_by_dt_nodelabel(ধ্রুব বর্ণ *nodelabel)
{
	বর্ণ nl_copy[Z_DEVICE_MAX_NODELABEL_LEN];

	যদি (k_usermode_string_copy(nl_copy, (বর্ণ *)nodelabel, পরিমাণ(nl_copy)) != 0) {
		প্রত্যাগম রিক্ত;
	}

	প্রত্যাগম z_impl_device_get_by_dt_nodelabel(nl_copy);
}
#অন্তর্ভাব <zephyr/syscalls/device_get_by_dt_nodelabel_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_DEVICE_DT_METADATA */

size_t z_device_get_all_static(সংরচনা device ধ্রুব **devices)
{
	size_t cnt;

	STRUCT_SECTION_GET(device, 0, devices);
	STRUCT_SECTION_গুনতি(device, &cnt);

	প্রত্যাগম cnt;
}

bool z_impl_device_is_ready(ধ্রুব সংরচনা device *dev)
{
	/*
	 * if an invalid device pointer is passed as argument, this call
	 * reports the `device` as not ready for usage.
	 */
	যদি (dev == রিক্ত) {
		প্রত্যাগম false;
	}

	প্রত্যাগম dev->state->initialized;
}

পূর্ণাঙ্ক z_impl_device_deinit(ধ্রুব সংরচনা device *dev)
{
#ifdef CONFIG_DEVICE_DEINIT_SUPPORT
	পূর্ণাঙ্ক ret;

	যদি (!dev->state->initialized) {
		প্রত্যাগম -EPERM;
	}

	যদি (dev->ops.deinit == রিক্ত) {
		প্রত্যাগম -ENOTSUP;
	}

	ret = dev->ops.deinit(dev);
	যদি (ret < 0) {
		প্রত্যাগম ret;
	}

	dev->state->initialized = false;

	প্রত্যাগম 0;
#অন্যথা
	ARG_UNUSED(dev);
	প্রত্যাগম -ENOTSUP;
#endif /* CONFIG_DEVICE_DEINIT_SUPPORT */
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ পূর্ণাঙ্ক z_vrfy_device_deinit(ধ্রুব সংরচনা device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	প্রত্যাগম z_impl_device_deinit(dev);
}
#অন্তর্ভাব <zephyr/syscalls/device_deinit_mrsh.c>
#endif

#ifdef CONFIG_DEVICE_DEPS

স্থির পূর্ণাঙ্ক device_visitor(ধ্রুব device_handle_t *handles,
			   size_t handle_গুনতি,
			   device_visitor_callback_t visitor_cb,
			   শূন্য *context)
{
	/* Iterate over fixed devices */
	প্রতি (size_t i = 0; i < handle_গুনতি; ++i) {
		device_handle_t dh = handles[i];
		ধ্রুব সংরচনা device *rdev = device_from_handle(dh);
		পূর্ণাঙ্ক rc = visitor_cb(rdev, context);

		যদি (rc < 0) {
			প্রত্যাগম rc;
		}
	}

	প্রত্যাগম handle_গুনতি;
}

পূর্ণাঙ্ক device_required_foreach(ধ্রুব সংরচনা device *dev,
			    device_visitor_callback_t visitor_cb,
			    শূন্য *context)
{
	size_t handle_গুনতি = 0;
	ধ্রুব device_handle_t *handles = device_required_handles_get(dev, &handle_গুনতি);

	প্রত্যাগম device_visitor(handles, handle_গুনতি, visitor_cb, context);
}

পূর্ণাঙ্ক device_supported_foreach(ধ্রুব সংরচনা device *dev,
			     device_visitor_callback_t visitor_cb,
			     শূন্য *context)
{
	size_t handle_গুনতি = 0;
	ধ্রুব device_handle_t *handles = device_supported_handles_get(dev, &handle_গুনতি);

	প্রত্যাগম device_visitor(handles, handle_গুনতি, visitor_cb, context);
}

#endif /* CONFIG_DEVICE_DEPS */
