/*
 * Copyright (C) 2007 Michael Brown <mbrown@fensystems.co.uk>.
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

FILE_LICENCE ( GPL2_OR_LATER );

/**
 * @file
 *
 * iPXE scripts
 *
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <ipxe/command.h>
#include <ipxe/parseopt.h>
#include <ipxe/image.h>
#include <ipxe/shell.h>
#include <usr/prompt.h>
#include <ipxe/script.h>

/** Offset within current script
 *
 * This is a global in order to allow goto_exec() to update the
 * offset.
 */
static size_t script_offset;

/**
 * Process script lines
 *
 * @v image		Script
 * @v process_line	Line processor
 * @v terminate		Termination check
 * @ret rc		Return status code
 */
static int process_script ( struct image *image,
			    int ( * process_line ) ( const char *line ),
			    int ( * terminate ) ( int rc ) ) {
	size_t len = 0;
	char *line = NULL;
	off_t eol;
	size_t frag_len;
	char *tmp;
	int rc;

	script_offset = 0;

	do {
	
		/* Find length of next line, excluding any terminating '\n' */
		eol = memchr_user ( image->data, script_offset, '\n',
				    ( image->len - script_offset ) );
		if ( eol < 0 )
			eol = image->len;
		frag_len = ( eol - script_offset );

		/* Allocate buffer for line */
		tmp = realloc ( line, ( len + frag_len + 1 /* NUL */ ) );
		if ( ! tmp ) {
			rc = -ENOMEM;
			goto err_alloc;
		}
		line = tmp;

		/* Copy line */
		copy_from_user ( ( line + len ), image->data, script_offset,
				 frag_len );
		len += frag_len;

		/* Move to next line in script */
		script_offset += ( frag_len + 1 );

		/* Strip trailing CR, if present */
		if ( len && ( line[ len - 1 ] == '\r' ) )
			len--;

		/* Handle backslash continuations */
		if ( len && ( line[ len - 1 ] == '\\' ) ) {
			len--;
			rc = -EINVAL;
			continue;
		}

		/* Terminate line */
		line[len] = '\0';
		DBG ( "$ %s\n", line );

		/* Process line */
		rc = process_line ( line );
		if ( terminate ( rc ) )
			goto err_process;

		/* Free line */
		free ( line );
		line = NULL;
		len = 0;

	} while ( script_offset < image->len );

 err_process:
 err_alloc:
	free ( line );
	return rc;
}

/**
 * Terminate script processing on shell exit or command failure
 *
 * @v rc		Line processing status
 * @ret terminate	Terminate script processing
 */
static int terminate_on_exit_or_failure ( int rc ) {

	return ( shell_stopped ( SHELL_STOP_COMMAND_SEQUENCE ) ||
		 ( rc != 0 ) );
}

/**
 * Find label within script line
 *
 * @v line		Line of script
 * @ret label		Start of label name, or NULL if not found
 */
static const char * find_label ( const char *line ) {

	/* Skip any leading whitespace */
	while ( isspace ( *line ) )
		line++;

	/* If first non-whitespace character is a ':', then we have a label */
	if ( *line == ':' ) {
		return ( line + 1 );
	} else {
		return NULL;
	}
}

/**
 * Execute script line
 *
 * @v line		Line of script
 * @ret rc		Return status code
 */
static int script_exec_line ( const char *line ) {
	int rc;

	/* Skip label lines */
	if ( find_label ( line ) != NULL )
		return 0;

	/* Execute command */
	if ( ( rc = system ( line ) ) != 0 )
		return rc;

	return 0;
}

/**
 * Execute script
 *
 * @v image		Script
 * @ret rc		Return status code
 */
static int script_exec ( struct image *image ) {
	size_t saved_offset;
	int rc;

	/* Temporarily de-register image, so that a "boot" command
	 * doesn't throw us into an execution loop.
	 */
	unregister_image ( image );

	/* Preserve state of any currently-running script */
	saved_offset = script_offset;

	/* Process script */
	rc = process_script ( image, script_exec_line,
			      terminate_on_exit_or_failure );

	/* Restore saved state */
	script_offset = saved_offset;

	/* Re-register image (unless we have been replaced) */
	if ( ! image->replacement )
		register_image ( image );

	return rc;
}

/**
 * Probe script image
 *
 * @v image		Script
 * @ret rc		Return status code
 */
