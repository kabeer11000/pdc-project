#pragma once
#include <Arduino.h>

// ─── Configuration ────────────────────────────────────────────────────────────
#define BUCKET_COUNT  256   // Number of buckets (must be power of 2)
#define BUCKET_SIZE   4     // Slots per bucket
#define FP_BITS       8     // Fingerprint size in bits
#define MAX_KICKS     500   // Max kick-out relocations before insert fails

// ─── Types ────────────────────────────────────────────────────────────────────
typedef uint8_t Fingerprint;

struct Bucket {
    Fingerprint slots[BUCKET_SIZE];
    uint8_t     count;
};

struct CuckooFilter {
    Bucket buckets[BUCKET_COUNT];
};

// ─── Message Types (ESP-NOW / WebSocket) ─────────────────────────────────────
enum OpType : uint8_t {
    OP_INSERT = 0,
    OP_DELETE,
    OP_LOOKUP,
    OP_SYNC
};

struct FilterMessage {
    OpType  op;
    char    item[32];   // item name (or use fp + bucket index for compactness)
    uint8_t nodeId;
    bool    result;     // used in LOOKUP response from slave → master
};
