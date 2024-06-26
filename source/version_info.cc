//
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Copyright (C) 2024 by the ryujin authors
//

#include "version_info.h"
#include "compile_time_options.h"

#include <deal.II/base/revision.h>
#include <deal.II/base/vectorization.h>

#include <iomanip>
#include <iostream>

namespace ryujin
{
  void print_revision_and_version(std::ostream &stream)
  {
    /* clang-format off */
    stream << std::endl;
    stream << "###" << std::endl;
    stream << "#" << std::endl;
    stream << "# deal.II version " << std::setw(8) << DEAL_II_PACKAGE_VERSION
            << "  -  " << DEAL_II_GIT_REVISION << std::endl;
    stream << "# ryujin  version " << std::setw(8) << RYUJIN_VERSION
            << "  -  " << RYUJIN_GIT_REVISION << std::endl;
    stream << "#" << std::endl;
    stream << "###" << std::endl;
    /* clang-format on */
  }
} // namespace ryujin
