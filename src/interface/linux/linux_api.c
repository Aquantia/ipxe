/*
 * Copyright (C) 2010 Piotr Jaroszyński <p.jaroszynski@gmail.com>.
 * Copyright (C) 2021 Michael Brown <mbrown@fensystems.co.uk>.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#define _GNU_SOURCE
#include <stdint.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <ipxe/linux_api.h>

/** @file
 *
 * Linux host API
 *
 */

/** Construct prefixed symbol name */
#define _C1( x, y ) x ## y
#define _C2( x, y ) _C1 ( x, y )

/** Construct prefixed symbol name for iPXE symbols */
#define IPXE_SYM( symbol ) _C2 ( SYMBOL_PREFIX, symbol )

/** Provide a prefixed symbol alias visible to iPXE code */
#define PROVIDE_IPXE_SYM( symbol )					\
	extern typeof ( symbol ) IPXE_SYM ( symbol )			\
		__attribute__ (( alias ( #symbol) ))

/** Most recent system call error */
int linux_errno __attribute__ (( nocommon ));

/******************************************************************************
 *
 * Host entry point
 *
 ******************************************************************************
 */

extern int IPXE_SYM ( _linux_start ) ( int argc, char **argv );

/**
 * Main entry point
 *
 * @v argc		Argument count
 * @v argv		Argument list
 * @ret rc		Exit status
 */
int main ( int argc, char **argv ) {

	return IPXE_SYM ( _linux_start ) ( argc, argv );
}

/******************************************************************************
 *
 * System call wrappers
 *
 ******************************************************************************
 */

/**
 * Wrap open()
 *
 */
int __asmcall linux_open ( const char *pathname, int flags, ... ) {
	va_list args;
	mode_t mode;
	int ret;

	va_start ( args, flags );
	mode = va_arg ( args, mode_t );
	va_end ( args );
	ret = open ( pathname, flags, mode );
	if ( ret == -1 )
		linux_errno = errno;
	return ret;
}

/**
 * Wrap close()
 *
 */
int __asmcall linux_close ( int fd ) {
	int ret;

	ret = close ( fd );
	if ( ret == -1 )
		linux_errno = errno;
	return ret;
}

/**
 * Wrap lseek()
 *
 */
off_t __asmcall linux_lseek ( int fd, off_t offset, int whence ) {
	off_t ret;

	ret = lseek ( fd, offset, whence );
	if ( ret == -1 )
		linux_errno = errno;
	return ret;
}

/**
 * Wrap read()
 *
 */
ssize_t __asmcall linux_read ( int fd, void *buf, size_t count ) {
	ssize_t ret;

	ret = read ( fd, buf, count );
	if ( ret == -1 )
		linux_errno = errno;
	return ret;
}

/**
 * Wrap write()
 *
 */
ssize_t __asmcall linux_write ( int fd, const void *buf, size_t count ) {
	ssize_t ret;

	ret = write ( fd, buf, count );
	if ( ret == -1 )
		linux_errno = errno;
	return ret;
}

/**
 * Wrap fcntl()
 *
 */
int __asmcall linux_fcntl ( int fd, int cmd, ... ) {
	va_list args;
	long arg;
	int ret;

	va_start ( args, cmd );
	arg = va_arg ( args, long );
	va_end ( args );
	ret = fcntl ( fd, cmd, arg );
	if ( ret == -1 )
		linux_errno = errno;
	return ret;
}

/**
 * Wrap ioctl()
 *
 */
int __asmcall linux_ioctl ( int fd, unsigned long request, ... ) {
	va_list args;
	void *arg;
	int ret;

	va_start ( args, request );
	arg = va_arg ( args, void * );
	va_end ( args );
	ret = ioctl ( fd, request, arg );
	if ( ret == -1 )
		linux_errno = errno;
	return ret;
}

/**
 * Wrap poll()
 *
 */
int __asmcall linux_poll ( struct pollfd *fds, unsigned int nfds,
			   int timeout ) {
	int ret;

	ret = poll ( fds, nfds, timeout );
	if ( ret == -1 )
		linux_errno = errno;
}

/**
 * Wrap nanosleep()
 *
 */
int __asmcall linux_nanosleep ( const struct timespec *req,
				struct timespec *rem ) {
	int ret;

	ret = nanosleep ( req, rem );
	if ( ret == -1 )
		linux_errno = errno;
	return ret;
}

/**
 * Wrap usleep()
 *
 */
int __asmcall linux_usleep ( unsigned int usec ) {
	int ret;

	ret = usleep ( usec );
	if ( ret == -1 )
		linux_errno = errno;
	return ret;
}

/**
 * Wrap gettimeofday()
 *
 */
int __asmcall linux_gettimeofday ( struct timeval *tv, struct timezone *tz ) {
	int ret;

	ret = gettimeofday ( tv, tz );
	if ( ret == -1 )
		linux_errno = errno;
	return ret;
}

/**
 * Wrap mmap()
 *
*/
void * __asmcall linux_mmap ( void *addr, size_t length, int prot, int flags,
			      int fd, off_t offset ) {
	void *ret;

	ret = mmap ( addr, length, prot, flags, fd, offset );
	if ( ret == MAP_FAILED )
		linux_errno = errno;
	return ret;
}

/**
 * Wrap mremap()
 *
 */
void * __asmcall linux_mremap ( void *old_address, size_t old_size,
				size_t new_size, int flags, ... ) {
	va_list args;
	void *new_address;
	void *ret;

	va_start ( args, flags );
	new_address = va_arg ( args, void * );
	va_end ( args );
	ret = mremap ( old_address, old_size, new_size, flags, new_address );
	if ( ret == MAP_FAILED )
		linux_errno = errno;
	return ret;
}

/**
 * Wrap munmap()
 *
 */
int __asmcall linux_munmap ( void *addr, size_t length ) {
	int ret;

	ret = munmap ( addr, length );
	if ( ret == -1 )
		linux_errno = errno;
	return ret;
}

/**
 * Wrap socket()
 *
 */
int __asmcall linux_socket ( int domain, int type, int protocol ) {
	int ret;

	ret = socket ( domain, type, protocol );
	if ( ret == -1 )
		linux_errno = errno;
	return ret;
}

/**
 * Wrap bind()
 *
 */
int __asmcall linux_bind ( int sockfd, const struct sockaddr *addr,
			   size_t addrlen ) {
	int ret;

	ret = bind ( sockfd, addr, addrlen );
	if ( ret == -1 )
		linux_errno = errno;
	return ret;
}

/**
 * Wrap sendto()
 *
 */
ssize_t __asmcall linux_sendto ( int sockfd, const void *buf, size_t len,
				 int flags, const struct sockaddr *dest_addr,
				 size_t addrlen ) {
	ssize_t ret;

	ret = sendto ( sockfd, buf, len, flags, dest_addr, addrlen );
	if ( ret == -1 )
		linux_errno = errno;
	return ret;
}

/******************************************************************************
 *
 * C library wrappers
 *
 ******************************************************************************
 */

/**
 * Wrap strerror()
 *
 */
const char * __asmcall linux_strerror ( int linux_errno ) {

	return strerror ( linux_errno );
}

/******************************************************************************
 *
 * Symbol aliases
 *
 ******************************************************************************
 */

PROVIDE_IPXE_SYM ( linux_errno );
PROVIDE_IPXE_SYM ( linux_open );
PROVIDE_IPXE_SYM ( linux_close );
PROVIDE_IPXE_SYM ( linux_lseek );
PROVIDE_IPXE_SYM ( linux_read );
PROVIDE_IPXE_SYM ( linux_write );
PROVIDE_IPXE_SYM ( linux_fcntl );
PROVIDE_IPXE_SYM ( linux_ioctl );
PROVIDE_IPXE_SYM ( linux_poll );
PROVIDE_IPXE_SYM ( linux_nanosleep );
PROVIDE_IPXE_SYM ( linux_usleep );
PROVIDE_IPXE_SYM ( linux_gettimeofday );
PROVIDE_IPXE_SYM ( linux_mmap );
PROVIDE_IPXE_SYM ( linux_mremap );
PROVIDE_IPXE_SYM ( linux_munmap );
PROVIDE_IPXE_SYM ( linux_socket );
PROVIDE_IPXE_SYM ( linux_bind );
PROVIDE_IPXE_SYM ( linux_sendto );
PROVIDE_IPXE_SYM ( linux_strerror );
