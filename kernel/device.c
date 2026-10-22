/*
 * Copyright (c) 2015-2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <stddef.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <zephyr/device.h>
#അന്തര്ഭാവ <zephyr/sys/atomic.h>
#അന്തര്ഭാവ <zephyr/sys/iterable_sections.h>
#അന്തര്ഭാവ <zephyr/sys/kobject.h>
#അന്തര്ഭാവ <zephyr/internal/syscall_handler.h>
#അന്തര്ഭാവ <zephyr/toolchain.h>
#അന്തര്ഭാവ <zephyr/pm/device_runtime.h>

പൂർണ്ണസംഖ്യ do_device_init(സ്ഥിരം ഘടന device *dev)
{
	പൂർണ്ണസംഖ്യ rc = 0;

	എങ്കിൽ (dev->ops.init != ശൂന്യ) {
		rc = dev->ops.init(dev);
		/* If initialization failed, record in dev->state->init_res
		 * the POSITIVE value of the resulting errno
		 */
		എങ്കിൽ (rc != 0) {
			/* device's init function should return:
			 *   0 on success
			 *   a negative value on failure (-errno)
			 * errno value maps to an uint8_t range as of now.
			 */
			__ASSERT(rc >= -UINT8_MAX && rc < 0, "device %s init: invalid error (%d)",
				 dev->name, rc);

			എങ്കിൽ (rc < 0) {
				rc = -rc;
			}
			/* handle error value overflow in production
			 * this is likely a bug in the device's init function. Signals it
			 */
			എങ്കിൽ (rc > UINT8_MAX) {
				rc = UINT8_MAX;
			}
		}

		/* Always record the result: a device that failed to initialize
		 * earlier must not keep that error once it initializes
		 * successfully.
		 */
		dev->state->init_res = rc;
	}

	എങ്കിൽ (rc == 0) {
		/* A failed init leaves the device uninitialized, so it can be
		 * retried by device_init() and nothing can be released by
		 * device_deinit().
		 */
		dev->state->initialized = true;

		/* Run automatic device runtime enablement */
		(ശൂന്യം)pm_device_runtime_auto_enable(dev);
	}

	/* here, the value of rc is either 0 or +errno
	 * flip the sign to return a negative value on failure as expected
	 */
	തിരികെ -rc;
}

