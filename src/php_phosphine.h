/**
 * Phosphine
 * PHP bindings to libhydrogen - fast, native, lightweight crypto in PHP!
 *
 * https://github.com/absalomedia/phosphine
 * Copyright (c) 2018 Lawrence Meckan <media@absalom.biz>
 * 
 */


#ifndef PHP_PHOSPHINE_H
#define PHP_PHOSPHINE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <php.h>
#include <ext/standard/info.h>
#include <Zend/zend_extensions.h>
#include <Zend/zend_exceptions.h>

#include <hydrogen.h>

zend_class_entry *phosphine_ce;
zend_class_entry *phosphine_exception_ce;

zend_class_entry *phosphine_get_exception_base();

PHP_METHOD(Phosphine, __construct);
PHP_METHOD(Phosphine, compile);
PHP_METHOD(Phosphine, compileFile);
PHP_METHOD(Phosphine, getStyle);
PHP_METHOD(Phosphine, setStyle);
PHP_METHOD(Phosphine, getIncludePath);
PHP_METHOD(Phosphine, setIncludePath);
PHP_METHOD(Phosphine, getPrecision);
PHP_METHOD(Phosphine, setPrecision);
PHP_METHOD(Phosphine, getComments);
PHP_METHOD(Phosphine, setComments);
PHP_METHOD(Phosphine, getIndent);
PHP_METHOD(Phosphine, setIndent);
PHP_METHOD(Phosphine, getEmbed);
PHP_METHOD(Phosphine, setEmbed);
PHP_METHOD(Phosphine, getMapPath);
PHP_METHOD(Phosphine, setMapPath);
PHP_METHOD(Phosphine, getMapRoot);
PHP_METHOD(Phosphine, setMapRoot);

#endif
