// plc-security.h - Security, authentication, and session management
//
// Copyright (c) 2022-2026 J.Vovk
//
// This file is part of VovkPLCRuntime.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#ifdef PLCRUNTIME_TRANSPORT

#include <Arduino.h>
#include "../arithmetics/crc8.h"

// ============================================================================
// Permission Flags
// ============================================================================

/**
 * @brief Permission flags for PLC operations (bitmask)
 */
enum PLCPermission : uint16_t {
    PERM_NONE           = 0x0000,
    PERM_READ_STATUS    = 0x0001,  // Read device info, health stats
    PERM_READ_MEMORY    = 0x0002,  // Read PLC memory
    PERM_READ_PROGRAM   = 0x0004,  // Upload/read program
    PERM_WRITE_MEMORY   = 0x0010,  // Write to memory (force values)
    PERM_WRITE_PROGRAM  = 0x0020,  // Download programs
    PERM_CONTROL_RUN    = 0x0040,  // Start/stop/reset PLC
    PERM_CONFIG_DEVICE  = 0x0080,  // Change device settings
    PERM_ADMIN          = 0x0100,  // Manage users, security settings
    
    // Preset combinations for common roles
    PERM_MONITOR    = PERM_READ_STATUS | PERM_READ_MEMORY,
    PERM_OPERATOR   = PERM_MONITOR | PERM_WRITE_MEMORY | PERM_CONTROL_RUN,
    PERM_PROGRAMMER = PERM_OPERATOR | PERM_READ_PROGRAM | PERM_WRITE_PROGRAM,
    PERM_FULL       = 0xFFFF
};

/**
 * @brief Security levels for transports
 */
enum PLCSecurity : uint8_t {
    PLC_SEC_NONE = 0,       // No authentication (trusted local connection)
    PLC_SEC_PASSWORD = 1,   // Simple password authentication
    PLC_SEC_CHALLENGE = 2,  // Challenge-response (HMAC-based)
};

// ============================================================================
// Configuration Limits
// ============================================================================

#ifndef PLCRUNTIME_MAX_SESSIONS
#define PLCRUNTIME_MAX_SESSIONS 4
#endif

#ifndef PLCRUNTIME_MAX_USERS
#define PLCRUNTIME_MAX_USERS 4
#endif

#ifndef PLCRUNTIME_SESSION_TIMEOUT_MS
#define PLCRUNTIME_SESSION_TIMEOUT_MS 300000  // 5 minutes default
#endif

// ============================================================================
// Session Management
// ============================================================================

/**
 * @brief Session state for authenticated connections
 */
struct PLCSession {
    uint32_t token = 0;             // Random session token
    uint32_t lastActivity = 0;      // millis() of last command
    uint32_t timeout = PLCRUNTIME_SESSION_TIMEOUT_MS;
    uint16_t permissions = PERM_NONE;  // Granted permissions
    uint8_t transportIndex = 0;     // Which transport this session is on
    uint8_t failedAttempts = 0;     // For lockout
    bool authenticated = false;
    
    bool isValid() const {
        if (!authenticated) return false;
        if (token == 0) return false;
        uint32_t elapsed = millis() - lastActivity;
        if (elapsed > timeout) return false;
        return true;
    }
    
    void touch() { 
        lastActivity = millis(); 
    }
    
    void invalidate() {
        token = 0;
        authenticated = false;
        permissions = PERM_NONE;
        failedAttempts = 0;
    }
    
    bool hasPermission(PLCPermission perm) const {
        if (!isValid()) return false;
        return (permissions & perm) != 0;
    }
};

/**
 * @brief Session manager - handles multiple concurrent sessions
 */
class PLCSessionManager {
private:
    PLCSession _sessions[PLCRUNTIME_MAX_SESSIONS];
    uint8_t _maxFailedAttempts = 5;
    uint32_t _lockoutDuration = 60000;  // 1 minute
    uint32_t _lockoutUntil = 0;
    uint32_t _sessionTimeout = PLCRUNTIME_SESSION_TIMEOUT_MS;
    
public:
    void setMaxAttempts(uint8_t max) { _maxFailedAttempts = max; }
    void setLockoutDuration(uint32_t ms) { _lockoutDuration = ms; }
    void setSessionTimeout(uint32_t ms) { _sessionTimeout = ms; }
    
