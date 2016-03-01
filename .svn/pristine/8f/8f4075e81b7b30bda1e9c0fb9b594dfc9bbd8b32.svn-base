# ##########################################################################
# This file is from the SeqAn project, www.seqan.de
# Distributed under 3-clause BSD license
# ##########################################################################
# Check availability of C++ TR1 contents.
#
# Sets the following variables:
#
# TR1_SHARED_PTR_FOUND          -- std::tr1::shared_ptr1<T> available
# TR1_SHARED_PTR_USE_TR1_MEMORY -- #include <tr1/memory>
# TR1_SHARED_PTR_USE_MEMORY     -- #include <memory>

# We need to have at least this version to support the VERSION_LESS argument to 'if' (2.6.2) and unset (2.6.3)
cmake_policy(PUSH)
cmake_minimum_required(VERSION 2.6.3)
cmake_policy(POP)

include (CheckCXXSourceCompiles)

# ---------------------------------------------------------------------------
# std::tr1::shared_ptr<T>
# ---------------------------------------------------------------------------

check_cxx_source_compiles(
    "
        #include <memory>
        int main() {
            std::shared_ptr<int> ptr;
            return 0;
        }
    "
    SHARED_PTR_USE_MEMORY)
check_cxx_source_compiles(
    "
        #include <tr1/memory>
        int main() {
            std::tr1::shared_ptr<int> ptr;
            return 0;
        }
    "
    TR1_SHARED_PTR_USE_TR1_MEMORY)
check_cxx_source_compiles(
    "
        #include <memory>
        int main() {
            std::tr1::shared_ptr<int> ptr;
            return 0;
        }
    "
    TR1_SHARED_PTR_USE_MEMORY)

set (SHARED_PTR -NOTFOUND)
if (SHARED_PTR_USE_MEMORY)
  set (SHARED_PTR_FOUND TRUE)
endif ()
if (TR1_SHARED_PTR_USE_TR1_MEMORY)
  set (SHARED_PTR_FOUND TRUE)
endif ()
if (TR1_SHARED_PTR_USE_MEMORY)
  set (SHARED_PTR_FOUND TRUE)
endif ()

mark_as_advanced (SHARED_PTR_FOUND)
mark_as_advanced (SHARED_PTR_USE_MEMORY)
mark_as_advanced (TR1_SHARED_PTR_USE_TR1_MEMORY)
mark_as_advanced (TR1_SHARED_PTR_USE_MEMORY)
