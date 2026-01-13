#include "render/shader.h"
#include "render/render_system.h"

namespace Mint {


    // RenderPass 纯虚类实现?派生对应的具体Pass？
    // 还是通过指定RenderPassSpecification, 来创建具体的Pass？

    // 考虑数据驱动, 即AddPass()的形式？
    class RenderPass : public RefCounter {
        public:
            virtual ~RenderPass() = default;
        private:
    };



}