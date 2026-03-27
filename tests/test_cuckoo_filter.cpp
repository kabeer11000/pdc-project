/**
 * Test Cases for Distributed Cuckoo Filter System
 * 
 * This file contains unit tests for:
 * 1. Cuckoo Filter operations (Hamza's implementation)
 * 2. WebSocket communication (Kabeer's implementation)
 * 3. Integration tests
 * 
 * Run tests using PlatformIO:
 *   pio test -e native
 */

#include <Arduino.h>
#include <unity.h>
#include "cuckoo_filter/cuckoo_filter.h"

// ─── Cuckoo Filter Tests (Hamza's Implementation) ─────────────────────────────

CuckooFilter testFilter;

void setUp(void) {
    // Initialize filter before each test
    filterInit(&testFilter);
}

void tearDown(void) {
    // Clean up if needed
}

/**
 * Test: Empty filter lookup should return false
 */
void test_cuckoo_empty_lookup(void) {
    filterInit(&testFilter);
    
    bool result = filterLookup(&testFilter, "nonexistent");
    
    TEST_ASSERT_FALSE(result);
}

/**
 * Test: Insert then lookup should return true
 */
void test_cuckoo_insert_and_lookup(void) {
    filterInit(&testFilter);
    
    bool inserted = filterInsert(&testFilter, "test_item");
    bool found = filterLookup(&testFilter, "test_item");
    
    TEST_ASSERT_TRUE(inserted);
    TEST_ASSERT_TRUE(found);
}

/**
 * Test: Delete existing item should return true
 */
void test_cuckoo_delete_existing(void) {
    filterInit(&testFilter);
    
    filterInsert(&testFilter, "to_delete");
    bool deleted = filterDelete(&testFilter, "to_delete");
    bool stillExists = filterLookup(&testFilter, "to_delete");
    
    TEST_ASSERT_TRUE(deleted);
    TEST_ASSERT_FALSE(stillExists);
}

/**
 * Test: Delete non-existing item should return false
 */
void test_cuckoo_delete_nonexistent(void) {
    filterInit(&testFilter);
    
    bool deleted = filterDelete(&testFilter, "never_inserted");
    
    TEST_ASSERT_FALSE(deleted);
}

/**
 * Test: Multiple inserts of same item
 */
void test_cuckoo_duplicate_insert(void) {
    filterInit(&testFilter);
    
    bool first = filterInsert(&testFilter, "duplicate");
    bool second = filterInsert(&testFilter, "duplicate");
    bool found = filterLookup(&testFilter, "duplicate");
    
    TEST_ASSERT_TRUE(first);
    TEST_ASSERT_TRUE(second);  // Should succeed (adds another fingerprint)
    TEST_ASSERT_TRUE(found);
}

/**
 * Test: Insert many items and verify false positive rate
 */
void test_cuckoo_false_positive_rate(void) {
    filterInit(&testFilter);
    
    const int insertCount = 500;
    const int queryCount = 500;
    int falsePositives = 0;
    
    // Insert items
    for (int i = 0; i < insertCount; i++) {
        char key[16];
        snprintf(key, sizeof(key), "item_%d", i);
        filterInsert(&testFilter, key);
    }
    
    // Query items that were never inserted
    for (int i = insertCount; i < insertCount + queryCount; i++) {
        char key[16];
        snprintf(key, sizeof(key), "item_%d", i);
        if (filterLookup(&testFilter, key)) {
            falsePositives++;
        }
    }
    
    float fpRate = (float)falsePositives / queryCount * 100.0f;
    
    // Expected FP rate is ~3% with default settings
    // Allow up to 10% for small sample sizes
    TEST_ASSERT_LESS_THAN(10.0, fpRate);
    
    Serial.printf("FP Rate: %.2f%% (%d/%d)\n", fpRate, falsePositives, queryCount);
}

/**
 * Test: Filter full behavior
 */
void test_cuckoo_filter_full(void) {
    filterInit(&testFilter);
    
    int insertFails = 0;
    const int maxInserts = 2000;  // Try to overfill
    
    for (int i = 0; i < maxInserts; i++) {
        char key[16];
        snprintf(key, sizeof(key), "overflow_%d", i);
        if (!filterInsert(&testFilter, key)) {
            insertFails++;
        }
    }
    
    // Some inserts should fail when filter is full
    TEST_ASSERT_GREATER_THAN(0, insertFails);
    
    Serial.printf("Insert fails at capacity: %d/%d\n", insertFails, maxInserts);
}

// ─── Test Runner ──────────────────────────────────────────────────────────────

void setup() {
    delay(2000);  // Give time for Serial to initialize
    
    Serial.begin(115200);
    Serial.println("\n=== Cuckoo Filter Unit Tests ===");
    
    UNITY_BEGIN();
    
    Serial.println("\n--- Running Cuckoo Filter Tests ---");
    RUN_TEST(test_cuckoo_empty_lookup);
    RUN_TEST(test_cuckoo_insert_and_lookup);
    RUN_TEST(test_cuckoo_delete_existing);
    RUN_TEST(test_cuckoo_delete_nonexistent);
    RUN_TEST(test_cuckoo_duplicate_insert);
    RUN_TEST(test_cuckoo_false_positive_rate);
    RUN_TEST(test_cuckoo_filter_full);
    
    Serial.println("\n=== All Tests Complete ===");
    
    UNITY_END();
}

void loop() {
    // Nothing here - tests run once in setup
    delay(1000);
}