    bool isLockedOut() const {
        return _lockoutUntil > 0 && millis() < _lockoutUntil;
    }
    
    PLCSession* createSession(uint8_t transportIndex) {
        // Check lockout
        if (isLockedOut()) {
            return nullptr;
        }
        
        // Find free slot or reuse expired
        for (int i = 0; i < PLCRUNTIME_MAX_SESSIONS; i++) {
            if (!_sessions[i].isValid()) {
                _sessions[i].token = generateToken();
                _sessions[i].transportIndex = transportIndex;
                _sessions[i].lastActivity = millis();
                _sessions[i].timeout = _sessionTimeout;
                _sessions[i].failedAttempts = 0;
                _sessions[i].authenticated = false;
                _sessions[i].permissions = PERM_NONE;
                return &_sessions[i];
            }
        }
        return nullptr;  // No slots available
    }
    
    PLCSession* findSession(uint32_t token) {
        for (int i = 0; i < PLCRUNTIME_MAX_SESSIONS; i++) {
            if (_sessions[i].token == token && _sessions[i].isValid()) {
                return &_sessions[i];
            }
        }
        return nullptr;
    }
    
    PLCSession* findByTransport(uint8_t transportIndex) {
        for (int i = 0; i < PLCRUNTIME_MAX_SESSIONS; i++) {
            if (_sessions[i].transportIndex == transportIndex && _sessions[i].isValid()) {
                return &_sessions[i];
            }
        }
        return nullptr;
    }
    
    void recordFailedAttempt(PLCSession* session) {
        if (session) {
            session->failedAttempts++;
            if (session->failedAttempts >= _maxFailedAttempts) {
                _lockoutUntil = millis() + _lockoutDuration;
                invalidateAll();
            }
        }
    }
    
    void invalidateAll() {
        for (int i = 0; i < PLCRUNTIME_MAX_SESSIONS; i++) {
            _sessions[i].invalidate();
        }
    }
    
    void cleanup() {
        // Called periodically to expire old sessions
        for (int i = 0; i < PLCRUNTIME_MAX_SESSIONS; i++) {
            if (_sessions[i].authenticated && !_sessions[i].isValid()) {
                _sessions[i].invalidate();
            }
        }
        
        // Clear lockout if expired
        if (_lockoutUntil > 0 && millis() >= _lockoutUntil) {
            _lockoutUntil = 0;
        }
    }
    
private:
    uint32_t generateToken() {
        // Simple PRNG - consider hardware RNG on ESP32
        uint32_t t = micros();
        uint32_t r = (uint32_t)random(0x10000000, 0x7FFFFFFF);
        return r ^ t ^ (t << 16);
    }
};

// ============================================================================
// User Credentials
// ============================================================================

/**
 * @brief User credential storage
 */
struct PLCCredential {
    const char* username = nullptr;
    uint8_t passwordHash[8] = {0};  // Hashed password
    uint16_t permissions = PERM_NONE;
    bool enabled = false;
};

/**
 * @brief Authentication provider - manages users and validates credentials
 */
class PLCAuthProvider {
private:
    PLCCredential _users[PLCRUNTIME_MAX_USERS];
    uint8_t _userCount = 0;
    
public:
    /**
     * @brief Add a user with credentials
     * @param username Username (pointer to static string)
     * @param password Password (will be hashed, not stored)
     * @param perms Permission flags
     * @return true if user was added
     */
    bool addUser(const char* username, const char* password, uint16_t perms) {
        if (_userCount >= PLCRUNTIME_MAX_USERS) return false;
        if (!username || !password) return false;
        
        _users[_userCount].username = username;
        hashPassword(password, _users[_userCount].passwordHash);
        _users[_userCount].permissions = perms;
        _users[_userCount].enabled = true;
        _userCount++;
        return true;
    }
    
