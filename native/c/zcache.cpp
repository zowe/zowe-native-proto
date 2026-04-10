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

#include "zcache.hpp"
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <algorithm>
#include <mutex>

// Global cache storage with mutex protection
static std::mutex uid_cache_mutex;
static std::unordered_map<uid_t, UidGidCache::CacheEntry> uid_cache;
static std::list<uid_t> uid_lru;

static std::mutex gid_cache_mutex;
static std::unordered_map<gid_t, UidGidCache::CacheEntry> gid_cache;
static std::list<gid_t> gid_lru;

std::string UidGidCache::get_username(uid_t uid) {
    std::lock_guard<std::mutex> lock(uid_cache_mutex);
    
    time_t now = time(nullptr);
    auto it = uid_cache.find(uid);
    
    // Check if cached and not expired
    if (it != uid_cache.end() && (now - it->second.timestamp) < CACHE_TTL) {
        // Move to front of LRU
        uid_lru.remove(uid);
        uid_lru.push_front(uid);
        return it->second.name;
    }
    
    // Cache miss or expired - lookup from system
    struct passwd *pw = getpwuid(uid);
    std::string name = (pw && pw->pw_name) ? pw->pw_name : "";
    
    // Evict oldest if cache is full
    if (uid_cache.size() >= MAX_CACHE_SIZE) {
        uid_t oldest = uid_lru.back();
        uid_lru.pop_back();
        uid_cache.erase(oldest);
    }
    
    // Add to cache
    uid_cache[uid] = CacheEntry(name, now);
    uid_lru.push_front(uid);
    
    return name;
}

std::string UidGidCache::get_groupname(gid_t gid) {
    std::lock_guard<std::mutex> lock(gid_cache_mutex);
    
    time_t now = time(nullptr);
    auto it = gid_cache.find(gid);
    
    // Check if cached and not expired
    if (it != gid_cache.end() && (now - it->second.timestamp) < CACHE_TTL) {
        // Move to front of LRU
        gid_lru.remove(gid);
        gid_lru.push_front(gid);
        return it->second.name;
    }
    
    // Cache miss or expired - lookup from system
    struct group *gr = getgrgid(gid);
    std::string name = (gr && gr->gr_name) ? gr->gr_name : "";
    
    // Evict oldest if cache is full
    if (gid_cache.size() >= MAX_CACHE_SIZE) {
        gid_t oldest = gid_lru.back();
        gid_lru.pop_back();
        gid_cache.erase(oldest);
    }
    
    // Add to cache
    gid_cache[gid] = CacheEntry(name, now);
    gid_lru.push_front(gid);
    
    return name;
}

void UidGidCache::evict_oldest_uid() {
    // This function is now unused since we inline the logic
}

void UidGidCache::evict_oldest_gid() {
    // This function is now unused since we inline the logic
}

void UidGidCache::update_uid_lru(uid_t uid) {
    // This function is now unused since we inline the logic
}

void UidGidCache::update_gid_lru(gid_t gid) {
    // This function is now unused since we inline the logic
}

void UidGidCache::clear_cache() {
    std::lock_guard<std::mutex> uid_lock(uid_cache_mutex);
    std::lock_guard<std::mutex> gid_lock(gid_cache_mutex);
    
    uid_cache.clear();
    gid_cache.clear();
    uid_lru.clear();
    gid_lru.clear();
}

void UidGidCache::get_cache_stats(size_t& uid_entries, size_t& gid_entries) {
    std::lock_guard<std::mutex> uid_lock(uid_cache_mutex);
    std::lock_guard<std::mutex> gid_lock(gid_cache_mutex);
    
    uid_entries = uid_cache.size();
    gid_entries = gid_cache.size();
}

// Global directory cache storage with mutex protection
static std::mutex dir_cache_mutex;
static std::unordered_map<SecureDirectoryCache::CacheKey, SecureDirectoryCache::CachedDirListing, SecureDirectoryCache::CacheKeyHash> dir_cache;
static std::list<SecureDirectoryCache::CacheKey> dir_lru;

bool SecureDirectoryCache::get_cached_listing(const std::string& dir_path, 
                                            std::vector<DirectoryEntry>& entries) {
    std::lock_guard<std::mutex> lock(dir_cache_mutex);
    
    entries.clear();
    
    CacheKey key = {dir_path, getuid(), getgid()};
    auto it = dir_cache.find(key);
    
    if (it == dir_cache.end()) {
        return false; // Cache miss
    }
    
    // Validate cache entry
    if (!is_cache_entry_valid(key, it->second)) {
        dir_cache.erase(it);
        dir_lru.remove(key);
        return false; // Invalid cache entry
    }
    
    // Update LRU and return cached data
    dir_lru.remove(key);
    dir_lru.push_front(key);
    entries = it->second.entries;
    return true;
}

