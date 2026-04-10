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

#define _POSIX_C_SOURCE 200809L
#define _LARGE_TIME_API
#define _POSIX_SOURCE
#include "zopt.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <cstring>

bool zopt_batch_stat_available() {
    // Batch stat is available but uses optimized individual stat calls on z/OS 2.5
    return true;
}

int zopt_batch_stat_directory(const std::string &dir_path, 
                             std::vector<BatchStatEntry> &entries,
                             bool follow_symlinks) {
    entries.clear();
    
    // Optimized implementation for z/OS 2.5 compatibility
    DIR *dir = opendir(dir_path.c_str());
    if (!dir) {
        return -1;
    }
    
    // Pre-allocate vector to reduce reallocations
    entries.reserve(128);
    
    // Pre-build directory path with separator to avoid repeated string operations
    std::string dir_with_sep = dir_path;
    if (!dir_with_sep.empty() && dir_with_sep.back() != '/') {
        dir_with_sep += '/';
    }
    const size_t base_path_len = dir_with_sep.length();
    
    // Pre-allocate path buffer to avoid repeated allocations
    std::string full_path;
    full_path.reserve(base_path_len + 256); // Reserve space for typical filename
    full_path = dir_with_sep;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        // Skip . and ..
        if (entry->d_name[0] == '.' && 
            (entry->d_name[1] == '\0' || 
             (entry->d_name[1] == '.' && entry->d_name[2] == '\0'))) {
            continue;
        }
        
        BatchStatEntry stat_entry;
        stat_entry.name = entry->d_name;
        
        // Efficient path construction - reuse buffer
        full_path.resize(base_path_len);
        full_path += entry->d_name;
        
        // Use lstat or stat based on follow_symlinks
        int stat_result;
        if (follow_symlinks) {
            stat_result = stat(full_path.c_str(), &stat_entry.stats);
        } else {
            stat_result = lstat(full_path.c_str(), &stat_entry.stats);
        }
        
        stat_entry.valid = (stat_result == 0);
        if (!stat_entry.valid) {
            memset(&stat_entry.stats, 0, sizeof(stat_entry.stats));
        }
        
        entries.push_back(stat_entry);
    }
    
    closedir(dir);
    return 0;
}

size_t zusf_estimate_directory_size(const std::string &dir_path) {
    DIR *dir = opendir(dir_path.c_str());
    if (!dir) {
        return 50; // Default estimate if can't open
    }
    
    size_t count = 0;
    struct dirent *entry;
    
    // Quick scan to count entries (limit to avoid long delays)
    while ((entry = readdir(dir)) != nullptr && count < 2000) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            count++;
        }
    }
    
    closedir(dir);
    return count;
}

bool ZOptConfig::initialized = false;
bool ZOptConfig::enable_buffer_pools = false;
size_t ZOptConfig::small_buffer_size = 4096;
size_t ZOptConfig::medium_buffer_size = 32768;
size_t ZOptConfig::large_buffer_size = 131072;

void ZOptConfig::initialize() {
    if (initialized) return;
    
    // Check environment variables
    const char* enable_env = getenv("ZOWE_NATIVE_ENABLE_BUFFER_POOLS");
    enable_buffer_pools = (enable_env && strcmp(enable_env, "1") == 0);
    
    const char* small_env = getenv("ZOWE_NATIVE_SMALL_BUFFER_SIZE");
    if (small_env) {
        small_buffer_size = static_cast<size_t>(atoi(small_env));
    }
    
    const char* medium_env = getenv("ZOWE_NATIVE_MEDIUM_BUFFER_SIZE");
    if (medium_env) {
        medium_buffer_size = static_cast<size_t>(atoi(medium_env));
    }
    
    const char* large_env = getenv("ZOWE_NATIVE_LARGE_BUFFER_SIZE");
    if (large_env) {
        large_buffer_size = static_cast<size_t>(atoi(large_env));
    }
    
    initialized = true;
}

bool ZOptConfig::buffer_pools_enabled() {
    if (!initialized) initialize();
    return enable_buffer_pools;
}

void ZOptConfig::get_buffer_pool_config(size_t& small_size, size_t& medium_size, size_t& large_size) {
    if (!initialized) initialize();
    small_size = small_buffer_size;
    medium_size = medium_buffer_size;
    large_size = large_buffer_size;
}