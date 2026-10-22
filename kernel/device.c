/*
 * Copyright (c) 2015-2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <errno.h>
#ضمن <stddef.h>
#ضمن <string.h>
#ضمن <zephyr/device.h>
#ضمن <zephyr/sys/atomic.h>
#ضمن <zephyr/sys/iterable_sections.h>
#ضمن <zephyr/sys/kobject.h>
#ضمن <zephyr/internal/syscall_handler.h>
#ضمن <zephyr/toolchain.h>
#ضمن <zephyr/pm/device_runtime.h>

صحيح do_device_init(ثابت بنية device *dev)
{
	صحيح rc = 0;

	إذا (dev->ops.init != عدم) {
		rc = dev->ops.init(dev);
		/* If initialization failed, record in dev->state->init_res
		 * the POSITIVE value of the resulting errno
		 */
		إذا (rc != 0) {
			/* device's init function should return:
			 *   0 on success
			 *   a negative value on failure (-errno)
			 * errno value maps to an uint8_t range as of now.
			 */
			__ASSERT(rc >= -UINT8_MAX && rc < 0, "device %s init: invalid error (%d)",
				 dev->name, rc);

			إذا (rc < 0) {
				rc = -rc;
			}
			/* handle error value overflow in production
			 * this is likely a bug in the device's init function. Signals it
			 */
			إذا (rc > UINT8_MAX) {
				rc = UINT8_MAX;
			}
		}

		/* Always record the result: a device that failed to initialize
		 * earlier must not keep that error once it initializes
		 * successfully.
		 */
		dev->state->init_res = rc;
	}

	إذا (rc == 0) {
		/* A failed init leaves the device uninitialized, so it can be
		 * retried by device_init() and nothing can be released by
		 * device_deinit().
		 */
		dev->state->initialized = true;

		/* Run automatic device runtime enablement */
		(فراغ)pm_device_runtime_auto_enable(dev);
	}

	/* here, the value of rc is either 0 or +errno
	 * flip the sign to return a negative value on failure as expected
	 */
	أرجع -rc;
}

