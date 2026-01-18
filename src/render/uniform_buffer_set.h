#pragma once
#include "render/uniform_buffer.h"
#include "core/ref.h"


namespace Mint {

    // should it be inherited from ref?
    // it should use factory model
    class UniformBufferSet : public Refcounter{
    public:
        // Set & Get Method?

        static Ref<UniformBufferSet> Create();
    }
}