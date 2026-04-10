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

#ifndef ZOPT_HPP
#define ZOPT_HPP

#include <vector>
#include <string>
#include <sys/stat.h>
#include <dirent.h>
#include "zusftype.h"

struct BatchStatEntry {
    std::string name;
    struct stat stats;
    bool valid;
};

/**
 * Batch stat operation using fstatat for improved performance
 */
int zopt_batch_stat_directory(const std::string &dir_path, 
                             std::vector<BatchStatEntry> &entries,
                             bool follow_symlinks = false);

/**
 * Check if batch stat optimization is available
 */
bool zopt_batch_stat_available();

/**
 * Estimate the number of entries in a directory for memory pre-allocation
 */
size_t zusf_estimate_directory_size(const std::string &dir_path);

/**
 * Runtime configuration for optimizations
 */
class ZOptConfig {
public:
    /**
     * Check if buffer pools should be enabled
     */
    static bool buffer_pools_enabled();
    
    /**
     * Get buffer pool configuration if enabled
     */
    static void get_buffer_pool_config(size_t& small_size, size_t& medium_size, size_t& large_size);
    
    /**
     * Initialize configuration from environment variables
     */
    static void initialize();
    
private:
    static bool initialized;
    static bool enable_buffer_pools;
    static size_t small_buffer_size;
    static size_t medium_buffer_size; 
    static size_t large_buffer_size;
};

#endif