    /**
     * @brief Authenticate with username and password
     * @param username Username to check
     * @param password Password to verify
     * @return Permission flags if authenticated, PERM_NONE if failed
     */
    uint16_t authenticate(const char* username, const char* password) {
        if (!username || !password) return PERM_NONE;
        
        uint8_t hash[8];
        hashPassword(password, hash);
        
        for (uint8_t i = 0; i < _userCount; i++) {
            if (_users[i].enabled && 
                _users[i].username &&
                strcmp(_users[i].username, username) == 0 &&
                memcmp(_users[i].passwordHash, hash, 8) == 0) {
                return _users[i].permissions;
            }
        }
        return PERM_NONE;
    }
    
    /**
     * @brief Generate a random challenge for challenge-response auth
     * @param challenge Output buffer
     * @param len Buffer length
     */
    void generateChallenge(uint8_t* challenge, size_t len) {
        for (size_t i = 0; i < len; i++) {
            challenge[i] = random(256);
        }
    }
    
    /**
     * @brief Verify a challenge-response
     * @param username Username
     * @param challenge The challenge that was sent
     * @param challengeLen Challenge length
     * @param response The response received
     * @param responseLen Response length
     * @return Permission flags if verified, PERM_NONE if failed
     */
    uint16_t verifyResponse(const char* username, 
                            const uint8_t* challenge, size_t challengeLen,
                            const uint8_t* response, size_t responseLen) {
        for (uint8_t i = 0; i < _userCount; i++) {
            if (_users[i].enabled && 
                _users[i].username &&
                strcmp(_users[i].username, username) == 0) {
                // Compute expected response: HMAC(password_hash, challenge)
                uint8_t expected[8];
                computeHMAC(_users[i].passwordHash, 8, challenge, challengeLen, expected);
                
                size_t cmpLen = (responseLen < 8) ? responseLen : 8;
                if (memcmp(expected, response, cmpLen) == 0) {
                    return _users[i].permissions;
                }
            }
        }
        return PERM_NONE;
    }
    
    uint8_t userCount() const { return _userCount; }
    
private:
    /**
     * @brief Hash a password for storage
     * Simple multi-round CRC-based hash suitable for embedded
     */
    void hashPassword(const char* password, uint8_t* out) {
        size_t len = strlen(password);
        
        // Multiple rounds with position-dependent mixing
        for (int round = 0; round < 8; round++) {
            uint8_t crc = 0x5A ^ round;
            for (size_t i = 0; i < len; i++) {
                crc8_simple(crc, password[i]);
                crc8_simple(crc, round);
                crc8_simple(crc, (uint8_t)(i & 0xFF));
            }
            out[round] = crc;
        }
    }
    
    /**
     * @brief Compute HMAC for challenge-response
     * Simplified HMAC suitable for embedded systems
     */
    void computeHMAC(const uint8_t* key, size_t keyLen,
                     const uint8_t* data, size_t dataLen,
                     uint8_t* out) {
        // Inner hash: H(K ^ ipad || data)
        uint8_t crc = 0;
        for (size_t i = 0; i < keyLen; i++) {
            crc8_simple(crc, key[i] ^ 0x36);
        }
        for (size_t i = 0; i < dataLen; i++) {
            crc8_simple(crc, data[i]);
        }
        out[0] = crc;
        
        // Outer hash: H(K ^ opad || inner)
        crc = 0;
        for (size_t i = 0; i < keyLen; i++) {
            crc8_simple(crc, key[i] ^ 0x5C);
        }
        crc8_simple(crc, out[0]);
        out[1] = crc;
        
        // Fill remaining bytes with more mixing
        for (int i = 2; i < 8; i++) {
            crc8_simple(crc, out[i-1] ^ key[i % keyLen] ^ data[i % dataLen]);
            out[i] = crc;
        }
    }
};

#endif // PLCRUNTIME_TRANSPORT