പൂർണ്ണസംഖ്യ z_impl_device_init(സ്ഥിരം ഘടന device *dev)
{
	എങ്കിൽ (dev->state->initialized) {
		തിരികെ -EALREADY;
	}

	തിരികെ do_device_init(dev);
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ z_vrfy_device_init(സ്ഥിരം ഘടന device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	തിരികെ z_impl_device_init(dev);
}
#അന്തര്ഭാവ <zephyr/syscalls/device_init_mrsh.c>
#endif


സ്ഥിരം ഘടന device *z_impl_device_get_binding(സ്ഥിരം അക്ഷരം *name)
{
	/* A null string identifies no device.  So does an empty
	 * string.
	 */
	എങ്കിൽ ((name == ശൂന്യ) || (name[0] == '\0')) {
		തിരികെ ശൂന്യ;
	}

	/* Return NULL if the device matching 'name' is not ready. */
	STRUCT_SECTION_FOREACH(device, dev) {
		എങ്കിൽ ((dev->name == name) || (strcmp(name, dev->name) == 0)) {
			തിരികെ z_impl_device_is_ready(dev) ? dev : ശൂന്യ;
		}
	}

	തിരികെ ശൂന്യ;
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ സ്ഥിരം ഘടന device *z_vrfy_device_get_binding(സ്ഥിരം അക്ഷരം *name)
{
	അക്ഷരം name_copy[Z_DEVICE_MAX_NAME_LEN];

	എങ്കിൽ (k_usermode_string_copy(name_copy, name, വലുപ്പം(name_copy))
	    != 0) {
		തിരികെ ശൂന്യ;
	}

	തിരികെ z_impl_device_get_binding(name_copy);
}
#അന്തര്ഭാവ <zephyr/syscalls/device_get_binding_mrsh.c>

സ്ഥിതം വരിയിൽ bool z_vrfy_device_is_ready(സ്ഥിരം ഘടന device *dev)
{
	/* NULL is a documented argument, not an invalid kernel object */
	എങ്കിൽ (dev == ശൂന്യ) {
		തിരികെ false;
	}

	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	തിരികെ z_impl_device_is_ready(dev);
}
#അന്തര്ഭാവ <zephyr/syscalls/device_is_ready_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_DEVICE_DT_METADATA
സ്ഥിരം ഘടന device *z_impl_device_get_by_dt_nodelabel(സ്ഥിരം അക്ഷരം *nodelabel)
{
	/* For consistency with device_get_binding(). */
	എങ്കിൽ ((nodelabel == ശൂന്യ) || (nodelabel[0] == '\0')) {
		തിരികെ ശൂന്യ;
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
		സ്ഥിരം ഘടന device_dt_nodelabels *nl = device_get_dt_nodelabels(dev);

		എങ്കിൽ (!z_impl_device_is_ready(dev) || nl == ശൂന്യ) {
			തുടരുക;
		}

		വേണ്ടി (size_t i = 0; i < nl->അക്കം_nodelabels; i++) {
			സ്ഥിരം അക്ഷരം *dev_nodelabel = nl->nodelabels[i];

			എങ്കിൽ (strcmp(nodelabel, dev_nodelabel) == 0) {
				തിരികെ dev;
			}
		}
	}

	തിരികെ ശൂന്യ;
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ സ്ഥിരം ഘടന device *z_vrfy_device_get_by_dt_nodelabel(സ്ഥിരം അക്ഷരം *nodelabel)
{
	അക്ഷരം nl_copy[Z_DEVICE_MAX_NODELABEL_LEN];

	എങ്കിൽ (k_usermode_string_copy(nl_copy, (അക്ഷരം *)nodelabel, വലുപ്പം(nl_copy)) != 0) {
		തിരികെ ശൂന്യ;
	}

	തിരികെ z_impl_device_get_by_dt_nodelabel(nl_copy);
}
#അന്തര്ഭാവ <zephyr/syscalls/device_get_by_dt_nodelabel_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_DEVICE_DT_METADATA */

size_t z_device_get_all_static(ഘടന device സ്ഥിരം **devices)
{
	size_t cnt;

	STRUCT_SECTION_GET(device, 0, devices);
	STRUCT_SECTION_എണ്ണം(device, &cnt);

	തിരികെ cnt;
}

bool z_impl_device_is_ready(സ്ഥിരം ഘടന device *dev)
{
	/*
	 * if an invalid device pointer is passed as argument, this call
	 * reports the `device` as not ready for usage.
	 */
	എങ്കിൽ (dev == ശൂന്യ) {
		തിരികെ false;
	}

	തിരികെ dev->state->initialized;
}

പൂർണ്ണസംഖ്യ z_impl_device_deinit(സ്ഥിരം ഘടന device *dev)
{
#ifdef CONFIG_DEVICE_DEINIT_SUPPORT
	പൂർണ്ണസംഖ്യ ret;

	എങ്കിൽ (!dev->state->initialized) {
		തിരികെ -EPERM;
	}

	എങ്കിൽ (dev->ops.deinit == ശൂന്യ) {
		തിരികെ -ENOTSUP;
	}

	ret = dev->ops.deinit(dev);
	എങ്കിൽ (ret < 0) {
		തിരികെ ret;
	}

	dev->state->initialized = false;

	തിരികെ 0;
#അല്ലെങ്കിൽ
	ARG_UNUSED(dev);
	തിരികെ -ENOTSUP;
#endif /* CONFIG_DEVICE_DEINIT_SUPPORT */
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ z_vrfy_device_deinit(സ്ഥിരം ഘടന device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	തിരികെ z_impl_device_deinit(dev);
}
#അന്തര്ഭാവ <zephyr/syscalls/device_deinit_mrsh.c>
#endif

#ifdef CONFIG_DEVICE_DEPS

സ്ഥിതം പൂർണ്ണസംഖ്യ device_visitor(സ്ഥിരം device_handle_t *handles,
			   size_t handle_എണ്ണം,
			   device_visitor_callback_t visitor_cb,
			   ശൂന്യം *context)
{
	/* Iterate over fixed devices */
	വേണ്ടി (size_t i = 0; i < handle_എണ്ണം; ++i) {
		device_handle_t dh = handles[i];
		സ്ഥിരം ഘടന device *rdev = device_from_handle(dh);
		പൂർണ്ണസംഖ്യ rc = visitor_cb(rdev, context);

		എങ്കിൽ (rc < 0) {
			തിരികെ rc;
		}
	}

	തിരികെ handle_എണ്ണം;
}

പൂർണ്ണസംഖ്യ device_required_foreach(സ്ഥിരം ഘടന device *dev,
			    device_visitor_callback_t visitor_cb,
			    ശൂന്യം *context)
{
	size_t handle_എണ്ണം = 0;
	സ്ഥിരം device_handle_t *handles = device_required_handles_get(dev, &handle_എണ്ണം);

	തിരികെ device_visitor(handles, handle_എണ്ണം, visitor_cb, context);
}

പൂർണ്ണസംഖ്യ device_supported_foreach(സ്ഥിരം ഘടന device *dev,
			     device_visitor_callback_t visitor_cb,
			     ശൂന്യം *context)
{
	size_t handle_എണ്ണം = 0;
	സ്ഥിരം device_handle_t *handles = device_supported_handles_get(dev, &handle_എണ്ണം);

	തിരികെ device_visitor(handles, handle_എണ്ണം, visitor_cb, context);
}

#endif /* CONFIG_DEVICE_DEPS */
