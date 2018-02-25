/**
 * Phosphine
 * PHP bindings to libhydrogen - fast, native, lightweight crypto in PHP!
 *
 * https://github.com/absalomedia/phosphine
 * Copyright (c) 2018 Lawrence Meckan <media@absalom.biz>
 *
 * 
 */

#include <stdio.h>
#if ZEND_MODULE_API_NO > 20131226
#include <stdlib.h>.
#endif

#include "php_phosphine.h"
#include "utilities.h"

/* --------------------------------------------------------------
 * Phosphine
 * ------------------------------------------------------------ */

zend_object_handlers phosphine_handlers;

typedef struct phosphine_object {
    #if PHP_MAJOR_VERSION < 7
    zend_object zo;
    #endif
    int style;
    char* include_paths;
    bool comments;
    bool indent;
    long precision;
    char* map_path;
    bool omit_map_url;
    bool map_embed;
    bool map_contents;
    char* map_root;
    #if PHP_MAJOR_VERSION >= 7
    zend_object zo;
    #endif
} phosphine_object;

zend_class_entry *phosphine_ce;

#if PHP_MAJOR_VERSION >= 7

static inline phosphine_object *phosphine_fetch_object(zend_object *obj)
{
    return (phosphine_object *) ((char*) (obj) - XtOffsetOf(phosphine_object, zo));
}

#define Z_phosphine_P(zv) phosphine_fetch_object(Z_OBJ_P((zv)));

static void phosphine_free_storage(zend_object *object TSRMLS_DC)
{
    phosphine_object *obj;
    obj = phosphine_fetch_object(object);
    zend_object_std_dtor(object TSRMLS_DC);

}
#else
void phosphine_free_storage(void *object TSRMLS_DC)
{
    phosphine_object *obj = (phosphine_object *)object;
    if (obj->include_paths != NULL)
        efree(obj->include_paths);
    if (obj->map_path != NULL)
        efree(obj->map_path);
    if (obj->map_root != NULL)
        efree(obj->map_root);
    zend_hash_destroy(obj->zo.properties);
    FREE_HASHTABLE(obj->zo.properties);
    efree(obj);
}
#endif


#if ZEND_MODULE_API_NO <= 20131226
zend_object_value phosphine_create_handler(zend_class_entry *type TSRMLS_DC) {
    zval *tmp;
    zend_object_value retval;

    phosphine_object *obj = (phosphine_object *)emalloc(sizeof(phosphine_object));
    memset(obj, 0, sizeof(phosphine_object));

    obj->zo.ce = type;

    ALLOC_HASHTABLE(obj->zo.properties);
    zend_hash_init(obj->zo.properties, 0, NULL, ZVAL_PTR_DTOR, 0);
#if PHP_VERSION_ID > 50399
    object_properties_init(&(obj->zo), type);
#endif

    retval.handle = zend_objects_store_put(obj, NULL,
        phosphine_free_storage, NULL TSRMLS_CC);
    retval.handlers = &phosphine_handlers;

    return retval;
 }
#endif

#if ZEND_MODULE_API_NO > 20131226
zend_object * phosphine_create_handler(zend_class_entry *type TSRMLS_DC) {
    struct phosphine_object *obj = ecalloc(1,
         sizeof(struct phosphine_object) +
         zend_object_properties_size(type));

     zend_object_std_init(&obj->zo, type TSRMLS_CC);
     object_properties_init(&obj->zo, type TSRMLS_CC);

     obj->zo.handlers = &phosphine_handlers;

     return &obj->zo;
}

#endif


PHP_METHOD(phosphine, __construct)
{
    zval *this = getThis();

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "", NULL) == FAILURE) {
        RETURN_NULL();
    }
    #if ZEND_MODULE_API_NO > 20131226
    phosphine_object *obj = Z_phosphine_P(this);
    #endif

    #if ZEND_MODULE_API_NO <= 20131226
    phosphine_object *obj = (phosphine_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    obj->style = phosphine_STYLE_NESTED;
    obj->include_paths = NULL;
    obj->map_path = NULL;
    obj->map_root = NULL;
    obj->comments = false;
    obj->indent = false;
    obj->map_embed = false;
    obj->map_contents = false;
    obj->omit_map_url = true;
    obj->precision = 5;

}


