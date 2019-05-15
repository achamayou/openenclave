// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <openenclave/corelibc/stdio.h>
#include <openenclave/corelibc/string.h>
#include <openenclave/corelibc/unistd.h>
#include <openenclave/enclave.h>
#include <openenclave/internal/posix/device.h>
#include <openenclave/internal/posix/fdtable.h>
#include <openenclave/internal/posix/raise.h>
#include <openenclave/internal/thread.h>
#include <openenclave/internal/trace.h>
#include "posix_t.h"

#define DEVICE_NAME "consolefs"
#define MAGIC 0x0b292bab

typedef struct _file
{
    struct _oe_device base;
    uint32_t magic;
    oe_host_fd_t host_fd;
} file_t;

static file_t* _cast_file(const oe_device_t* device)
{
    file_t* file = (file_t*)device;

    if (file == NULL || file->magic != MAGIC)
        return NULL;

    return file;
}

static int _consolefs_dup(oe_device_t* file_, oe_device_t** new_file_out)
{
    int ret = -1;
    file_t* file = _cast_file(file_);
    oe_host_fd_t retval = -1;

    if (new_file_out)
        *new_file_out = NULL;

    if (!file || !new_file_out)
        OE_RAISE_ERRNO(OE_EINVAL);

    /* Ask the host to perform this operation. */
    {
        oe_errno = 0;

        if (oe_posix_dup_ocall(&retval, file->host_fd) != OE_OK)
            OE_RAISE_ERRNO(OE_EINVAL);

        if (retval == -1)
            OE_RAISE_ERRNO(oe_errno);
    }

    /* Allocate and initialize a new file structure. */
    {
        file_t* new_file;

        if (!(new_file = oe_calloc(1, sizeof(file_t))))
            OE_RAISE_ERRNO(OE_ENOMEM);

        *new_file = *file;
        new_file->host_fd = retval;
        *new_file_out = (oe_device_t*)new_file;
    }

    ret = 0;

done:
    return ret;
}

static int _consolefs_ioctl(
    oe_device_t* file,
    unsigned long request,
    uint64_t arg)
{
    int ret = -1;

    OE_UNUSED(file);
    OE_UNUSED(request);
    OE_UNUSED(arg);
    OE_RAISE_ERRNO(OE_ENOTSUP);

done:
    return ret;
}

static int _consolefs_fcntl(oe_device_t* file, int cmd, uint64_t arg)
{
    int ret = -1;

    OE_UNUSED(file);
    OE_UNUSED(cmd);
    OE_UNUSED(arg);
    OE_RAISE_ERRNO(OE_ENOTSUP);

done:
    return ret;
}

static ssize_t _consolefs_read(oe_device_t* file_, void* buf, size_t count)
{
    ssize_t ret = -1;
    file_t* file = _cast_file(file_);

    oe_errno = 0;

    if (!file)
        OE_RAISE_ERRNO(OE_EINVAL);

    if (oe_posix_read_ocall(&ret, file->host_fd, buf, count) != OE_OK)
        OE_RAISE_ERRNO(OE_EINVAL);

done:
    return ret;
}

static ssize_t _consolefs_write(
    oe_device_t* file_,
    const void* buf,
    size_t count)
{
    ssize_t ret = -1;
    file_t* file = _cast_file(file_);

    oe_errno = 0;

    if (!file)
        OE_RAISE_ERRNO(OE_EINVAL);

    if (oe_posix_write_ocall(&ret, file->host_fd, buf, count) != OE_OK)
        OE_RAISE_ERRNO(OE_EINVAL);

done:
    return ret;
}

static oe_host_fd_t _consolefs_gethostfd(oe_device_t* file_)
{
    oe_host_fd_t ret = -1;
    file_t* file = _cast_file(file_);

    if (!file)
        OE_RAISE_ERRNO(OE_EINVAL);

    ret = file->host_fd;

done:
    return ret;
}