static int script_probe ( struct image *image ) {
	static const char ipxe_magic[] = "#!ipxe";
	static const char gpxe_magic[] = "#!gpxe";
	linker_assert ( sizeof ( ipxe_magic ) == sizeof ( gpxe_magic ),
			magic_size_mismatch );
	char test[ sizeof ( ipxe_magic ) - 1 /* NUL */
		   + 1 /* terminating space */];

	/* Sanity check */
	if ( image->len < sizeof ( test ) ) {
		DBG ( "Too short to be a script\n" );
		return -ENOEXEC;
	}

	/* Check for magic signature */
	copy_from_user ( test, image->data, 0, sizeof ( test ) );
	if ( ! ( ( ( memcmp ( test, ipxe_magic, sizeof ( test ) - 1 ) == 0 ) ||
		   ( memcmp ( test, gpxe_magic, sizeof ( test ) - 1 ) == 0 )) &&
		 isspace ( test[ sizeof ( test ) - 1 ] ) ) ) {
		DBG ( "Invalid magic signature\n" );
		return -ENOEXEC;
	}

	return 0;
}

/** Script image type */
struct image_type script_image_type __image_type ( PROBE_NORMAL ) = {
	.name = "script",
	.probe = script_probe,
	.exec = script_exec,
};

/** "goto" options */
struct goto_options {};

/** "goto" option list */
static struct option_descriptor goto_opts[] = {};

/** "goto" command descriptor */
static struct command_descriptor goto_cmd =
	COMMAND_DESC ( struct goto_options, goto_opts, 1, 1, "<label>" );

/**
 * Current "goto" label
 *
 * Valid only during goto_exec().  Consider this part of a closure.
 */
static const char *goto_label;

/**
 * Check for presence of label
 *
 * @v line		Script line
 * @ret rc		Return status code
 */
static int goto_find_label ( const char *line ) {
	size_t len = strlen ( goto_label );
	const char *label;

	/* Find label */
	label = find_label ( line );
	if ( ! label )
		return -ENOENT;

	/* Check if label matches */
	if ( strncmp ( goto_label, label, len ) != 0 )
		return -ENOENT;

	/* Check label is terminated by a NUL or whitespace */
	if ( label[len] && ! isspace ( label[len] ) )
		return -ENOENT;

	return 0;
}

/**
 * Terminate script processing when label is found
 *
 * @v rc		Line processing status
 * @ret terminate	Terminate script processing
 */
static int terminate_on_label_found ( int rc ) {
	return ( rc == 0 );
}

/**
 * "goto" command
 *
 * @v argc		Argument count
 * @v argv		Argument list
 * @ret rc		Return status code
 */
static int goto_exec ( int argc, char **argv ) {
	struct goto_options opts;
	size_t saved_offset;
	int rc;

	/* Parse options */
	if ( ( rc = parse_options ( argc, argv, &goto_cmd, &opts ) ) != 0 )
		return rc;

	/* Sanity check */
	if ( ! current_image ) {
		rc = -ENOTTY;
		printf ( "Not in a script: %s\n", strerror ( rc ) );
		return rc;
	}

	/* Parse label */
	goto_label = argv[optind];

	/* Find label */
	saved_offset = script_offset;
	if ( ( rc = process_script ( current_image, goto_find_label,
				     terminate_on_label_found ) ) != 0 ) {
		script_offset = saved_offset;
		return rc;
	}

	/* Terminate processing of current command */
	shell_stop ( SHELL_STOP_COMMAND );

	return 0;
}

/** "goto" command */
struct command goto_command __command = {
	.name = "goto",
	.exec = goto_exec,
};

/** "prompt" options */
struct prompt_options {
	/** Key to wait for */
	unsigned int key;
	/** Timeout */
	unsigned int timeout;
};

/** "prompt" option list */
static struct option_descriptor prompt_opts[] = {
	OPTION_DESC ( "key", 'k', required_argument,
		      struct prompt_options, key, parse_key ),
	OPTION_DESC ( "timeout", 't', required_argument,
		      struct prompt_options, timeout, parse_integer ),
};

/** "prompt" command descriptor */
static struct command_descriptor prompt_cmd =
	COMMAND_DESC ( struct prompt_options, prompt_opts, 0, MAX_ARGUMENTS,
		       "[--key <key>] [--timeout <timeout>] [<text>]" );

/**
 * "prompt" command
 *
 * @v argc		Argument count
 * @v argv		Argument list
 * @ret rc		Return status code
 */
static int prompt_exec ( int argc, char **argv ) {
	struct prompt_options opts;
	char *text;
	int rc;

	/* Parse options */
	if ( ( rc = parse_options ( argc, argv, &prompt_cmd, &opts ) ) != 0 )
		goto err_parse;

	/* Parse prompt text */
	text = concat_args ( &argv[optind] );
	if ( ! text ) {
		rc = -ENOMEM;
		goto err_concat;
	}

	/* Display prompt and wait for key */
	if ( ( rc = prompt ( text, opts.timeout, opts.key ) ) != 0 )
		goto err_prompt;

	/* Free prompt text */
	free ( text );

	return 0;

 err_prompt:
	free ( text );
 err_concat:
 err_parse:
	return rc;
}

/** "prompt" command */
struct command prompt_command __command = {
	.name = "prompt",
	.exec = prompt_exec,
};
