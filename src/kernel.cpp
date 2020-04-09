// Copyright 2018 The clvk authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <algorithm>

#include "kernel.hpp"
#include "memory.hpp"

cl_ulong cvk_kernel::local_mem_size() const {
    cl_ulong ret = 0; // FIXME take the compile-time allocations into account

    for (uint32_t i = 0; i < m_args.size(); i++) {
        auto const& arg = m_args[i];
        if (arg.kind == kernel_argument_kind::local) {
            ret += m_argument_values->local_arg_size(i);
        }
    }

    return ret;
}

cl_int cvk_kernel::init() {
    cl_int errcode;
    m_entry_point = m_program->get_entry_point(m_name, &errcode);
    if (!m_entry_point) {
        return errcode;
    }

    // Store a copy of the arguments
    m_args = m_entry_point->args();

    // Init argument values
    m_argument_values = cvk_kernel_argument_values::create(
        this, m_entry_point->num_resources());
    if (m_argument_values == nullptr) {
        return CL_OUT_OF_RESOURCES;
    }

    return CL_SUCCESS;
}

VkPipeline
cvk_kernel::create_pipeline(const cvk_spec_constant_map& spec_constants) {
    return m_entry_point->create_pipeline(spec_constants);
}

cl_int cvk_kernel::set_arg(cl_uint index, size_t size, const void* value) {
    std::lock_guard<std::mutex> lock(m_lock);

    auto const& arg = m_args[index];

    return m_argument_values->set_arg(arg, size, value);
}
