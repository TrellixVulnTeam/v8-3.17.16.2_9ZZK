// Copyright 2008 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <v8.h>
#include "cctest.h"
#include "debug.h"


CcTest* CcTest::last_ = NULL;


CcTest::CcTest(TestFunction* callback, const char* file, const char* name,
               const char* dependency, bool enabled)
    : callback_(callback), name_(name), dependency_(dependency), prev_(last_) {
  // Find the base name of this test (const_cast required on Windows).
  char *basename = strrchr(const_cast<char *>(file), '/');
  if (!basename) {
    basename = strrchr(const_cast<char *>(file), '\\');
  }
  if (!basename) {
    basename = v8::internal::StrDup(file);
  } else {
    basename = v8::internal::StrDup(basename + 1);
  }
  // Drop the extension, if there is one.
  char *extension = strrchr(basename, '.');
  if (extension) *extension = 0;
  // Install this test in the list of tests
  file_ = basename;
  enabled_ = enabled;
  prev_ = last_;
  last_ = this;
}


static void PrintTestList(CcTest* current) {
  if (current == NULL) return;
  PrintTestList(current->prev());
  if (current->dependency() != NULL) {
    printf("%s/%s<%s\n",
           current->file(), current->name(), current->dependency());
  } else {
    printf("%s/%s<\n", current->file(), current->name());
  }
}


v8::Isolate* CcTest::default_isolate_;

int main1(int argc, char* argv[]) {
  v8::internal::FlagList::SetFlagsFromCommandLine(&argc, argv, true);
  CcTest::set_default_isolate(v8::Isolate::GetCurrent());
  CHECK(CcTest::default_isolate() != NULL);
  int tests_run = 0;
  bool print_run_count = true;
  for (int i = 1; i < argc; i++) {
    char* arg = argv[i];
    if (strcmp(arg, "--list") == 0) {
      PrintTestList(CcTest::last());
      print_run_count = false;

    } else {
      char* arg_copy = v8::internal::StrDup(arg);
      char* testname = strchr(arg_copy, '/');
      if (testname) {
        // Split the string in two by nulling the slash and then run
        // exact matches.
        *testname = 0;
        char* file = arg_copy;
        char* name = testname + 1;
        CcTest* test = CcTest::last();
        while (test != NULL) {
          if (test->enabled()
              && strcmp(test->file(), file) == 0
              && strcmp(test->name(), name) == 0) {
            test->Run();
            tests_run++;
          }
          test = test->prev();
        }

      } else {
        // Run all tests with the specified file or test name.
        char* file_or_name = arg_copy;
        CcTest* test = CcTest::last();
        while (test != NULL) {
          if (test->enabled()
              && (strcmp(test->file(), file_or_name) == 0
                  || strcmp(test->name(), file_or_name) == 0)) {
            test->Run();
            tests_run++;
          }
          test = test->prev();
        }
      }
      v8::internal::DeleteArray<char>(arg_copy);
    }
  }
  if (print_run_count && tests_run != 1)
    printf("Ran %i tests.\n", tests_run);
  v8::V8::Dispose();
  return 0;
}

RegisterThreadedTest *RegisterThreadedTest::first_ = NULL;
int RegisterThreadedTest::count_ = 0;
