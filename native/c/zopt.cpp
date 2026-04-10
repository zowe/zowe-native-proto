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
    // Check if fstatat is available (should be on z/OS USS)
    return true;
}

int zopt_batch_stat_directory(const std::string &dir_path, 
                             std::vector<BatchStatEntry> &entries,
                             bool follow_symlinks) {
    entries.clear();
    
#ifdef __MVS__
    // z/OS implementation - use traditional approach if advanced functions not available
    DIR *dir = opendir(dir_path.c_str());
    if (!dir) {
        return -1;
    }
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        BatchStatEntry stat_entry;
        stat_entry.name = entry->d_name;
        
        // Build full path for stat
        std::string full_path = dir_path + "/" + entry->d_name;
        
        // Use lstat or stat based on follow_symlinks
        int stat_result;
        if (follow_symlinks) {
            stat_result = stat(full_path.c_str(), &stat_entry.stats);
        } else {
            stat_result = lstat(full_path.c_str(), &stat_entry.stats);
        }
        
        if (stat_result == 0) {
            stat_entry.valid = true;
        } else {
            stat_entry.valid = false;
            memset(&stat_entry.stats, 0, sizeof(stat_entry.stats));
        }
        
        entries.push_back(stat_entry);
    }
    
    closedir(dir);
    return 0;
#else
    // POSIX implementation with fstatat
    int dir_fd = open(dir_path.c_str(), O_RDONLY);
    if (dir_fd == -1) {
        return -1;
    }
    
    DIR *dir = fdopendir(dir_fd);
    if (!dir) {
        close(dir_fd);
        return -1;
    }
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        BatchStatEntry stat_entry;
        stat_entry.name = entry->d_name;
        
        int flags = follow_symlinks ? 0 : AT_SYMLINK_NOFOLLOW;
        if (fstatat(dir_fd, entry->d_name, &stat_entry.stats, flags) == 0) {
            stat_entry.valid = true;
        } else {
            stat_entry.valid = false;
            memset(&stat_entry.stats, 0, sizeof(stat_entry.stats));
        }
        
        entries.push_back(stat_entry);
    }
    
    closedir(dir); // Also closes dir_fd
    return 0;
#endif
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