صحيح z_impl_device_init(ثابت بنية device *dev)
{
	إذا (dev->state->initialized) {
		أرجع -EALREADY;
	}

	أرجع do_device_init(dev);
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_device_init(ثابت بنية device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	أرجع z_impl_device_init(dev);
}
#ضمن <zephyr/syscalls/device_init_mrsh.c>
#جيڪڏهن_پورو


ثابت بنية device *z_impl_device_get_binding(ثابت حرف *name)
{
	/* A null string identifies no device.  So does an empty
	 * string.
	 */
	إذا ((name == عدم) || (name[0] == '\0')) {
		أرجع عدم;
	}

	/* Return NULL if the device matching 'name' is not ready. */
	STRUCT_SECTION_FOREACH(device, dev) {
		إذا ((dev->name == name) || (strcmp(name, dev->name) == 0)) {
			أرجع z_impl_device_is_ready(dev) ? dev : عدم;
		}
	}

	أرجع عدم;
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن ثابت بنية device *z_vrfy_device_get_binding(ثابت حرف *name)
{
	حرف name_copy[Z_DEVICE_MAX_NAME_LEN];

	إذا (k_usermode_string_copy(name_copy, name, حجم(name_copy))
	    != 0) {
		أرجع عدم;
	}

	أرجع z_impl_device_get_binding(name_copy);
}
#ضمن <zephyr/syscalls/device_get_binding_mrsh.c>

ساكن مضمن bool z_vrfy_device_is_ready(ثابت بنية device *dev)
{
	/* NULL is a documented argument, not an invalid kernel object */
	إذا (dev == عدم) {
		أرجع false;
	}

	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	أرجع z_impl_device_is_ready(dev);
}
#ضمن <zephyr/syscalls/device_is_ready_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

#جيڪڏهن_وصف_ٿيل CONFIG_DEVICE_DT_METADATA
ثابت بنية device *z_impl_device_get_by_dt_nodelabel(ثابت حرف *nodelabel)
{
	/* For consistency with device_get_binding(). */
	إذا ((nodelabel == عدم) || (nodelabel[0] == '\0')) {
		أرجع عدم;
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
		ثابت بنية device_dt_nodelabels *nl = device_get_dt_nodelabels(dev);

		إذا (!z_impl_device_is_ready(dev) || nl == عدم) {
			استمر;
		}

		لكل (size_t i = 0; i < nl->انگ_nodelabels; i++) {
			ثابت حرف *dev_nodelabel = nl->nodelabels[i];

			إذا (strcmp(nodelabel, dev_nodelabel) == 0) {
				أرجع dev;
			}
		}
	}

	أرجع عدم;
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن ثابت بنية device *z_vrfy_device_get_by_dt_nodelabel(ثابت حرف *nodelabel)
{
	حرف nl_copy[Z_DEVICE_MAX_NODELABEL_LEN];

	إذا (k_usermode_string_copy(nl_copy, (حرف *)nodelabel, حجم(nl_copy)) != 0) {
		أرجع عدم;
	}

	أرجع z_impl_device_get_by_dt_nodelabel(nl_copy);
}
#ضمن <zephyr/syscalls/device_get_by_dt_nodelabel_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */
#جيڪڏهن_پورو /* CONFIG_DEVICE_DT_METADATA */

size_t z_device_get_all_static(بنية device ثابت **devices)
{
	size_t cnt;

	STRUCT_SECTION_GET(device, 0, devices);
	STRUCT_SECTION_ڳڻپ(device, &cnt);

	أرجع cnt;
}

bool z_impl_device_is_ready(ثابت بنية device *dev)
{
	/*
	 * if an invalid device pointer is passed as argument, this call
	 * reports the `device` as not ready for usage.
	 */
	إذا (dev == عدم) {
		أرجع false;
	}

	أرجع dev->state->initialized;
}

صحيح z_impl_device_deinit(ثابت بنية device *dev)
{
#جيڪڏهن_وصف_ٿيل CONFIG_DEVICE_DEINIT_SUPPORT
	صحيح ret;

	إذا (!dev->state->initialized) {
		أرجع -EPERM;
	}

	إذا (dev->ops.deinit == عدم) {
		أرجع -ENOTSUP;
	}

	ret = dev->ops.deinit(dev);
	إذا (ret < 0) {
		أرجع ret;
	}

	dev->state->initialized = false;

	أرجع 0;
#وإلا
	ARG_اڻ_استعمال_ٿيل(dev);
	أرجع -ENOTSUP;
#جيڪڏهن_پورو /* CONFIG_DEVICE_DEINIT_SUPPORT */
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_device_deinit(ثابت بنية device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	أرجع z_impl_device_deinit(dev);
}
#ضمن <zephyr/syscalls/device_deinit_mrsh.c>
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل CONFIG_DEVICE_DEPS

ساكن صحيح device_visitor(ثابت device_handle_t *handles,
			   size_t handle_ڳڻپ,
			   device_visitor_callback_t visitor_cb,
			   فراغ *context)
{
	/* Iterate over fixed devices */
	لكل (size_t i = 0; i < handle_ڳڻپ; ++i) {
		device_handle_t dh = handles[i];
		ثابت بنية device *rdev = device_from_handle(dh);
		صحيح rc = visitor_cb(rdev, context);

		إذا (rc < 0) {
			أرجع rc;
		}
	}

	أرجع handle_ڳڻپ;
}

صحيح device_required_foreach(ثابت بنية device *dev,
			    device_visitor_callback_t visitor_cb,
			    فراغ *context)
{
	size_t handle_ڳڻپ = 0;
	ثابت device_handle_t *handles = device_required_handles_get(dev, &handle_ڳڻپ);

	أرجع device_visitor(handles, handle_ڳڻپ, visitor_cb, context);
}

صحيح device_supported_foreach(ثابت بنية device *dev,
			     device_visitor_callback_t visitor_cb,
			     فراغ *context)
{
	size_t handle_ڳڻپ = 0;
	ثابت device_handle_t *handles = device_supported_handles_get(dev, &handle_ڳڻپ);

	أرجع device_visitor(handles, handle_ڳڻپ, visitor_cb, context);
}

#جيڪڏهن_پورو /* CONFIG_DEVICE_DEPS */
