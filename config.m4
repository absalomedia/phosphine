PHP_ARG_ENABLE(phosphine, [whether to enable phosphine extension],
[  --enable-phosphine      Enable phosphine extension], yes, yes)

if test "$PHP_PHOSPHINE" != "no"; then
  PHP_ADD_INCLUDE(lib/libhydrogen)
  PHP_ADD_LIBRARY_WITH_PATH(phosphine, lib/libexec, LIBHYDROGEN_SHARED_LIBADD)

  LDFLAGS="-lphosphine -L"`pwd`"/lib/libhydrogen -lstdc++"

  PHP_NEW_EXTENSION(phosphine, src/phosphine.c src/utilities.c, $ext_shared)
fi