void SecureDirectoryCache::cache_listing(const std::string& dir_path, 
                                       const std::vector<DirectoryEntry>& entries) {
    std::lock_guard<std::mutex> lock(dir_cache_mutex);
    
    // Get directory metadata for validation
    struct stat dir_stat;
    if (stat(dir_path.c_str(), &dir_stat) != 0) {
        return; // Can't stat directory, don't cache
    }
    
    CacheKey key = {dir_path, getuid(), getgid()};
    
    // Evict oldest if cache is full
    if (dir_cache.size() >= MAX_CACHE_ENTRIES) {
        CacheKey oldest = dir_lru.back();
        dir_lru.pop_back();
        dir_cache.erase(oldest);
    }
    
    // Create cache entry
    CachedDirListing cached;
    cached.entries = entries;
    cached.cache_time = time(nullptr);
    cached.dir_mtime = dir_stat.st_mtime;
    cached.dir_ctime = dir_stat.st_ctime;
    cached.cached_for_uid = getuid();
    
    dir_cache[key] = cached;
    dir_lru.push_front(key);
}

bool SecureDirectoryCache::is_cache_entry_valid(const CacheKey& key, const CachedDirListing& cached) {
    time_t now = time(nullptr);
    
    // Security check: ensure cache is for current user
    if (cached.cached_for_uid != getuid()) {
        return false;
    }
    
    // Time-based expiration
    if (now - cached.cache_time > CACHE_TTL) {
        return false;
    }
    
    // Directory modification check
    struct stat dir_stat;
    if (stat(key.path.c_str(), &dir_stat) == 0) {
        if (dir_stat.st_mtime != cached.dir_mtime || 
            dir_stat.st_ctime != cached.dir_ctime) {
            return false;
        }
    } else {
        // Can't stat directory, assume invalid
        return false;
    }
    
    return true;
}

void SecureDirectoryCache::evict_oldest_entry() {
    // This function is now unused since we inline the logic
}

void SecureDirectoryCache::update_lru(const CacheKey& key) {
    // This function is now unused since we inline the logic
}

void SecureDirectoryCache::clear_cache() {
    std::lock_guard<std::mutex> lock(dir_cache_mutex);
    
    dir_cache.clear();
    dir_lru.clear();
}

size_t SecureDirectoryCache::get_cache_size() {
    std::lock_guard<std::mutex> lock(dir_cache_mutex);
    return dir_cache.size();
}

// Global PDS cache storage with mutex protection
static std::mutex pds_cache_mutex;
static std::unordered_map<std::string, PDSMemberCache::CachedMemberList> pds_cache;
static std::list<std::string> pds_lru;

bool PDSMemberCache::get_cached_members(const std::string& dsn, std::vector<ZDSMem>& members) {
    std::lock_guard<std::mutex> lock(pds_cache_mutex);
    
    members.clear();
    
    auto it = pds_cache.find(dsn);
    if (it == pds_cache.end()) {
        return false; // Cache miss
    }
    
    // Validate cache entry
    if (!is_cache_entry_valid(dsn, it->second)) {
        pds_cache.erase(it);
        pds_lru.remove(dsn);
        return false; // Invalid cache entry
    }
    
    // Update LRU and return cached data
    pds_lru.remove(dsn);
    pds_lru.push_front(dsn);
    members = it->second.members;
    return true;
}

void PDSMemberCache::cache_members(const std::string& dsn, const std::vector<ZDSMem>& members) {
    std::lock_guard<std::mutex> lock(pds_cache_mutex);
    
    // Evict oldest if cache is full
    if (pds_cache.size() >= MAX_CACHE_ENTRIES) {
        std::string oldest = pds_lru.back();
        pds_lru.pop_back();
        pds_cache.erase(oldest);
    }
    
    // Create cache entry
    CachedMemberList cached;
    cached.members = members;
    cached.cache_time = time(nullptr);
    cached.cached_for_uid = getuid();
    cached.dataset_key = generate_dataset_key(dsn);
    
    pds_cache[dsn] = cached;
    pds_lru.push_front(dsn);
}

bool PDSMemberCache::is_cache_entry_valid(const std::string& dsn, const CachedMemberList& cached) {
    time_t now = time(nullptr);
    
    // Security check: ensure cache is for current user
    if (cached.cached_for_uid != getuid()) {
        return false;
    }
    
    // Time-based expiration
    if (now - cached.cache_time > CACHE_TTL) {
        return false;
    }
    
    // Dataset modification check (simplified - in real implementation would check catalog)
    std::string current_key = generate_dataset_key(dsn);
    if (current_key != cached.dataset_key) {
        return false;
    }
    
    return true;
}

std::string PDSMemberCache::generate_dataset_key(const std::string& dsn) {
    // Simplified key generation - in real implementation would use catalog info
    // For now, use current time as a placeholder (cache will expire via TTL)
    return dsn + "_" + std::to_string(time(nullptr) / CACHE_TTL);
}

void PDSMemberCache::evict_oldest_entry() {
    // This function is now unused since we inline the logic
}

void PDSMemberCache::update_lru(const std::string& dsn) {
    // This function is now unused since we inline the logic
}

void PDSMemberCache::clear_cache() {
    std::lock_guard<std::mutex> lock(pds_cache_mutex);
    
    pds_cache.clear();
    pds_lru.clear();
}

void PDSMemberCache::invalidate_dataset(const std::string& dsn) {
    std::lock_guard<std::mutex> lock(pds_cache_mutex);
    
    auto it = pds_cache.find(dsn);
    if (it != pds_cache.end()) {
        pds_cache.erase(it);
        pds_lru.remove(dsn);
    }
}

size_t PDSMemberCache::get_cache_size() {
    std::lock_guard<std::mutex> lock(pds_cache_mutex);
    return pds_cache.size();
}