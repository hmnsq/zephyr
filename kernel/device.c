/*
 * Copyright (c) 2015-2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <stddef.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/device.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/atomic.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/iterable_sections.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/kobject.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/internal/syscall_handler.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/toolchain.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/pm/device_runtime.h>

ପୂର୍ଣ୍ଣସଂଖ୍ୟା do_device_init(ଅଚଳ ସଂରଚନା device *dev)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;

	ଯଦି (dev->ops.init != ଖାଲି) {
		rc = dev->ops.init(dev);
		/* If initialization failed, record in dev->state->init_res
		 * the POSITIVE value of the resulting errno
		 */
		ଯଦି (rc != 0) {
			/* device's init function should return:
			 *   0 on success
			 *   a negative value on failure (-errno)
			 * errno value maps to an uint8_t range as of now.
			 */
			__ASSERT(rc >= -UINT8_MAX && rc < 0, "device %s init: invalid error (%d)",
				 dev->name, rc);

			ଯଦି (rc < 0) {
				rc = -rc;
			}
			/* handle error value overflow in production
			 * this is likely a bug in the device's init function. Signals it
			 */
			ଯଦି (rc > UINT8_MAX) {
				rc = UINT8_MAX;
			}
		}

		/* Always record the result: a device that failed to initialize
		 * earlier must not keep that error once it initializes
		 * successfully.
		 */
		dev->state->init_res = rc;
	}

	ଯଦି (rc == 0) {
		/* A failed init leaves the device uninitialized, so it can be
		 * retried by device_init() and nothing can be released by
		 * device_deinit().
		 */
		dev->state->initialized = true;

		/* Run automatic device runtime enablement */
		(ଶୂନ୍ୟ)pm_device_runtime_auto_enable(dev);
	}

	/* here, the value of rc is either 0 or +errno
	 * flip the sign to return a negative value on failure as expected
	 */
	ଫେରନ୍ତୁ -rc;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_device_init(ଅଚଳ ସଂରଚନା device *dev)
{
	ଯଦି (dev->state->initialized) {
		ଫେରନ୍ତୁ -EALREADY;
	}

	ଫେରନ୍ତୁ do_device_init(dev);
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_device_init(ଅଚଳ ସଂରଚନା device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	ଫେରନ୍ତୁ z_impl_device_init(dev);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/device_init_mrsh.c>
#endif


ଅଚଳ ସଂରଚନା device *z_impl_device_get_binding(ଅଚଳ ଅକ୍ଷର *name)
{
	/* A null string identifies no device.  So does an empty
	 * string.
	 */
	ଯଦି ((name == ଖାଲି) || (name[0] == '\0')) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	/* Return NULL if the device matching 'name' is not ready. */
	STRUCT_SECTION_FOREACH(device, dev) {
		ଯଦି ((dev->name == name) || (strcmp(name, dev->name) == 0)) {
			ଫେରନ୍ତୁ z_impl_device_is_ready(dev) ? dev : ଖାଲି;
		}
	}

	ଫେରନ୍ତୁ ଖାଲି;
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଅଚଳ ସଂରଚନା device *z_vrfy_device_get_binding(ଅଚଳ ଅକ୍ଷର *name)
{
	ଅକ୍ଷର name_copy[Z_DEVICE_MAX_NAME_LEN];

	ଯଦି (k_usermode_string_copy(name_copy, name, ଆକାର(name_copy))
	    != 0) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଫେରନ୍ତୁ z_impl_device_get_binding(name_copy);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/device_get_binding_mrsh.c>

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool z_vrfy_device_is_ready(ଅଚଳ ସଂରଚନା device *dev)
{
	/* NULL is a documented argument, not an invalid kernel object */
	ଯଦି (dev == ଖାଲି) {
		ଫେରନ୍ତୁ false;
	}

	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	ଫେରନ୍ତୁ z_impl_device_is_ready(dev);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/device_is_ready_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_DEVICE_DT_METADATA
ଅଚଳ ସଂରଚନା device *z_impl_device_get_by_dt_nodelabel(ଅଚଳ ଅକ୍ଷର *nodelabel)
{
	/* For consistency with device_get_binding(). */
	ଯଦି ((nodelabel == ଖାଲି) || (nodelabel[0] == '\0')) {
		ଫେରନ୍ତୁ ଖାଲି;
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
		ଅଚଳ ସଂରଚନା device_dt_nodelabels *nl = device_get_dt_nodelabels(dev);

		ଯଦି (!z_impl_device_is_ready(dev) || nl == ଖାଲି) {
			ଜାରି;
		}

		ପାଇଁ (size_t i = 0; i < nl->ଅଙ୍କ_nodelabels; i++) {
			ଅଚଳ ଅକ୍ଷର *dev_nodelabel = nl->nodelabels[i];

			ଯଦି (strcmp(nodelabel, dev_nodelabel) == 0) {
				ଫେରନ୍ତୁ dev;
			}
		}
	}

	ଫେରନ୍ତୁ ଖାଲି;
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଅଚଳ ସଂରଚନା device *z_vrfy_device_get_by_dt_nodelabel(ଅଚଳ ଅକ୍ଷର *nodelabel)
{
	ଅକ୍ଷର nl_copy[Z_DEVICE_MAX_NODELABEL_LEN];

	ଯଦି (k_usermode_string_copy(nl_copy, (ଅକ୍ଷର *)nodelabel, ଆକାର(nl_copy)) != 0) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଫେରନ୍ତୁ z_impl_device_get_by_dt_nodelabel(nl_copy);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/device_get_by_dt_nodelabel_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_DEVICE_DT_METADATA */

size_t z_device_get_all_static(ସଂରଚନା device ଅଚଳ **devices)
{
	size_t cnt;

	STRUCT_SECTION_GET(device, 0, devices);
	STRUCT_SECTION_ଗଣନା(device, &cnt);

	ଫେରନ୍ତୁ cnt;
}

bool z_impl_device_is_ready(ଅଚଳ ସଂରଚନା device *dev)
{
	/*
	 * if an invalid device pointer is passed as argument, this call
	 * reports the `device` as not ready for usage.
	 */
	ଯଦି (dev == ଖାଲି) {
		ଫେରନ୍ତୁ false;
	}

	ଫେରନ୍ତୁ dev->state->initialized;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_device_deinit(ଅଚଳ ସଂରଚନା device *dev)
{
#ifdef CONFIG_DEVICE_DEINIT_SUPPORT
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ଯଦି (!dev->state->initialized) {
		ଫେରନ୍ତୁ -EPERM;
	}

	ଯଦି (dev->ops.deinit == ଖାଲି) {
		ଫେରନ୍ତୁ -ENOTSUP;
	}

	ret = dev->ops.deinit(dev);
	ଯଦି (ret < 0) {
		ଫେରନ୍ତୁ ret;
	}

	dev->state->initialized = false;

	ଫେରନ୍ତୁ 0;
#ଅନ୍ୟଥା
	ARG_UNUSED(dev);
	ଫେରନ୍ତୁ -ENOTSUP;
#endif /* CONFIG_DEVICE_DEINIT_SUPPORT */
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_device_deinit(ଅଚଳ ସଂରଚନା device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	ଫେରନ୍ତୁ z_impl_device_deinit(dev);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/device_deinit_mrsh.c>
#endif

#ifdef CONFIG_DEVICE_DEPS

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା device_visitor(ଅଚଳ device_handle_t *handles,
			   size_t handle_ଗଣନା,
			   device_visitor_callback_t visitor_cb,
			   ଶୂନ୍ୟ *context)
{
	/* Iterate over fixed devices */
	ପାଇଁ (size_t i = 0; i < handle_ଗଣନା; ++i) {
		device_handle_t dh = handles[i];
		ଅଚଳ ସଂରଚନା device *rdev = device_from_handle(dh);
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = visitor_cb(rdev, context);

		ଯଦି (rc < 0) {
			ଫେରନ୍ତୁ rc;
		}
	}

	ଫେରନ୍ତୁ handle_ଗଣନା;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା device_required_foreach(ଅଚଳ ସଂରଚନା device *dev,
			    device_visitor_callback_t visitor_cb,
			    ଶୂନ୍ୟ *context)
{
	size_t handle_ଗଣନା = 0;
	ଅଚଳ device_handle_t *handles = device_required_handles_get(dev, &handle_ଗଣନା);

	ଫେରନ୍ତୁ device_visitor(handles, handle_ଗଣନା, visitor_cb, context);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା device_supported_foreach(ଅଚଳ ସଂରଚନା device *dev,
			     device_visitor_callback_t visitor_cb,
			     ଶୂନ୍ୟ *context)
{
	size_t handle_ଗଣନା = 0;
	ଅଚଳ device_handle_t *handles = device_supported_handles_get(dev, &handle_ଗଣନା);

	ଫେରନ୍ତୁ device_visitor(handles, handle_ଗଣନା, visitor_cb, context);
}

#endif /* CONFIG_DEVICE_DEPS */
