/*
 * Copyright (c) 2015-2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <zephyr/device.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/atomic.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/iterable_sections.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/kobject.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>
#ಅನ್ತರ್ಭಾವ <zephyr/toolchain.h>
#ಅನ್ತರ್ಭಾವ <zephyr/pm/device_runtime.h>

ಪೂರ್ಣಾಙ್ಕ do_device_init(ಧ್ರುವ ಸಂರಚನಾ device *dev)
{
	ಪೂರ್ಣಾಙ್ಕ rc = 0;

	ಯದಿ (dev->ops.init != ರಿಕ್ತ) {
		rc = dev->ops.init(dev);
		/* If initialization failed, record in dev->state->init_res
		 * the POSITIVE value of the resulting errno
		 */
		ಯದಿ (rc != 0) {
			/* device's init function should return:
			 *   0 on success
			 *   a negative value on failure (-errno)
			 * errno value maps to an uint8_t range as of now.
			 */
			__ASSERT(rc >= -UINT8_MAX && rc < 0, "device %s init: invalid error (%d)",
				 dev->name, rc);

			ಯದಿ (rc < 0) {
				rc = -rc;
			}
			/* handle error value overflow in production
			 * this is likely a bug in the device's init function. Signals it
			 */
			ಯದಿ (rc > UINT8_MAX) {
				rc = UINT8_MAX;
			}
		}

		/* Always record the result: a device that failed to initialize
		 * earlier must not keep that error once it initializes
		 * successfully.
		 */
		dev->state->init_res = rc;
	}

	ಯದಿ (rc == 0) {
		/* A failed init leaves the device uninitialized, so it can be
		 * retried by device_init() and nothing can be released by
		 * device_deinit().
		 */
		dev->state->initialized = true;

		/* Run automatic device runtime enablement */
		(ಶೂನ್ಯ)pm_device_runtime_auto_enable(dev);
	}

	/* here, the value of rc is either 0 or +errno
	 * flip the sign to return a negative value on failure as expected
	 */
	ಪ್ರತ್ಯಾಗಮ -rc;
}

