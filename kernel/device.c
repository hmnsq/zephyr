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

цяло do_device_init(const struct device *dev)
{
	цяло rc = 0;

	ако (dev->ops.init != NULL) {
		rc = dev->ops.init(dev);
		/* If initialization failed, record in dev->state->init_res
		 * the POSITIVE value of the resulting errno
		 */
		ако (rc != 0) {
			/* device's init function should return:
			 *   0 on success
			 *   a negative value on failure (-errno)
			 * errno value maps to an uint8_t range as of now.
			 */
			__ASSERT(rc >= -UINT8_MAX && rc < 0, "device %s init: invalid error (%d)",
				 dev->name, rc);

			ако (rc < 0) {
				rc = -rc;
			}
			/* handle error value overflow in production
			 * this is likely a bug in the device's init function. Signals it
			 */
			ако (rc > UINT8_MAX) {
				rc = UINT8_MAX;
			}
		}

		/* Always record the result: a device that failed to initialize
		 * earlier must not keep that error once it initializes
		 * successfully.
		 */
		dev->state->init_res = rc;
	}

	ако (rc == 0) {
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
	върни -rc;
}

цяло z_impl_device_init(const struct device *dev)
{
	ако (dev->state->initialized) {
		върни -EALREADY;
	}

	върни do_device_init(dev);
}

#ifdef CONFIG_USERSPACE
static inline цяло z_vrfy_device_init(const struct device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	върни z_impl_device_init(dev);
}
#include <zephyr/syscalls/device_init_mrsh.c>
#endif


const struct device *z_impl_device_get_binding(const char *name)
{
	/* A null string identifies no device.  So does an empty
	 * string.
	 */
	ако ((name == NULL) || (name[0] == '\0')) {
		върни NULL;
	}

	/* Return NULL if the device matching 'name' is not ready. */
	STRUCT_SECTION_FOREACH(device, dev) {
		ако ((dev->name == name) || (strcmp(name, dev->name) == 0)) {
			върни z_impl_device_is_ready(dev) ? dev : NULL;
		}
	}

	върни NULL;
}

#ifdef CONFIG_USERSPACE
static inline const struct device *z_vrfy_device_get_binding(const char *name)
{
	char name_copy[Z_DEVICE_MAX_NAME_LEN];

	ако (k_usermode_string_copy(name_copy, name, sizeof(name_copy))
	    != 0) {
		върни NULL;
	}

	върни z_impl_device_get_binding(name_copy);
}
#include <zephyr/syscalls/device_get_binding_mrsh.c>

static inline bool z_vrfy_device_is_ready(const struct device *dev)
{
	/* NULL is a documented argument, not an invalid kernel object */
	ако (dev == NULL) {
		върни false;
	}

	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	върни z_impl_device_is_ready(dev);
}
#include <zephyr/syscalls/device_is_ready_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_DEVICE_DT_METADATA
const struct device *z_impl_device_get_by_dt_nodelabel(const char *nodelabel)
{
	/* For consistency with device_get_binding(). */
	ако ((nodelabel == NULL) || (nodelabel[0] == '\0')) {
		върни NULL;
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

		ако (!z_impl_device_is_ready(dev) || nl == NULL) {
			continue;
		}

		за (size_t i = 0; i < nl->num_nodelabels; i++) {
			const char *dev_nodelabel = nl->nodelabels[i];

			ако (strcmp(nodelabel, dev_nodelabel) == 0) {
				върни dev;
			}
		}
	}

	върни NULL;
}

#ifdef CONFIG_USERSPACE
static inline const struct device *z_vrfy_device_get_by_dt_nodelabel(const char *nodelabel)
{
	char nl_copy[Z_DEVICE_MAX_NODELABEL_LEN];

	ако (k_usermode_string_copy(nl_copy, (char *)nodelabel, sizeof(nl_copy)) != 0) {
		върни NULL;
	}

	върни z_impl_device_get_by_dt_nodelabel(nl_copy);
}
#include <zephyr/syscalls/device_get_by_dt_nodelabel_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_DEVICE_DT_METADATA */

size_t z_device_get_all_static(struct device const **devices)
{
	size_t cnt;

	STRUCT_SECTION_GET(device, 0, devices);
	STRUCT_SECTION_COUNT(device, &cnt);

	върни cnt;
}

bool z_impl_device_is_ready(const struct device *dev)
{
	/*
	 * if an invalid device pointer is passed as argument, this call
	 * reports the `device` as not ready for usage.
	 */
	ако (dev == NULL) {
		върни false;
	}

	върни dev->state->initialized;
}

цяло z_impl_device_deinit(const struct device *dev)
{
#ifdef CONFIG_DEVICE_DEINIT_SUPPORT
	цяло ret;

	ако (!dev->state->initialized) {
		върни -EPERM;
	}

	ако (dev->ops.deinit == NULL) {
		върни -ENOTSUP;
	}

	ret = dev->ops.deinit(dev);
	ако (ret < 0) {
		върни ret;
	}

	dev->state->initialized = false;

	върни 0;
#иначе
	ARG_UNUSED(dev);
	върни -ENOTSUP;
#endif /* CONFIG_DEVICE_DEINIT_SUPPORT */
}

#ifdef CONFIG_USERSPACE
static inline цяло z_vrfy_device_deinit(const struct device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	върни z_impl_device_deinit(dev);
}
#include <zephyr/syscalls/device_deinit_mrsh.c>
#endif

#ifdef CONFIG_DEVICE_DEPS

static цяло device_visitor(const device_handle_t *handles,
			   size_t handle_count,
			   device_visitor_callback_t visitor_cb,
			   void *context)
{
	/* Iterate over fixed devices */
	за (size_t i = 0; i < handle_count; ++i) {
		device_handle_t dh = handles[i];
		const struct device *rdev = device_from_handle(dh);
		цяло rc = visitor_cb(rdev, context);

		ако (rc < 0) {
			върни rc;
		}
	}

	върни handle_count;
}

цяло device_required_foreach(const struct device *dev,
			    device_visitor_callback_t visitor_cb,
			    void *context)
{
	size_t handle_count = 0;
	const device_handle_t *handles = device_required_handles_get(dev, &handle_count);

	върни device_visitor(handles, handle_count, visitor_cb, context);
}

цяло device_supported_foreach(const struct device *dev,
			     device_visitor_callback_t visitor_cb,
			     void *context)
{
	size_t handle_count = 0;
	const device_handle_t *handles = device_supported_handles_get(dev, &handle_count);

	върни device_visitor(handles, handle_count, visitor_cb, context);
}

#endif /* CONFIG_DEVICE_DEPS */