void set_options(phosphine_object *this, struct phosphine_Context *ctx)
{
    struct phosphine_Options* opts = phosphine_context_get_options(ctx);

    phosphine_option_set_precision(opts, this->precision);
    phosphine_option_set_output_style(opts, this->style);
    phosphine_option_set_is_indented_syntax_src(opts, this->indent);
    if (this->include_paths != NULL) {
    phosphine_option_set_include_path(opts, this->include_paths);
    }
    phosphine_option_set_source_comments(opts, this->comments);
    if (this->comments) {
    phosphine_option_set_omit_source_map_url(opts, false);
    }
    phosphine_option_set_source_map_embed(opts, this->map_embed);
    phosphine_option_set_source_map_contents(opts, this->map_contents);
    if (this->map_path != NULL) {
    phosphine_option_set_source_map_file(opts, this->map_path);
    phosphine_option_set_omit_source_map_url(opts, false);
    phosphine_option_set_source_map_contents(opts, true);
    }
    if (this->map_root != NULL) {
    phosphine_option_set_source_map_root(opts, this->map_root);
    }

}

/**
 * $phosphine->parse(string $source, [  ]);
 *
 * Parse a string of phosphine; a basic input -> output affair.
 */
PHP_METHOD(phosphine, compile)
{
    #if ZEND_MODULE_API_NO > 20131226
    phosphine_object *this = Z_phosphine_P(getThis());
    #endif

    #if ZEND_MODULE_API_NO <= 20131226
    phosphine_object *this = (phosphine_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
    #endif

    // Define our parameters as local variables
    char *source;
    #if ZEND_MODULE_API_NO <= 20131226
    int source_len;
    #endif
    #if ZEND_MODULE_API_NO > 20131226
    size_t source_len;
    #endif

    // Use zend_parse_parameters() to grab our source from the function call
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &source, &source_len) == FAILURE){
        RETURN_FALSE;
    }

    // Create a new phosphine_context
    struct phosphine_Data_Context* data_context = phosphine_make_data_context(strdup(source));

    struct phosphine_Context* ctx = phosphine_data_context_get_context(data_context);

    set_options(this, ctx);

    int status = phosphine_compile_data_context(data_context);

    // Check the context for any errors...
    if (status != 0)
    {
        zend_throw_exception(phosphine_exception_ce, phosphine_context_get_error_message(ctx), 0 TSRMLS_CC);
    }
    else
    {
        #if ZEND_MODULE_API_NO <= 20131226
        RETVAL_STRING(phosphine_context_get_output_string(ctx), 1);
        #endif
        #if ZEND_MODULE_API_NO > 20131226
        RETVAL_STRING(phosphine_context_get_output_string(ctx));
        #endif
    }

    phosphine_delete_data_context(data_context);
}

/**
 * $phosphine->parse_file(string $file_name);
 *
 * Parse a whole file FULL of phosphine and return the CSS output
 */
