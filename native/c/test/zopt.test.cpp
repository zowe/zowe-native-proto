/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 */

#include "zopt.hpp"
#include "zusf.hpp"
#include "ztest.hpp"
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using namespace ztst;

void zopt_tests()
{
  describe("zopt batch stat tests", [&]() -> void
           {
        
        it("should batch stat directory entries", [&]() -> void {
            std::string test_dir = "/tmp/zopt_test_" + std::to_string(getpid());
            ZUSF zusf = {0};
            
            // Create test directory using zusf API
            if (zusf_create_uss_file_or_dir(&zusf, test_dir, 0755, true) != 0) {
                // Directory creation failed, skip test
                return;
            }
            
            // Create test files
            std::ofstream(test_dir + "/file1.txt") << "content1";
            std::ofstream(test_dir + "/file2.txt") << "content2";
            
            // Create subdirectory using zusf API
            std::string subdir = test_dir + "/subdir";
            zusf_create_uss_file_or_dir(&zusf, subdir, 0755, true);
            
            std::vector<BatchStatEntry> entries;
            int rc = zopt_batch_stat_directory(test_dir, entries);
            
            Expect(rc == 0);
            Expect(entries.size() == 3); // file1.txt, file2.txt, subdir
            
            // Verify all entries are valid
            for (const auto& entry : entries) {
                Expect(entry.valid);
                Expect(!entry.name.empty());
            }
            
            // Cleanup using zusf API
            ZUSF dzusf = {0};
            zusf_delete_uss_item(&dzusf, test_dir, true); // recursive delete
        });
        
        it("should fail for nonexistent directory", [&]() -> void {
            std::vector<BatchStatEntry> entries;
            
            int rc = zopt_batch_stat_directory("/nonexistent", entries);
            
            Expect(rc != 0);
            Expect(entries.empty());
        });
        
        it("should check batch stat availability", [&]() -> void {
            bool available = zopt_batch_stat_available();
            Expect(available); // Should be true on z/OS USS
        }); });
}