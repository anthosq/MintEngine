#pragma once

//temporary macros
#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

// basic mappers