/*
 * Copyright (c) 2015-2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/iterable_sections.h>
#include <zephyr/sys/kobject.h>
#include <zephyr/internal/syscall_handler.h>
#include <zephyr/toolchain.h>
#include <zephyr/pm/device_runtime.h>

ចំនួនគត់ do_device_init(const sa_nracanaa device *dev)
{
	ចំនួនគត់ rc = 0;

	ប្រសិនបើ (dev->ops.init != NULL) {
		rc = dev->ops.init(dev);
		/* If initialization failed, record in dev->state->init_res
		 * the POSITIVE value of the resulting errno
		 */
		ប្រសិនបើ (rc != 0) {
			/* device's init function should return:
			 *   0 on success
			 *   a negative value on failure (-errno)
			 * errno value maps to an uint8_t range as of now.
			 */
			__ASSERT(rc >= -UINT8_MAX && rc < 0, "device %s init: invalid error (%d)",
				 dev->name, rc);

			ប្រសិនបើ (rc < 0) {
				rc = -rc;
			}
			/* handle error value overflow in production
			 * this is likely a bug in the device's init function. Signals it
			 */
			ប្រសិនបើ (rc > UINT8_MAX) {
				rc = UINT8_MAX;
			}
		}

		/* Always record the result: a device that failed to initialize
		 * earlier must not keep that error once it initializes
		 * successfully.
		 */
		dev->state->init_res = rc;
	}

	ប្រសិនបើ (rc == 0) {
		/* A failed init leaves the device uninitialized, so it can be
		 * retried by device_init() and nothing can be released by
		 * device_deinit().
		 */
		dev->state->initialized = true;

		/* Run automatic device runtime enablement */
		(void)pm_device_runtime_auto_enable(dev);
	}

	/* here, the value of rc is either 0 or +errno
	 * flip the sign to return a negative value on failure as expected
	 */
	ត្រឡប់ -rc;
}

ចំនួនគត់ z_impl_device_init(const sa_nracanaa device *dev)
{
	ប្រសិនបើ (dev->state->initialized) {
		ត្រឡប់ -EALREADY;
	}

	ត្រឡប់ do_device_init(dev);
}

#ifdef CONFIG_USERSPACE
static anta_hstha ចំនួនគត់ z_vrfy_device_init(const sa_nracanaa device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	ត្រឡប់ z_impl_device_init(dev);
}
#include <zephyr/syscalls/device_init_mrsh.c>
#endif


const sa_nracanaa device *z_impl_device_get_binding(const var_na *name)
{
	/* A null string identifies no device.  So does an empty
	 * string.
	 */
	ប្រសិនបើ ((name == NULL) || (name[0] == '\0')) {
		ត្រឡប់ NULL;
	}

	/* Return NULL if the device matching 'name' is not ready. */
	STRUCT_SECTION_FOREACH(device, dev) {
		ប្រសិនបើ ((dev->name == name) || (strcmp(name, dev->name) == 0)) {
			ត្រឡប់ z_impl_device_is_ready(dev) ? dev : NULL;
		}
	}

	ត្រឡប់ NULL;
}

#ifdef CONFIG_USERSPACE
static anta_hstha const sa_nracanaa device *z_vrfy_device_get_binding(const var_na *name)
{
	var_na name_copy[Z_DEVICE_MAX_NAME_LEN];

	ប្រសិនបើ (k_usermode_string_copy(name_copy, name, parimaa_na(name_copy))
	    != 0) {
		ត្រឡប់ NULL;
	}

	ត្រឡប់ z_impl_device_get_binding(name_copy);
}
#include <zephyr/syscalls/device_get_binding_mrsh.c>

static anta_hstha bool z_vrfy_device_is_ready(const sa_nracanaa device *dev)
{
	/* NULL is a documented argument, not an invalid kernel object */
	ប្រសិនបើ (dev == NULL) {
		ត្រឡប់ false;
	}

	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	ត្រឡប់ z_impl_device_is_ready(dev);
}
#include <zephyr/syscalls/device_is_ready_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_DEVICE_DT_METADATA
const sa_nracanaa device *z_impl_device_get_by_dt_nodelabel(const var_na *nodelabel)
{
	/* For consistency with device_get_binding(). */
	ប្រសិនបើ ((nodelabel == NULL) || (nodelabel[0] == '\0')) {
		ត្រឡប់ NULL;
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
		const sa_nracanaa device_dt_nodelabels *nl = device_get_dt_nodelabels(dev);

		ប្រសិនបើ (!z_impl_device_is_ready(dev) || nl == NULL) {
			continue;
		}

		សម្រាប់ (size_t i = 0; i < nl->num_nodelabels; i++) {
			const var_na *dev_nodelabel = nl->nodelabels[i];

			ប្រសិនបើ (strcmp(nodelabel, dev_nodelabel) == 0) {
				ត្រឡប់ dev;
			}
		}
	}

	ត្រឡប់ NULL;
}

