#ifndef COMPACT_GLOBAL_H
#define COMPACT_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(COMPACT_LIBRARY)
#  define COMPACT_EXPORT Q_DECL_EXPORT
#else
#  define COMPACT_EXPORT Q_DECL_IMPORT
#endif

#endif // COMPACT_GLOBAL_H