static oe_off_t _consolefs_lseek(
    oe_device_t* file_,
    oe_off_t offset,
    int whence)
{
    oe_off_t ret = -1;
    file_t* file = _cast_file(file_);

    oe_errno = 0;

    if (!file)
        OE_RAISE_ERRNO(OE_EINVAL);

    if (oe_posix_lseek_ocall(&ret, file->host_fd, offset, whence) != OE_OK)
        OE_RAISE_ERRNO(OE_EINVAL);

done:
    return ret;
}

static int _consolefs_close(oe_device_t* file_)
{
    int ret = -1;
    file_t* file = _cast_file(file_);
    oe_errno = 0;

    if (!file)
        OE_RAISE_ERRNO(OE_EINVAL);

    /* Ask the host to perform this operation. */
    {
        if (oe_posix_close_ocall(&ret, file->host_fd) != OE_OK)
            OE_RAISE_ERRNO(OE_EINVAL);

        if (ret == -1)
            OE_RAISE_ERRNO(oe_errno);
    }

    switch (file->host_fd)
    {
        case OE_STDIN_FILENO:
        case OE_STDOUT_FILENO:
        case OE_STDERR_FILENO:
        {
            /* Do not free these statically initialized structures. */
            file->host_fd = -1;
            break;
        }
        default:
        {
            /* Free file obtained with dup(). */
            oe_free(file);
        }
    }

done:
    return ret;
}

static oe_fs_ops_t _ops = {
    .base.dup = _consolefs_dup,
    .base.shutdown = NULL,
    .base.ioctl = _consolefs_ioctl,
    .base.fcntl = _consolefs_fcntl,
    .open = NULL,
    .base.read = _consolefs_read,
    .base.write = _consolefs_write,
    .base.get_host_fd = _consolefs_gethostfd,
    .clone = NULL,
    .release = NULL,
    .mount = NULL,
    .unmount = NULL,
    .lseek = _consolefs_lseek,
    .base.close = _consolefs_close,
    .getdents = NULL,
    .stat = NULL,
    .access = NULL,
    .link = NULL,
    .unlink = NULL,
    .rename = NULL,
    .truncate = NULL,
    .mkdir = NULL,
    .rmdir = NULL,
};

static file_t _stdin_file = {
    .base.type = OE_DEVICE_TYPE_FILESYSTEM,
    .base.name = DEVICE_NAME,
    .base.ops.fs = &_ops,
    .magic = MAGIC,
    .host_fd = OE_STDIN_FILENO,
};

static file_t _stdout_file = {
    .base.type = OE_DEVICE_TYPE_FILESYSTEM,
    .base.name = DEVICE_NAME,
    .base.ops.fs = &_ops,
    .magic = MAGIC,
    .host_fd = OE_STDOUT_FILENO,
};

static file_t _stderr_file = {
    .base.type = OE_DEVICE_TYPE_FILESYSTEM,
    .base.name = DEVICE_NAME,
    .base.ops.fs = &_ops,
    .magic = MAGIC,
    .host_fd = OE_STDERR_FILENO,
};

static oe_once_t _once = OE_ONCE_INITIALIZER;
static bool _loaded;

static void _load_once(void)
{
    oe_result_t result = OE_FAILURE;

    if (oe_fdtable_reassign(OE_STDIN_FILENO, &_stdin_file.base) != 0)
        OE_RAISE_ERRNO(oe_errno);

    if (oe_fdtable_reassign(OE_STDOUT_FILENO, &_stdout_file.base) != 0)
        OE_RAISE_ERRNO(oe_errno);

    if (oe_fdtable_reassign(OE_STDERR_FILENO, &_stderr_file.base) != 0)
        OE_RAISE_ERRNO(oe_errno);

    result = OE_OK;

done:

    if (result == OE_OK)
        _loaded = true;
}

oe_result_t oe_load_module_console_file_system(void)
{
    if (oe_once(&_once, _load_once) != OE_OK || !_loaded)
        return OE_FAILURE;

    return OE_OK;
}