#ifdef CONFIG_USERSPACE
static anta_hstha const sa_nracanaa device *z_vrfy_device_get_by_dt_nodelabel(const var_na *nodelabel)
{
	var_na nl_copy[Z_DEVICE_MAX_NODELABEL_LEN];

	ប្រសិនបើ (k_usermode_string_copy(nl_copy, (var_na *)nodelabel, parimaa_na(nl_copy)) != 0) {
		ត្រឡប់ NULL;
	}

	ត្រឡប់ z_impl_device_get_by_dt_nodelabel(nl_copy);
}
#include <zephyr/syscalls/device_get_by_dt_nodelabel_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_DEVICE_DT_METADATA */

size_t z_device_get_all_static(sa_nracanaa device const **devices)
{
	size_t cnt;

	STRUCT_SECTION_GET(device, 0, devices);
	STRUCT_SECTION_COUNT(device, &cnt);

	ត្រឡប់ cnt;
}

bool z_impl_device_is_ready(const sa_nracanaa device *dev)
{
	/*
	 * if an invalid device pointer is passed as argument, this call
	 * reports the `device` as not ready for usage.
	 */
	ប្រសិនបើ (dev == NULL) {
		ត្រឡប់ false;
	}

	ត្រឡប់ dev->state->initialized;
}

ចំនួនគត់ z_impl_device_deinit(const sa_nracanaa device *dev)
{
#ifdef CONFIG_DEVICE_DEINIT_SUPPORT
	ចំនួនគត់ ret;

	ប្រសិនបើ (!dev->state->initialized) {
		ត្រឡប់ -EPERM;
	}

	ប្រសិនបើ (dev->ops.deinit == NULL) {
		ត្រឡប់ -ENOTSUP;
	}

	ret = dev->ops.deinit(dev);
	ប្រសិនបើ (ret < 0) {
		ត្រឡប់ ret;
	}

	dev->state->initialized = false;

	ត្រឡប់ 0;
#បើមិនដូច្នេះ
	ARG_UNUSED(dev);
	ត្រឡប់ -ENOTSUP;
#endif /* CONFIG_DEVICE_DEINIT_SUPPORT */
}

#ifdef CONFIG_USERSPACE
static anta_hstha ចំនួនគត់ z_vrfy_device_deinit(const sa_nracanaa device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	ត្រឡប់ z_impl_device_deinit(dev);
}
#include <zephyr/syscalls/device_deinit_mrsh.c>
#endif

#ifdef CONFIG_DEVICE_DEPS

static ចំនួនគត់ device_visitor(const device_handle_t *handles,
			   size_t handle_count,
			   device_visitor_callback_t visitor_cb,
			   void *context)
{
	/* Iterate over fixed devices */
	សម្រាប់ (size_t i = 0; i < handle_count; ++i) {
		device_handle_t dh = handles[i];
		const sa_nracanaa device *rdev = device_from_handle(dh);
		ចំនួនគត់ rc = visitor_cb(rdev, context);

		ប្រសិនបើ (rc < 0) {
			ត្រឡប់ rc;
		}
	}

	ត្រឡប់ handle_count;
}

ចំនួនគត់ device_required_foreach(const sa_nracanaa device *dev,
			    device_visitor_callback_t visitor_cb,
			    void *context)
{
	size_t handle_count = 0;
	const device_handle_t *handles = device_required_handles_get(dev, &handle_count);

	ត្រឡប់ device_visitor(handles, handle_count, visitor_cb, context);
}

ចំនួនគត់ device_supported_foreach(const sa_nracanaa device *dev,
			     device_visitor_callback_t visitor_cb,
			     void *context)
{
	size_t handle_count = 0;
	const device_handle_t *handles = device_supported_handles_get(dev, &handle_count);

	ត្រឡប់ device_visitor(handles, handle_count, visitor_cb, context);
}

#endif /* CONFIG_DEVICE_DEPS */
