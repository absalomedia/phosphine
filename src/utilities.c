/**
 * Phosphine
 * PHP bindings to libhydrogen - fast, native, lightweight crypto in PHP!
 *
 * https://github.com/absalomedia/phosphine
 * Copyright (c) 2018 Lawrence Meckan <media@absalom.biz>
 * 
 */

#include <string.h>
#include <ctype.h>

#include "utilities.h"

/**
 * A C implementation of PHP's trim()
 */
char *trim(char *str)
{
	char *end;

	while(isspace(*str)) str++;

	if(*str == 0)
	{
		return str;
	}

	end = str + strlen(str) - 1;
	while(end > str && isspace(*end)) end--;

	*(end+1) = 0;

	return str;
}