ಪೂರ್ಣಾಙ್ಕ z_impl_device_init(ಧ್ರುವ ಸಂರಚನಾ device *dev)
{
	ಯದಿ (dev->state->initialized) {
		ಪ್ರತ್ಯಾಗಮ -EALREADY;
	}

	ಪ್ರತ್ಯಾಗಮ do_device_init(dev);
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ z_vrfy_device_init(ಧ್ರುವ ಸಂರಚನಾ device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	ಪ್ರತ್ಯಾಗಮ z_impl_device_init(dev);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/device_init_mrsh.c>
#endif


ಧ್ರುವ ಸಂರಚನಾ device *z_impl_device_get_binding(ಧ್ರುವ ವರ್ಣ *name)
{
	/* A null string identifies no device.  So does an empty
	 * string.
	 */
	ಯದಿ ((name == ರಿಕ್ತ) || (name[0] == '\0')) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	/* Return NULL if the device matching 'name' is not ready. */
	STRUCT_SECTION_FOREACH(device, dev) {
		ಯದಿ ((dev->name == name) || (strcmp(name, dev->name) == 0)) {
			ಪ್ರತ್ಯಾಗಮ z_impl_device_is_ready(dev) ? dev : ರಿಕ್ತ;
		}
	}

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಧ್ರುವ ಸಂರಚನಾ device *z_vrfy_device_get_binding(ಧ್ರುವ ವರ್ಣ *name)
{
	ವರ್ಣ name_copy[Z_DEVICE_MAX_NAME_LEN];

	ಯದಿ (k_usermode_string_copy(name_copy, name, ಪರಿಮಾಣ(name_copy))
	    != 0) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ z_impl_device_get_binding(name_copy);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/device_get_binding_mrsh.c>

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_vrfy_device_is_ready(ಧ್ರುವ ಸಂರಚನಾ device *dev)
{
	/* NULL is a documented argument, not an invalid kernel object */
	ಯದಿ (dev == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	ಪ್ರತ್ಯಾಗಮ z_impl_device_is_ready(dev);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/device_is_ready_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_DEVICE_DT_METADATA
ಧ್ರುವ ಸಂರಚನಾ device *z_impl_device_get_by_dt_nodelabel(ಧ್ರುವ ವರ್ಣ *nodelabel)
{
	/* For consistency with device_get_binding(). */
	ಯದಿ ((nodelabel == ರಿಕ್ತ) || (nodelabel[0] == '\0')) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
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
		ಧ್ರುವ ಸಂರಚನಾ device_dt_nodelabels *nl = device_get_dt_nodelabels(dev);

		ಯದಿ (!z_impl_device_is_ready(dev) || nl == ರಿಕ್ತ) {
			ಅನುವರ್ತ;
		}

		ಪ್ರತಿ (size_t i = 0; i < nl->ಅಂಕೆ_nodelabels; i++) {
			ಧ್ರುವ ವರ್ಣ *dev_nodelabel = nl->nodelabels[i];

			ಯದಿ (strcmp(nodelabel, dev_nodelabel) == 0) {
				ಪ್ರತ್ಯಾಗಮ dev;
			}
		}
	}

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಧ್ರುವ ಸಂರಚನಾ device *z_vrfy_device_get_by_dt_nodelabel(ಧ್ರುವ ವರ್ಣ *nodelabel)
{
	ವರ್ಣ nl_copy[Z_DEVICE_MAX_NODELABEL_LEN];

	ಯದಿ (k_usermode_string_copy(nl_copy, (ವರ್ಣ *)nodelabel, ಪರಿಮಾಣ(nl_copy)) != 0) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ z_impl_device_get_by_dt_nodelabel(nl_copy);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/device_get_by_dt_nodelabel_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_DEVICE_DT_METADATA */

size_t z_device_get_all_static(ಸಂರಚನಾ device ಧ್ರುವ **devices)
{
	size_t cnt;

	STRUCT_SECTION_GET(device, 0, devices);
	STRUCT_SECTION_ಎಣಿಕೆ(device, &cnt);

	ಪ್ರತ್ಯಾಗಮ cnt;
}

bool z_impl_device_is_ready(ಧ್ರುವ ಸಂರಚನಾ device *dev)
{
	/*
	 * if an invalid device pointer is passed as argument, this call
	 * reports the `device` as not ready for usage.
	 */
	ಯದಿ (dev == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಪ್ರತ್ಯಾಗಮ dev->state->initialized;
}

ಪೂರ್ಣಾಙ್ಕ z_impl_device_deinit(ಧ್ರುವ ಸಂರಚನಾ device *dev)
{
#ifdef CONFIG_DEVICE_DEINIT_SUPPORT
	ಪೂರ್ಣಾಙ್ಕ ret;

	ಯದಿ (!dev->state->initialized) {
		ಪ್ರತ್ಯಾಗಮ -EPERM;
	}

	ಯದಿ (dev->ops.deinit == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ -ENOTSUP;
	}

	ret = dev->ops.deinit(dev);
	ಯದಿ (ret < 0) {
		ಪ್ರತ್ಯಾಗಮ ret;
	}

	dev->state->initialized = false;

	ಪ್ರತ್ಯಾಗಮ 0;
#ಅನ್ಯಥಾ
	ARG_UNUSED(dev);
	ಪ್ರತ್ಯಾಗಮ -ENOTSUP;
#endif /* CONFIG_DEVICE_DEINIT_SUPPORT */
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ z_vrfy_device_deinit(ಧ್ರುವ ಸಂರಚನಾ device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	ಪ್ರತ್ಯಾಗಮ z_impl_device_deinit(dev);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/device_deinit_mrsh.c>
#endif

#ifdef CONFIG_DEVICE_DEPS

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ device_visitor(ಧ್ರುವ device_handle_t *handles,
			   size_t handle_ಎಣಿಕೆ,
			   device_visitor_callback_t visitor_cb,
			   ಶೂನ್ಯ *context)
{
	/* Iterate over fixed devices */
	ಪ್ರತಿ (size_t i = 0; i < handle_ಎಣಿಕೆ; ++i) {
		device_handle_t dh = handles[i];
		ಧ್ರುವ ಸಂರಚನಾ device *rdev = device_from_handle(dh);
		ಪೂರ್ಣಾಙ್ಕ rc = visitor_cb(rdev, context);

		ಯದಿ (rc < 0) {
			ಪ್ರತ್ಯಾಗಮ rc;
		}
	}

	ಪ್ರತ್ಯಾಗಮ handle_ಎಣಿಕೆ;
}

ಪೂರ್ಣಾಙ್ಕ device_required_foreach(ಧ್ರುವ ಸಂರಚನಾ device *dev,
			    device_visitor_callback_t visitor_cb,
			    ಶೂನ್ಯ *context)
{
	size_t handle_ಎಣಿಕೆ = 0;
	ಧ್ರುವ device_handle_t *handles = device_required_handles_get(dev, &handle_ಎಣಿಕೆ);

	ಪ್ರತ್ಯಾಗಮ device_visitor(handles, handle_ಎಣಿಕೆ, visitor_cb, context);
}

ಪೂರ್ಣಾಙ್ಕ device_supported_foreach(ಧ್ರುವ ಸಂರಚನಾ device *dev,
			     device_visitor_callback_t visitor_cb,
			     ಶೂನ್ಯ *context)
{
	size_t handle_ಎಣಿಕೆ = 0;
	ಧ್ರುವ device_handle_t *handles = device_supported_handles_get(dev, &handle_ಎಣಿಕೆ);

	ಪ್ರತ್ಯಾಗಮ device_visitor(handles, handle_ಎಣಿಕೆ, visitor_cb, context);
}

#endif /* CONFIG_DEVICE_DEPS */
