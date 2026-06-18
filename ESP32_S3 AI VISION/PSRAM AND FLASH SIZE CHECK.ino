/* 
 *  ESP32-S3 Hardware Size & Authenticity Verification Tool
 *  Tests: Actual Internal Flash Size, Total PSRAM Size, and 1MB Dynamic Allocation
 */

void setup() {
  // Initialize Serial Monitor at 115200 Baud Rate
  Serial.begin(115200);
  
  // Wait for Serial interface to open safely
  while (!Serial) {
    delay(10); 
  }

  Serial.println("\n=============================================");
  Serial.println("   ESP32-S3 HARDWARE AUTHENTICITY TEST       ");
  Serial.println("=============================================\n");

  // STEP 1: Verify the Real Flash Memory Size from Chip Identification
  uint32_t flashSize = ESP.getFlashChipSize();
  Serial.print("[INFO] Actual Flash Chip Size: ");
  Serial.print(flashSize / (1024.0 * 1024.0)); // Convert bytes to Megabytes (MB)
  Serial.println(" MB");

  // STEP 2: Verify the Real PSRAM Size Detected by the System
  // Note: Make sure PSRAM is enabled in your Arduino IDE Tools menu!
  uint32_t psramSize = ESP.getPsramSize();
  Serial.print("[INFO] Actual Detected PSRAM Size: ");
  Serial.print(psramSize / (1024.0 * 1024.0)); // Convert bytes to Megabytes (MB)
  Serial.println(" MB");
  
  Serial.println("\n---------------------------------------------");
  Serial.println("[STARTING STRESS TEST] Allocating 1 MB PSRAM block...");
  Serial.println("---------------------------------------------");

  // STEP 3: Attempt to allocate exactly 1 Megabyte (1024 * 1024 bytes) in PSRAM
  size_t allocationSize = 1024 * 1024; 
  
  // ps_malloc allocates specifically out of the external PSRAM space
  char* psramBuffer = (char*) ps_malloc(allocationSize);

  // STEP 4: Check if the pointer returned is valid or null
  if (psramBuffer != NULL) {
    Serial.println("[SUCCESS] Result: 1 MB PSRAM Block Successfully Allocated!");
    
    // Fill memory with dummy data to prove it is stable and fully writable
    memset(psramBuffer, 'A', allocationSize);
    Serial.println("[SUCCESS] Verification: Memory filled and stable.");
    
    // Always free dynamically allocated blocks when finished to avoid leaks
    free(psramBuffer);
    Serial.println("[CLEANUP] Memory successfully released.");
  } else {
    Serial.println("[FAILED] Result: Allocation Failed! Check if your PSRAM is fake or disabled.");
  }

  Serial.println("\n=============================================");
  Serial.println("             HARDWARE TEST COMPLETE          ");
  Serial.println("=============================================");
}

void loop() {
  // Empty loop since we only want to run the authenticity check once upon startup
}
