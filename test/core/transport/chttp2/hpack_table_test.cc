/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "src/core/ext/transport/chttp2/transport/hpack_table.h"

#include <stdio.h>
#include <string.h>

#include <string>

#include "absl/strings/str_cat.h"

#include <grpc/grpc.h>
#include <grpc/support/alloc.h>
#include <grpc/support/log.h>

#include "src/core/lib/gpr/string.h"
#include "src/core/lib/iomgr/exec_ctx.h"
#include "src/core/lib/slice/slice_internal.h"
#include "test/core/util/test_config.h"

#define LOG_TEST(x) gpr_log(GPR_INFO, "%s", x)

using grpc_core::HPackTable;

static void assert_str(const HPackTable* /*tbl*/, grpc_slice mdstr,
                       const char* str) {
  GPR_ASSERT(grpc_slice_str_cmp(mdstr, str) == 0);
}

static void assert_index(const HPackTable* tbl, uint32_t idx, const char* key,
                         const char* value) {
  grpc_mdelem md = tbl->Peek(idx);
  assert_str(tbl, GRPC_MDKEY(md), key);
  assert_str(tbl, GRPC_MDVALUE(md), value);
}

static void test_static_lookup(void) {
  grpc_core::ExecCtx exec_ctx;
  HPackTable tbl;

  LOG_TEST("test_static_lookup");
  assert_index(&tbl, 1, ":authority", "");
  assert_index(&tbl, 2, ":method", "GET");
  assert_index(&tbl, 3, ":method", "POST");
  assert_index(&tbl, 4, ":path", "/");
  assert_index(&tbl, 5, ":path", "/index.html");
  assert_index(&tbl, 6, ":scheme", "http");
  assert_index(&tbl, 7, ":scheme", "https");
  assert_index(&tbl, 8, ":status", "200");
  assert_index(&tbl, 9, ":status", "204");
  assert_index(&tbl, 10, ":status", "206");
  assert_index(&tbl, 11, ":status", "304");
  assert_index(&tbl, 12, ":status", "400");
  assert_index(&tbl, 13, ":status", "404");
  assert_index(&tbl, 14, ":status", "500");
  assert_index(&tbl, 15, "accept-charset", "");
  assert_index(&tbl, 16, "accept-encoding", "gzip, deflate");
  assert_index(&tbl, 17, "accept-language", "");
  assert_index(&tbl, 18, "accept-ranges", "");
  assert_index(&tbl, 19, "accept", "");
  assert_index(&tbl, 20, "access-control-allow-origin", "");
  assert_index(&tbl, 21, "age", "");
  assert_index(&tbl, 22, "allow", "");
  assert_index(&tbl, 23, "authorization", "");
  assert_index(&tbl, 24, "cache-control", "");
  assert_index(&tbl, 25, "content-disposition", "");
  assert_index(&tbl, 26, "content-encoding", "");
  assert_index(&tbl, 27, "content-language", "");
  assert_index(&tbl, 28, "content-length", "");
  assert_index(&tbl, 29, "content-location", "");
  assert_index(&tbl, 30, "content-range", "");
  assert_index(&tbl, 31, "content-type", "");
  assert_index(&tbl, 32, "cookie", "");
  assert_index(&tbl, 33, "date", "");
  assert_index(&tbl, 34, "etag", "");
  assert_index(&tbl, 35, "expect", "");
  assert_index(&tbl, 36, "expires", "");
  assert_index(&tbl, 37, "from", "");
  assert_index(&tbl, 38, "host", "");
  assert_index(&tbl, 39, "if-match", "");
  assert_index(&tbl, 40, "if-modified-since", "");
  assert_index(&tbl, 41, "if-none-match", "");
  assert_index(&tbl, 42, "if-range", "");
  assert_index(&tbl, 43, "if-unmodified-since", "");
  assert_index(&tbl, 44, "last-modified", "");
  assert_index(&tbl, 45, "link", "");
  assert_index(&tbl, 46, "location", "");
  assert_index(&tbl, 47, "max-forwards", "");
  assert_index(&tbl, 48, "proxy-authenticate", "");
  assert_index(&tbl, 49, "proxy-authorization", "");
  assert_index(&tbl, 50, "range", "");
  assert_index(&tbl, 51, "referer", "");
  assert_index(&tbl, 52, "refresh", "");
  assert_index(&tbl, 53, "retry-after", "");
  assert_index(&tbl, 54, "server", "");
  assert_index(&tbl, 55, "set-cookie", "");
  assert_index(&tbl, 56, "strict-transport-security", "");
  assert_index(&tbl, 57, "transfer-encoding", "");
  assert_index(&tbl, 58, "user-agent", "");
  assert_index(&tbl, 59, "vary", "");
  assert_index(&tbl, 60, "via", "");
  assert_index(&tbl, 61, "www-authenticate", "");
}

static void test_many_additions(void) {
  HPackTable tbl;
  int i;

  LOG_TEST("test_many_additions");

  grpc_core::ExecCtx exec_ctx;

  for (i = 0; i < 100000; i++) {
    grpc_mdelem elem;
    std::string key = absl::StrCat("K:", i);
    std::string value = absl::StrCat("VALUE:", i);
    elem = grpc_mdelem_from_slices(grpc_slice_from_cpp_string(key),
                                   grpc_slice_from_cpp_string(value));
    GPR_ASSERT(tbl.Add(elem) == GRPC_ERROR_NONE);
    GRPC_MDELEM_UNREF(elem);
    assert_index(&tbl, 1 + HPackTable::kLastStaticEntry, key.c_str(),
                 value.c_str());
    if (i) {
      std::string key = absl::StrCat("K:", i - 1);
      std::string value = absl::StrCat("VALUE:", i - 1);
      assert_index(&tbl, 2 + HPackTable::kLastStaticEntry, key.c_str(),
                   value.c_str());
    }
  }
}

int main(int argc, char** argv) {
  grpc::testing::TestEnvironment env(argc, argv);
  grpc_init();
  test_static_lookup();
  test_many_additions();
  grpc_shutdown();
  return 0;
}
