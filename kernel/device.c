/*
 * Copyright (c) 2015-2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <stddef.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <zephyr/device.h>
#અન્તર્ભાવ <zephyr/sys/atomic.h>
#અન્તર્ભાવ <zephyr/sys/iterable_sections.h>
#અન્તર્ભાવ <zephyr/sys/kobject.h>
#અન્તર્ભાવ <zephyr/internal/syscall_handler.h>
#અન્તર્ભાવ <zephyr/toolchain.h>
#અન્તર્ભાવ <zephyr/pm/device_runtime.h>

પૂર્ણાંક do_device_init(અચલ સંરચના device *dev)
{
	પૂર્ણાંક rc = 0;

	જો (dev->ops.init != ખાલી) {
		rc = dev->ops.init(dev);
		/* If initialization failed, record in dev->state->init_res
		 * the POSITIVE value of the resulting errno
		 */
		જો (rc != 0) {
			/* device's init function should return:
			 *   0 on success
			 *   a negative value on failure (-errno)
			 * errno value maps to an uint8_t range as of now.
			 */
			__ASSERT(rc >= -UINT8_MAX && rc < 0, "device %s init: invalid error (%d)",
				 dev->name, rc);

			જો (rc < 0) {
				rc = -rc;
			}
			/* handle error value overflow in production
			 * this is likely a bug in the device's init function. Signals it
			 */
			જો (rc > UINT8_MAX) {
				rc = UINT8_MAX;
			}
		}

		/* Always record the result: a device that failed to initialize
		 * earlier must not keep that error once it initializes
		 * successfully.
		 */
		dev->state->init_res = rc;
	}

	જો (rc == 0) {
		/* A failed init leaves the device uninitialized, so it can be
		 * retried by device_init() and nothing can be released by
		 * device_deinit().
		 */
		dev->state->initialized = true;

		/* Run automatic device runtime enablement */
		(શૂન્ય)pm_device_runtime_auto_enable(dev);
	}

	/* here, the value of rc is either 0 or +errno
	 * flip the sign to return a negative value on failure as expected
	 */
	પાછા -rc;
}

