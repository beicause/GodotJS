﻿#ifndef GODOTJS_CONSOLE_OUTPUT_H
#define GODOTJS_CONSOLE_OUTPUT_H

#include "../compat/jsb_compat.h"
#include "jsb_log_severity.h"

namespace jsb
{
    namespace internal
    {
        class IConsoleOutput
        {
        public:
            virtual void write(ELogSeverity::Type p_severity, const String& p_text) = 0;

            static void internal_write(ELogSeverity::Type p_severity, const String& p_text);

            IConsoleOutput();
            virtual ~IConsoleOutput();
        };
    }
}
#endif