PHP_METHOD(phosphine, compileFile)
{
    array_init(return_value);

    #if ZEND_MODULE_API_NO > 20131226
    phosphine_object *this = Z_phosphine_P(getThis());
    #endif

    #if ZEND_MODULE_API_NO <= 20131226
    phosphine_object *this = (phosphine_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
    #endif

    // We need a file name and a length
    char *file;
    #if ZEND_MODULE_API_NO <= 20131226
    int file_len;
    #endif
    #if ZEND_MODULE_API_NO > 20131226
    size_t file_len;
    #endif

    // Grab the file name from the function
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &file, &file_len) == FAILURE)
    {
        RETURN_FALSE;
    }

    // First, do a little checking of our own. Does the file exist?
    if( access( file, F_OK ) == -1 )
    {
        zend_throw_exception_ex(phosphine_exception_ce, 0 TSRMLS_CC, "File %s could not be found", file);
        RETURN_FALSE;
    }

    struct phosphine_File_Context* file_ctx = phosphine_make_file_context(file);

    struct phosphine_Context* ctx = phosphine_file_context_get_context(file_ctx);

    set_options(this, ctx);

    int status = phosphine_compile_file_context(file_ctx);

    // Check the context for any errors...
    if (status != 0)
    {
        zend_throw_exception(phosphine_exception_ce, phosphine_context_get_error_message(ctx), 0 TSRMLS_CC);
    }
    else
    {

        #if ZEND_MODULE_API_NO <= 20131226
        if (this->map_path != NULL ) {
        // Send it over to PHP.
        add_next_index_string(return_value, phosphine_context_get_output_string(ctx), 1);
        } else {
        RETVAL_STRING(phosphine_context_get_output_string(ctx), 1);
        }

         // Do we have source maps to go?
         if (this->map_path != NULL)
         {
         // Send it over to PHP.
         add_next_index_string(return_value, phosphine_context_get_source_map_string(ctx), 1);
         }
         #endif

        #if ZEND_MODULE_API_NO > 20131226
        if (this->map_path != NULL ) {
        // Send it over to PHP.
        add_next_index_string(return_value, phosphine_context_get_output_string(ctx));
        } else {
        RETVAL_STRING(phosphine_context_get_output_string(ctx));
        }

         // Do we have source maps to go?
         if (this->map_path != NULL)
         {
         // Send it over to PHP.
         add_next_index_string(return_value, phosphine_context_get_source_map_string(ctx));
         }
         #endif
    }

    phosphine_delete_file_context(file_ctx);
}

PHP_METHOD(phosphine, getStyle)
{
    zval *this = getThis();

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "", NULL) == FAILURE) {
        RETURN_FALSE;
    }

    #if ZEND_MODULE_API_NO > 20131226
    phosphine_object *obj = Z_phosphine_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    phosphine_object *obj = (phosphine_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    RETURN_LONG(obj->style);
}

PHP_METHOD(phosphine, setStyle)
{
    zval *this = getThis();

    long new_style;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &new_style) == FAILURE) {
        RETURN_FALSE;
    }

    #if ZEND_MODULE_API_NO > 20131226
    phosphine_object *obj = Z_phosphine_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    phosphine_object *obj = (phosphine_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif
    obj->style = new_style;

    RETURN_NULL();
}

PHP_METHOD(phosphine, getIncludePath)
{
    zval *this = getThis();

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "", NULL) == FAILURE) {
        RETURN_FALSE;
    }

    #if ZEND_MODULE_API_NO > 20131226
    phosphine_object *obj = Z_phosphine_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    phosphine_object *obj = (phosphine_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    #if ZEND_MODULE_API_NO <= 20131226
    if (obj->include_paths == NULL) RETURN_STRING("", 1);
    RETURN_STRING(obj->include_paths, 1);
    #endif

    #if ZEND_MODULE_API_NO > 20131226
    if (obj->include_paths == NULL) RETURN_STRING("");
    RETURN_STRING(obj->include_paths);
    #endif
}

PHP_METHOD(phosphine, setIncludePath)
{
    zval *this = getThis();

    char *path;
    #if ZEND_MODULE_API_NO <= 20131226
    int path_len;
    #endif
    #if ZEND_MODULE_API_NO > 20131226
    size_t path_len;
    #endif    

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &path, &path_len) == FAILURE)
        RETURN_FALSE;

    #if ZEND_MODULE_API_NO > 20131226
    phosphine_object *obj = Z_phosphine_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    phosphine_object *obj = (phosphine_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    if (obj->include_paths != NULL)
        efree(obj->include_paths);
    obj->include_paths = estrndup(path, path_len);

    RETURN_NULL();
}

PHP_METHOD(phosphine, getMapPath)
{
    zval *this = getThis();

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "", NULL) == FAILURE) {
        RETURN_FALSE;
    }

    #if ZEND_MODULE_API_NO > 20131226
    phosphine_object *obj = Z_phosphine_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    phosphine_object *obj = (phosphine_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    #if ZEND_MODULE_API_NO <= 20131226
    if (obj->map_path == NULL) RETURN_STRING("", 1);
    RETURN_STRING(obj->map_path, 1);
    #endif

    #if ZEND_MODULE_API_NO > 20131226
    if (obj->map_path == NULL) RETURN_STRING("");
    RETURN_STRING(obj->map_path);
    #endif
}