પૂર્ણાંક z_impl_device_init(અચલ સંરચના device *dev)
{
	જો (dev->state->initialized) {
		પાછા -EALREADY;
	}

	પાછા do_device_init(dev);
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ પૂર્ણાંક z_vrfy_device_init(અચલ સંરચના device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	પાછા z_impl_device_init(dev);
}
#અન્તર્ભાવ <zephyr/syscalls/device_init_mrsh.c>
#endif


અચલ સંરચના device *z_impl_device_get_binding(અચલ અક્ષર *name)
{
	/* A null string identifies no device.  So does an empty
	 * string.
	 */
	જો ((name == ખાલી) || (name[0] == '\0')) {
		પાછા ખાલી;
	}

	/* Return NULL if the device matching 'name' is not ready. */
	STRUCT_SECTION_FOREACH(device, dev) {
		જો ((dev->name == name) || (strcmp(name, dev->name) == 0)) {
			પાછા z_impl_device_is_ready(dev) ? dev : ખાલી;
		}
	}

	પાછા ખાલી;
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ અચલ સંરચના device *z_vrfy_device_get_binding(અચલ અક્ષર *name)
{
	અક્ષર name_copy[Z_DEVICE_MAX_NAME_LEN];

	જો (k_usermode_string_copy(name_copy, name, કદ(name_copy))
	    != 0) {
		પાછા ખાલી;
	}

	પાછા z_impl_device_get_binding(name_copy);
}
#અન્તર્ભાવ <zephyr/syscalls/device_get_binding_mrsh.c>

સ્થિર અંતઃરેખ bool z_vrfy_device_is_ready(અચલ સંરચના device *dev)
{
	/* NULL is a documented argument, not an invalid kernel object */
	જો (dev == ખાલી) {
		પાછા false;
	}

	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	પાછા z_impl_device_is_ready(dev);
}
#અન્તર્ભાવ <zephyr/syscalls/device_is_ready_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_DEVICE_DT_METADATA
અચલ સંરચના device *z_impl_device_get_by_dt_nodelabel(અચલ અક્ષર *nodelabel)
{
	/* For consistency with device_get_binding(). */
	જો ((nodelabel == ખાલી) || (nodelabel[0] == '\0')) {
		પાછા ખાલી;
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
		અચલ સંરચના device_dt_nodelabels *nl = device_get_dt_nodelabels(dev);

		જો (!z_impl_device_is_ready(dev) || nl == ખાલી) {
			ચાલુ;
		}

		માટે (size_t i = 0; i < nl->અંક_nodelabels; i++) {
			અચલ અક્ષર *dev_nodelabel = nl->nodelabels[i];

			જો (strcmp(nodelabel, dev_nodelabel) == 0) {
				પાછા dev;
			}
		}
	}

	પાછા ખાલી;
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ અચલ સંરચના device *z_vrfy_device_get_by_dt_nodelabel(અચલ અક્ષર *nodelabel)
{
	અક્ષર nl_copy[Z_DEVICE_MAX_NODELABEL_LEN];

	જો (k_usermode_string_copy(nl_copy, (અક્ષર *)nodelabel, કદ(nl_copy)) != 0) {
		પાછા ખાલી;
	}

	પાછા z_impl_device_get_by_dt_nodelabel(nl_copy);
}
#અન્તર્ભાવ <zephyr/syscalls/device_get_by_dt_nodelabel_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_DEVICE_DT_METADATA */

size_t z_device_get_all_static(સંરચના device અચલ **devices)
{
	size_t cnt;

	STRUCT_SECTION_GET(device, 0, devices);
	STRUCT_SECTION_ગણતરી(device, &cnt);

	પાછા cnt;
}

bool z_impl_device_is_ready(અચલ સંરચના device *dev)
{
	/*
	 * if an invalid device pointer is passed as argument, this call
	 * reports the `device` as not ready for usage.
	 */
	જો (dev == ખાલી) {
		પાછા false;
	}

	પાછા dev->state->initialized;
}

પૂર્ણાંક z_impl_device_deinit(અચલ સંરચના device *dev)
{
#ifdef CONFIG_DEVICE_DEINIT_SUPPORT
	પૂર્ણાંક ret;

	જો (!dev->state->initialized) {
		પાછા -EPERM;
	}

	જો (dev->ops.deinit == ખાલી) {
		પાછા -ENOTSUP;
	}

	ret = dev->ops.deinit(dev);
	જો (ret < 0) {
		પાછા ret;
	}

	dev->state->initialized = false;

	પાછા 0;
#નહીંતર
	ARG_UNUSED(dev);
	પાછા -ENOTSUP;
#endif /* CONFIG_DEVICE_DEINIT_SUPPORT */
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ પૂર્ણાંક z_vrfy_device_deinit(અચલ સંરચના device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	પાછા z_impl_device_deinit(dev);
}
#અન્તર્ભાવ <zephyr/syscalls/device_deinit_mrsh.c>
#endif

#ifdef CONFIG_DEVICE_DEPS

સ્થિર પૂર્ણાંક device_visitor(અચલ device_handle_t *handles,
			   size_t handle_ગણતરી,
			   device_visitor_callback_t visitor_cb,
			   શૂન્ય *context)
{
	/* Iterate over fixed devices */
	માટે (size_t i = 0; i < handle_ગણતરી; ++i) {
		device_handle_t dh = handles[i];
		અચલ સંરચના device *rdev = device_from_handle(dh);
		પૂર્ણાંક rc = visitor_cb(rdev, context);

		જો (rc < 0) {
			પાછા rc;
		}
	}

	પાછા handle_ગણતરી;
}

પૂર્ણાંક device_required_foreach(અચલ સંરચના device *dev,
			    device_visitor_callback_t visitor_cb,
			    શૂન્ય *context)
{
	size_t handle_ગણતરી = 0;
	અચલ device_handle_t *handles = device_required_handles_get(dev, &handle_ગણતરી);

	પાછા device_visitor(handles, handle_ગણતરી, visitor_cb, context);
}

પૂર્ણાંક device_supported_foreach(અચલ સંરચના device *dev,
			     device_visitor_callback_t visitor_cb,
			     શૂન્ય *context)
{
	size_t handle_ગણતરી = 0;
	અચલ device_handle_t *handles = device_supported_handles_get(dev, &handle_ગણતરી);

	પાછા device_visitor(handles, handle_ગણતરી, visitor_cb, context);
}

#endif /* CONFIG_DEVICE_DEPS */
