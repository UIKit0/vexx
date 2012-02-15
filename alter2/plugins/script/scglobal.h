#ifndef SCRIPT_GLOBAL_H
#define SCRIPT_GLOBAL_H

#include "sglobal.h"
#include <QtCore/qglobal.h>
#include "XProfiler"

#if defined(SCRIPT_BUILD)
# define SCRIPT_EXPORT Q_DECL_EXPORT
#else
# define SCRIPT_EXPORT Q_DECL_IMPORT
#endif

#define ScriptProfileScope 502
#define ScProfileFunction XProfileFunctionBase(ScriptProfileScope)
#define ScProfileScopedBlock(mess) XProfileScopedBlockBase(ScriptProfileScope, mess)

S_DEFINE_INTERFACE_TYPE(ScriptInterface, 100)

#endif // SCRIPT_GLOBAL_H