PHP_METHOD(phosphine, setMapPath)
{
    zval *this = getThis();

    char *path;
    #if ZEND_MODULE_API_NO <= 20131226
    int path_len;
    #endif
    #if ZEND_MODULE_API_NO > 20131226
    size_t path_len;
    #endif

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &path, &path_len) == FAILURE)
        RETURN_FALSE;

    #if ZEND_MODULE_API_NO > 20131226
    phosphine_object *obj = Z_phosphine_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    phosphine_object *obj = (phosphine_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    if (obj->map_path != NULL)
        efree(obj->map_path);
    obj->map_path = estrndup(path, path_len);

    RETURN_NULL();
}


PHP_METHOD(phosphine, getMapRoot)
{
    zval *this = getThis();

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "", NULL) == FAILURE) {
        RETURN_FALSE;
    }

    #if ZEND_MODULE_API_NO > 20131226
    phosphine_object *obj = Z_phosphine_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    phosphine_object *obj = (phosphine_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    #if ZEND_MODULE_API_NO <= 20131226
    if (obj->map_root == NULL) RETURN_STRING("", 1);
    RETURN_STRING(obj->map_root, 1);
    #endif

    #if ZEND_MODULE_API_NO > 20131226
    if (obj->map_root == NULL) RETURN_STRING("");
    RETURN_STRING(obj->map_root);
    #endif
}

PHP_METHOD(phosphine, setMapRoot)
{
    zval *this = getThis();

    char *path;
    #if ZEND_MODULE_API_NO <= 20131226
    int path_len;
    #endif
    #if ZEND_MODULE_API_NO > 20131226
    size_t path_len;
    #endif

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &path, &path_len) == FAILURE)
        RETURN_FALSE;

    #if ZEND_MODULE_API_NO > 20131226
    phosphine_object *obj = Z_phosphine_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    phosphine_object *obj = (phosphine_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    if (obj->map_root != NULL)
        efree(obj->map_root);
    obj->map_root = estrndup(path, path_len);

    RETURN_NULL();
}




PHP_METHOD(phosphine, getPrecision)
{
    zval *this = getThis();

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "", NULL) == FAILURE) {
        RETURN_FALSE;
    }

    #if ZEND_MODULE_API_NO > 20131226
    phosphine_object *obj = Z_phosphine_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    phosphine_object *obj = (phosphine_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    RETURN_LONG(obj->precision);
}

PHP_METHOD(phosphine, setPrecision)
{
    zval *this = getThis();

    long new_precision;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &new_precision) == FAILURE) {
        RETURN_FALSE;
    }

    #if ZEND_MODULE_API_NO > 20131226
    phosphine_object *obj = Z_phosphine_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    phosphine_object *obj = (phosphine_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    obj->precision = new_precision;

    RETURN_NULL();
}

PHP_METHOD(phosphine, getEmbed)
{
    zval *this = getThis();

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "", NULL) == FAILURE) {
        RETURN_FALSE;
    }

    #if ZEND_MODULE_API_NO > 20131226
    phosphine_object *obj = Z_phosphine_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    phosphine_object *obj = (phosphine_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    RETURN_LONG(obj->map_embed);
}

PHP_METHOD(phosphine, setEmbed)
{
    zval *this = getThis();

    bool new_map_embed;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &new_map_embed) == FAILURE) {
        RETURN_FALSE;
    }

    #if ZEND_MODULE_API_NO > 20131226
    phosphine_object *obj = Z_phosphine_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    phosphine_object *obj = (phosphine_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    obj->map_embed = new_map_embed;

    RETURN_NULL();
}


