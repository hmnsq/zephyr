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

정수 do_device_init(const struct device *dev)
{
	정수 rc = 0;

	만약 (dev->ops.init != NULL) {
		rc = dev->ops.init(dev);
		/* If initialization failed, record in dev->state->init_res
		 * the POSITIVE value of the resulting errno
		 */
		만약 (rc != 0) {
			/* device's init function should return:
			 *   0 on success
			 *   a negative value on failure (-errno)
			 * errno value maps to an uint8_t range as of now.
			 */
			__ASSERT(rc >= -UINT8_MAX && rc < 0, "device %s init: invalid error (%d)",
				 dev->name, rc);

			만약 (rc < 0) {
				rc = -rc;
			}
			/* handle error value overflow in production
			 * this is likely a bug in the device's init function. Signals it
			 */
			만약 (rc > UINT8_MAX) {
				rc = UINT8_MAX;
			}
		}

		/* Always record the result: a device that failed to initialize
		 * earlier must not keep that error once it initializes
		 * successfully.
		 */
		dev->state->init_res = rc;
	}

	만약 (rc == 0) {
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
	반환 -rc;
}

정수 z_impl_device_init(const struct device *dev)
{
	만약 (dev->state->initialized) {
		반환 -EALREADY;
	}

	반환 do_device_init(dev);
}

#ifdef CONFIG_USERSPACE
static inline 정수 z_vrfy_device_init(const struct device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	반환 z_impl_device_init(dev);
}
#include <zephyr/syscalls/device_init_mrsh.c>
#endif


const struct device *z_impl_device_get_binding(const char *name)
{
	/* A null string identifies no device.  So does an empty
	 * string.
	 */
	만약 ((name == NULL) || (name[0] == '\0')) {
		반환 NULL;
	}

	/* Return NULL if the device matching 'name' is not ready. */
	STRUCT_SECTION_FOREACH(device, dev) {
		만약 ((dev->name == name) || (strcmp(name, dev->name) == 0)) {
			반환 z_impl_device_is_ready(dev) ? dev : NULL;
		}
	}

	반환 NULL;
}

#ifdef CONFIG_USERSPACE
static inline const struct device *z_vrfy_device_get_binding(const char *name)
{
	char name_copy[Z_DEVICE_MAX_NAME_LEN];

	만약 (k_usermode_string_copy(name_copy, name, sizeof(name_copy))
	    != 0) {
		반환 NULL;
	}

	반환 z_impl_device_get_binding(name_copy);
}
#include <zephyr/syscalls/device_get_binding_mrsh.c>

static inline bool z_vrfy_device_is_ready(const struct device *dev)
{
	/* NULL is a documented argument, not an invalid kernel object */
	만약 (dev == NULL) {
		반환 false;
	}

	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	반환 z_impl_device_is_ready(dev);
}
#include <zephyr/syscalls/device_is_ready_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_DEVICE_DT_METADATA
const struct device *z_impl_device_get_by_dt_nodelabel(const char *nodelabel)
{
	/* For consistency with device_get_binding(). */
	만약 ((nodelabel == NULL) || (nodelabel[0] == '\0')) {
		반환 NULL;
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
		const struct device_dt_nodelabels *nl = device_get_dt_nodelabels(dev);

		만약 (!z_impl_device_is_ready(dev) || nl == NULL) {
			continue;
		}

		위해 (size_t i = 0; i < nl->num_nodelabels; i++) {
			const char *dev_nodelabel = nl->nodelabels[i];

			만약 (strcmp(nodelabel, dev_nodelabel) == 0) {
				반환 dev;
			}
		}
	}

	반환 NULL;
}

#ifdef CONFIG_USERSPACE
static inline const struct device *z_vrfy_device_get_by_dt_nodelabel(const char *nodelabel)
{
	char nl_copy[Z_DEVICE_MAX_NODELABEL_LEN];

	만약 (k_usermode_string_copy(nl_copy, (char *)nodelabel, sizeof(nl_copy)) != 0) {
		반환 NULL;
	}

	반환 z_impl_device_get_by_dt_nodelabel(nl_copy);
}
#include <zephyr/syscalls/device_get_by_dt_nodelabel_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_DEVICE_DT_METADATA */

size_t z_device_get_all_static(struct device const **devices)
{
	size_t cnt;

	STRUCT_SECTION_GET(device, 0, devices);
	STRUCT_SECTION_COUNT(device, &cnt);

	반환 cnt;
}

bool z_impl_device_is_ready(const struct device *dev)
{
	/*
	 * if an invalid device pointer is passed as argument, this call
	 * reports the `device` as not ready for usage.
	 */
	만약 (dev == NULL) {
		반환 false;
	}

	반환 dev->state->initialized;
}

정수 z_impl_device_deinit(const struct device *dev)
{
#ifdef CONFIG_DEVICE_DEINIT_SUPPORT
	정수 ret;

	만약 (!dev->state->initialized) {
		반환 -EPERM;
	}

	만약 (dev->ops.deinit == NULL) {
		반환 -ENOTSUP;
	}

	ret = dev->ops.deinit(dev);
	만약 (ret < 0) {
		반환 ret;
	}

	dev->state->initialized = false;

	반환 0;
#아니면
	ARG_UNUSED(dev);
	반환 -ENOTSUP;
#endif /* CONFIG_DEVICE_DEINIT_SUPPORT */
}

#ifdef CONFIG_USERSPACE
static inline 정수 z_vrfy_device_deinit(const struct device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	반환 z_impl_device_deinit(dev);
}
#include <zephyr/syscalls/device_deinit_mrsh.c>
#endif

#ifdef CONFIG_DEVICE_DEPS

static 정수 device_visitor(const device_handle_t *handles,
			   size_t handle_count,
			   device_visitor_callback_t visitor_cb,
			   void *context)
{
	/* Iterate over fixed devices */
	위해 (size_t i = 0; i < handle_count; ++i) {
		device_handle_t dh = handles[i];
		const struct device *rdev = device_from_handle(dh);
		정수 rc = visitor_cb(rdev, context);

		만약 (rc < 0) {
			반환 rc;
		}
	}

	반환 handle_count;
}

정수 device_required_foreach(const struct device *dev,
			    device_visitor_callback_t visitor_cb,
			    void *context)
{
	size_t handle_count = 0;
	const device_handle_t *handles = device_required_handles_get(dev, &handle_count);

	반환 device_visitor(handles, handle_count, visitor_cb, context);
}

정수 device_supported_foreach(const struct device *dev,
			     device_visitor_callback_t visitor_cb,
			     void *context)
{
	size_t handle_count = 0;
	const device_handle_t *handles = device_supported_handles_get(dev, &handle_count);

	반환 device_visitor(handles, handle_count, visitor_cb, context);
}

#endif /* CONFIG_DEVICE_DEPS */
