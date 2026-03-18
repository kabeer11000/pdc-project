#include "cuckoo_filter.h"

// ─── Hash Functions ───────────────────────────────────────────────────────────

// FNV-1a 32-bit — lightweight and suitable for ESP32
static uint32_t fnv1a(const char* data) {
    uint32_t h = 2166136261u;
    while (*data) {
        h ^= (uint8_t)(*data++);
        h *= 16777619u;
    }
    return h;
}

// Primary bucket index derived from item
static uint32_t bucket1(const char* item) {
    return fnv1a(item) % BUCKET_COUNT;
}

// Fingerprint: non-zero 8-bit hash of the item
static Fingerprint fingerprint(const char* item) {
    Fingerprint fp = (Fingerprint)(fnv1a(item) & 0xFF);
    return fp == 0 ? 1 : fp;   // fingerprint must never be 0
}

// Alternate bucket via XOR — allows recovery without storing original item
// Property: hash2(hash2(i, fp), fp) == i   (self-inverse)
static uint32_t bucket2(uint32_t index, Fingerprint fp) {
    return ((uint32_t)(index ^ ((uint32_t)fp * 0x5bd1e995u))) % BUCKET_COUNT;
}

// ─── Bucket Helpers ───────────────────────────────────────────────────────────

static bool bucketInsert(Bucket* b, Fingerprint fp) {
    if (b->count >= BUCKET_SIZE) return false;
    b->slots[b->count++] = fp;
    return true;
}

static bool bucketContains(const Bucket* b, Fingerprint fp) {
    for (uint8_t i = 0; i < b->count; i++) {
        if (b->slots[i] == fp) return true;
    }
    return false;
}

static bool bucketRemove(Bucket* b, Fingerprint fp) {
    for (uint8_t i = 0; i < b->count; i++) {
        if (b->slots[i] == fp) {
            // Replace with last element and shrink
            b->slots[i] = b->slots[--b->count];
            return true;
        }
    }
    return false;
}

// Evict a random slot from bucket, store its fingerprint, insert new fp
static Fingerprint kickout(Bucket* b, Fingerprint fp) {
    uint8_t victim = random(BUCKET_SIZE);
    Fingerprint evicted = b->slots[victim];
    b->slots[victim] = fp;
    return evicted;
}

// ─── Filter Initialisation ────────────────────────────────────────────────────

void filterInit(CuckooFilter* cf) {
    memset(cf, 0, sizeof(CuckooFilter));
}

// ─── Core Operations ──────────────────────────────────────────────────────────

/**
 * INSERT
 * Returns true on success, false if the filter is too full.
 * False positive rate ≈ 2*BUCKET_SIZE / 2^FP_BITS  (~3% at default settings)
 */
bool filterInsert(CuckooFilter* cf, const char* item) {
    Fingerprint fp = fingerprint(item);
    uint32_t    i1 = bucket1(item);
    uint32_t    i2 = bucket2(i1, fp);

    // Try direct insertion into either bucket
    if (bucketInsert(&cf->buckets[i1], fp)) return true;
    if (bucketInsert(&cf->buckets[i2], fp)) return true;

    // Cuckoo kick-out loop
    uint32_t i = (random(2) == 0) ? i1 : i2;
    for (int k = 0; k < MAX_KICKS; k++) {
        fp = kickout(&cf->buckets[i], fp);   // evict an occupant
        i  = bucket2(i, fp);                 // relocate evictee to alt bucket
        if (bucketInsert(&cf->buckets[i], fp)) return true;
    }

    // Filter is too full — caller should report failure to master node
    Serial.println("[CuckooFilter] Insert failed: filter full");
    return false;
}

/**
 * LOOKUP
 * May return true for items never inserted (false positive).
 * Will never return false for items that were inserted (no false negatives).
 */
bool filterLookup(const CuckooFilter* cf, const char* item) {
    Fingerprint fp = fingerprint(item);
    uint32_t    i1 = bucket1(item);
    uint32_t    i2 = bucket2(i1, fp);
    return bucketContains(&cf->buckets[i1], fp) ||
           bucketContains(&cf->buckets[i2], fp);
}

/**
 * DELETE
 * Only delete items that were actually inserted — deleting a non-member
 * can corrupt the filter by incorrectly removing a true fingerprint.
 */
bool filterDelete(CuckooFilter* cf, const char* item) {
    Fingerprint fp = fingerprint(item);
    uint32_t    i1 = bucket1(item);
    uint32_t    i2 = bucket2(i1, fp);
    if (bucketRemove(&cf->buckets[i1], fp)) return true;
    if (bucketRemove(&cf->buckets[i2], fp)) return true;
    return false;   // item not found
}

// ─── Test Harness ─────────────────────────────────────────────────────────────

/**
 * Inserts `insertCount` items, then queries `queryCount` items that were
 * never inserted and counts false positives.
 *
 * Expected FP rate ≈ 2*BUCKET_SIZE / 2^FP_BITS
 * With defaults (BUCKET_SIZE=4, FP_BITS=8): ≈ 3.1%
 */
void runTestHarness(CuckooFilter* cf) {
    const int insertCount = 800;
    const int queryCount  = 1000;
    int       insertFails = 0;
    int       falsePos    = 0;

    Serial.println("=== Cuckoo Filter Test Harness ===");

    // INSERT phase
    for (int i = 0; i < insertCount; i++) {
        char key[16];
        snprintf(key, sizeof(key), "item_%d", i);
        if (!filterInsert(cf, key)) insertFails++;
    }
    Serial.printf("Inserted %d items, %d failures\n", insertCount, insertFails);

    // LOOKUP phase — query items that were never inserted
    for (int i = insertCount; i < insertCount + queryCount; i++) {
        char key[16];
        snprintf(key, sizeof(key), "item_%d", i);
        if (filterLookup(cf, key)) falsePos++;
    }

    float fpRate = (float)falsePos / queryCount * 100.0f;
    Serial.printf("False positives: %d / %d  (%.2f%%)\n",
                  falsePos, queryCount, fpRate);
    Serial.println("==================================");
}