PHP_METHOD(phosphine, getComments)
{
    zval *this = getThis();

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "", NULL) == FAILURE) {
        RETURN_FALSE;
    }

    #if ZEND_MODULE_API_NO > 20131226
    phosphine_object *obj = Z_phosphine_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    phosphine_object *obj = (phosphine_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    RETURN_LONG(obj->comments);
}

PHP_METHOD(phosphine, setComments)
{
    zval *this = getThis();

    bool new_comments;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &new_comments) == FAILURE) {
        RETURN_FALSE;
    }

    #if ZEND_MODULE_API_NO > 20131226
    phosphine_object *obj = Z_phosphine_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    phosphine_object *obj = (phosphine_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    obj->comments = new_comments;

    RETURN_NULL();
}


PHP_METHOD(phosphine, getIndent)
{
    zval *this = getThis();

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "", NULL) == FAILURE) {
        RETURN_FALSE;
    }

    #if ZEND_MODULE_API_NO > 20131226
    phosphine_object *obj = Z_phosphine_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    phosphine_object *obj = (phosphine_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    RETURN_LONG(obj->indent);
}

PHP_METHOD(phosphine, setIndent)
{
    zval *this = getThis();

    bool new_indent;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &new_indent) == FAILURE) {
        RETURN_FALSE;
    }

    #if ZEND_MODULE_API_NO > 20131226
    phosphine_object *obj = Z_phosphine_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    phosphine_object *obj = (phosphine_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    obj->indent = new_indent;

    RETURN_NULL();
}


PHP_METHOD(phosphine, getLibraryVersion)
{
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "", NULL) == FAILURE) {
        RETURN_FALSE;
    }

    RETURN_STRING(HYDRO_VERSION_MAJOR+"."+HYDRO_VERSION_MINOR, 1);
    #endif

}
/* --------------------------------------------------------------
 * EXCEPTION HANDLING
 * ------------------------------------------------------------ */

zend_class_entry *phosphine_get_exception_base(TSRMLS_D)
{
    return zend_exception_get_default(TSRMLS_C);
}

/* --------------------------------------------------------------
 * PHP EXTENSION INFRASTRUCTURE
 * ------------------------------------------------------------ */

ZEND_BEGIN_ARG_INFO(arginfo_phosphine_void, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phosphine_compile, 0, 0, 1)
    ZEND_ARG_INFO(0, phosphine_string)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phosphine_compileFile, 0, 0, 1)
    ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phosphine_setStyle, 0, 0, 1)
    ZEND_ARG_INFO(0, style)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phosphine_setIncludePath, 0, 0, 1)
    ZEND_ARG_INFO(0, include_path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phosphine_setPrecision, 0, 0, 1)
    ZEND_ARG_INFO(0, precision)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phosphine_setComments, 0, 0, 1)
    ZEND_ARG_INFO(0, comments)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phosphine_setIndent, 0, 0, 1)
    ZEND_ARG_INFO(0, indent)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phosphine_setEmbed, 0, 0, 1)
    ZEND_ARG_INFO(0, map_embed)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phosphine_setMapPath, 0, 0, 1)
    ZEND_ARG_INFO(0, map_path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phosphine_setMapRoot, 0, 0, 1)
    ZEND_ARG_INFO(0, map_root)
ZEND_END_ARG_INFO()
    
zend_function_entry phosphine_methods[] = {
    PHP_ME(phosphine,  __construct,       arginfo_phosphine_void,           ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(phosphine,  compile,           arginfo_phosphine_compile,        ZEND_ACC_PUBLIC)
    PHP_ME(phosphine,  compileFile,       arginfo_phosphine_compileFile,    ZEND_ACC_PUBLIC)
    PHP_ME(phosphine,  getStyle,          arginfo_phosphine_void,           ZEND_ACC_PUBLIC)
    PHP_ME(phosphine,  setStyle,          arginfo_phosphine_setStyle,       ZEND_ACC_PUBLIC)
    PHP_ME(phosphine,  getIncludePath,    arginfo_phosphine_void,           ZEND_ACC_PUBLIC)
    PHP_ME(phosphine,  setIncludePath,    arginfo_phosphine_setIncludePath, ZEND_ACC_PUBLIC)
    PHP_ME(phosphine,  getPrecision,      arginfo_phosphine_void,           ZEND_ACC_PUBLIC)
    PHP_ME(phosphine,  setPrecision,      arginfo_phosphine_setPrecision,   ZEND_ACC_PUBLIC)
    PHP_ME(phosphine,  getComments,       arginfo_phosphine_void,           ZEND_ACC_PUBLIC)
    PHP_ME(phosphine,  setComments,       arginfo_phosphine_setComments,    ZEND_ACC_PUBLIC)
    PHP_ME(phosphine,  getIndent,         arginfo_phosphine_void,           ZEND_ACC_PUBLIC)
    PHP_ME(phosphine,  setIndent,         arginfo_phosphine_setIndent,      ZEND_ACC_PUBLIC)
    PHP_ME(phosphine,  getEmbed,          arginfo_phosphine_void,           ZEND_ACC_PUBLIC)
    PHP_ME(phosphine,  setEmbed,          arginfo_phosphine_setEmbed,       ZEND_ACC_PUBLIC)
    PHP_ME(phosphine,  getMapPath,        arginfo_phosphine_void,           ZEND_ACC_PUBLIC)
    PHP_ME(phosphine,  setMapPath,        arginfo_phosphine_setMapPath,     ZEND_ACC_PUBLIC)
    PHP_ME(phosphine,  getMapRoot,        arginfo_phosphine_void,           ZEND_ACC_PUBLIC)
    PHP_ME(phosphine,  setMapRoot,        arginfo_phosphine_setMapRoot,     ZEND_ACC_PUBLIC)
    PHP_ME(phosphine,  getLibraryVersion, arginfo_phosphine_void,           ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_MALIAS(phosphine, compile_file, compileFile, NULL, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};


static PHP_MINIT_FUNCTION(phosphine)
{
    zend_class_entry ce;
    zend_class_entry exception_ce;

    INIT_CLASS_ENTRY(ce, "phosphine", phosphine_methods);

    phosphine_ce = zend_register_internal_class(&ce TSRMLS_CC);
    phosphine_ce->create_object = phosphine_create_handler;

    memcpy(&phosphine_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    #if ZEND_MODULE_API_NO > 20131226
    phosphine_handlers.offset = XtOffsetOf(struct phosphine_object, zo);
    phosphine_handlers.free_obj = phosphine_free_storage;
    #endif
    phosphine_handlers.clone_obj = NULL;

    INIT_CLASS_ENTRY(exception_ce, "phosphineException", NULL);

    #if ZEND_MODULE_API_NO > 20131226
    phosphine_exception_ce = zend_register_internal_class_ex(&exception_ce, phosphine_get_exception_base(TSRMLS_C));
    #endif

    #if ZEND_MODULE_API_NO <= 20131226
    phosphine_exception_ce = zend_register_internal_class_ex(&exception_ce, phosphine_get_exception_base(TSRMLS_C), NULL TSRMLS_CC);
    #endif

    REGISTER_STRING_CONSTANT("PHOSPHINE_FLAVOR", PHOSPHINE_FLAVOR, CONST_CS | CONST_PERSISTENT);

    return SUCCESS;
}

static PHP_MINFO_FUNCTION(phosphine)
{
    php_info_print_table_start();
    php_info_print_table_row(2, "phosphine support", "enabled");
    php_info_print_table_row(2, "version", PHOSPHINE_VERSION);
    php_info_print_table_row(2, "flavor", PHOSPHINE_FLAVOR);
    php_info_print_table_row(2, "libhydrogen version", HYDRO_VERSION_MAJOR+"."+HYDRO_VERSION_MINOR);
    php_info_print_table_end();
}

static zend_module_entry phosphine_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "phosphine",
    NULL,
    PHP_MINIT(phosphine),
    NULL,
    NULL,
    NULL,
    PHP_MINFO(phosphine),
#if ZEND_MODULE_API_NO >= 20010901
    PHOSPHINE_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};


#ifdef COMPILE_DL_phosphine
ZEND_GET_MODULE(phosphine)
#endif
