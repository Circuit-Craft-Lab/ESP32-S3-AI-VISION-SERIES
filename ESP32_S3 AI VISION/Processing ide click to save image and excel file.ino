import processing.serial.*;
import java.io.ByteArrayInputStream; 
import javax.imageio.ImageIO;        
import java.awt.image.BufferedImage;
import java.io.FileWriter;
import java.io.BufferedWriter;
import java.io.File;
import java.text.SimpleDateFormat; 
import java.util.Date;

Serial myPort;
PImage img;
PImage displayImg; 

boolean fitToScreen = true; 
boolean switchingChannels = false; 
int lastRequestTime = 0;
final int REQUEST_INTERVAL = 1200; 
String currentResLabel = "QVGA (320x240)";
String logStatusMessage = "Mode: QVGA | Click window area to AUTOMATICALLY SAVE";

// AUTOMATION: Sequential tracking counter variable
int objectCounter = 1;

void setup() {
  size(1024, 768); 
  surface.setResizable(true);
  surface.setTitle("ESP32-S3 Auto-Capture & Attendance Logger");
  
  println("Available Serial Ports:");
  printArray(Serial.list());
  
  // NOTE: Verify your array port index matches your active ESP32-S3 board
  String portName = Serial.list()[3]; 
  println("Connecting to: " + portName);
  
  myPort = new Serial(this, portName, 921600);
  myPort.clear();
  
  // Create the local "captures" folder automatically inside your sketch directory if missing
  File folder = new File(sketchPath("captures"));
  if (!folder.exists()) {
    folder.mkdir();
    println("📁 Created 'captures' folder automatically inside the project workspace.");
  }
  
  // PERSISTENCE ENGINE: Check if ledger exists and read lines to resume counter correctly
  String excelSheetPath = sketchPath("captures/captures_ledger.csv");
  File sheetFile = new File(excelSheetPath);
  if (sheetFile.exists()) {
    String[] lines = loadStrings(excelSheetPath);
    if (lines != null && lines.length > 1) {
      objectCounter = lines.length; // Resumes counter from the next slot automatically
      println("📊 Resuming sequence tracking from: Object " + objectCounter);
    }
  }
  
  thread("serialListener");
}

void draw() {
  background(20); 
  
  synchronized(this) {
    if (img != null) {
      displayImg = img;
    }
  }
  
  if (displayImg != null) {
    pushMatrix(); 
    if (fitToScreen) {
      float aspect = (float)displayImg.width / (float)displayImg.height;
      float w = width;
      float h = width / aspect;
      if (h > height) {
        h = height;
        w = height * aspect;
      }
      translate(width/2, height/2);
      scale(-1, 1); 
      image(displayImg, -w/2, -h/2, w, h);
    } else {
      translate(displayImg.width, 0);
      scale(-1, 1); 
      image(displayImg, 0, 0); 
    }
    popMatrix(); 
  } else {
    fill(255);
    textAlign(CENTER, CENTER);
    text("Loading Frame Stream Configuration...", width/2, height/2);
  }
  
  // Graphical Status Overlays
  fill(0, 200);
  noStroke();
  rect(0, height - 45, width, 45);
  fill(255);
  textAlign(LEFT, CENTER);
  text(" Press Keys 1:QVGA | 2:VGA | 3:SVGA | 4:QXGA 3MP", 10, height - 30);
  
  if (switchingChannels) {
    fill(255, 100, 0);
    text(" Status: PROCESSING AUTO-SAVE / SENSOR MODE UPDATE...", 10, height - 15);
  } else {
    fill(0, 255, 150);
    text(" " + logStatusMessage, 10, height - 15);
  }
}

void mousePressed() {
  if (img != null && !switchingChannels) {
    switchingChannels = true; 
    logStatusMessage = "SAVING... Generating image asset and writing entry to spreadsheet ledger.";
    
    // Launch the fully automated save thread sequence immediately 
    thread("autoSaveWorker"); 
  }
}

void autoSaveWorker() {
  synchronized(this) {
    if (img == null) {
      switchingChannels = false;
      return;
    }
    
    // Generate horizontal layout mirror inversion alignment pixel arrays
    PImage flippedImg = createImage(img.width, img.height, RGB);
    img.loadPixels();
    flippedImg.loadPixels();
    for (int y = 0; y < img.height; y++) {
      for (int x = 0; x < img.width; x++) {
        int sourceIndex = x + y * img.width;
        int destIndex = (img.width - 1 - x) + y * img.width;
        flippedImg.pixels[destIndex] = img.pixels[sourceIndex];
      }
    }
    flippedImg.updatePixels();
    
    // Create unique timestamp formats for file names and data logs
    String fileTimestamp = new SimpleDateFormat("yyyyMMdd_HHmmss").format(new Date());
    String excelTimestamp = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date());
    
    // Format image name using the sequential tracking system labels
    String baseName = "object_" + objectCounter + "_" + fileTimestamp + ".jpg";
    String savePathCoordinates = sketchPath("captures/" + baseName);
    
    // Escape backslashes for Windows file path string matching inside Excel formulas
    String formattedExcelPath = savePathCoordinates.replace("\\", "\\\\");
    
    // Write physical image asset onto the local automated harddrive subfolder path
    flippedImg.save(savePathCoordinates);
    println("💾 Picture file saved automatically to: " + savePathCoordinates);
    
    String objectLabel = "Object " + objectCounter;
    String status = "Present";
    
    // Target the spreadsheet file to save directly INSIDE the captures folder
    String excelSheetPath = sketchPath("captures/captures_ledger.csv");
    File sheetFile = new File(excelSheetPath);
    boolean initializedNewSheet = !sheetFile.exists();
    
    try {
      FileWriter fw = new FileWriter(excelSheetPath, true); 
      BufferedWriter bw = new BufferedWriter(fw);
      
      // FIXED SEQUENCE: Set the specific layout column sorting headers
      if (initializedNewSheet) {
        bw.write("Object Sequence,Timestamp,Status,Excel Clickable Hyperlink\n");
      }
      
      // Standard comma parameter separation and explicit absolute string path parameters
      String hyperlinkFormula = "=HYPERLINK(\"" + formattedExcelPath + "\",\"Open Image\")";
      
      // FIXED SEQUENCE: Append data exactly matching your sequence requirement layout
      bw.write(objectLabel + "," + excelTimestamp + "," + status + "," + hyperlinkFormula + "\n");
      
      bw.close();
      fw.close();
      
      println("📊 Ledger successfully updated at: " + excelSheetPath);
      logStatusMessage = "Saved: " + objectLabel + " logged in sequence!";
      
      // Increment counter for the next click asset
      objectCounter++;
      
    } catch (Exception e) {
      println("CRITICAL Error updating spreadsheet registry matrix: " + e.getMessage());
      logStatusMessage = "Error: Spreadsheet entry blocked.";
    }
    
    // Re-enable serial listener processing operations
    switchingChannels = false;
  }
}

void keyPressed() {
  if (key == 's' || key == 'S') {
    fitToScreen = !fitToScreen;
    return;
  }
  
  char targetKey = key;
  if (targetKey == '1' || targetKey == '2' || targetKey == '3' || targetKey == '4') {
    switchingChannels = true; 
    myPort.write(targetKey); 
    
    if (targetKey == '1') currentResLabel = "QVGA (320x240)";
    if (targetKey == '2') currentResLabel = "VGA (640x480)";
    if (targetKey == '3') currentResLabel = "SVGA (800x600)";
    if (targetKey == '4') currentResLabel = "QXGA (2048x1536)";
    
    logStatusMessage = "Mode: " + currentResLabel + " | Click window area to AUTOMATICALLY SAVE";
    println("⚡ Triggered Mode Update -> " + currentResLabel);
    
    delay(800); 
    myPort.clear();
    switchingChannels = false; 
  }
}

void serialListener() {
  while (true) {
    if (!switchingChannels && (millis() - lastRequestTime > REQUEST_INTERVAL)) {
      myPort.write('R'); 
      lastRequestTime = millis();
      
      long waitStart = millis();
      while (myPort.available() <= 0 && millis() - waitStart < 400) {
        delay(5); 
      }
      
      if (myPort.available() > 0 && !switchingChannels) {
        String header = myPort.readStringUntil('\n');
        if (header != null && header.trim().equals("START")) {
          String sizeStr = myPort.readStringUntil('\n');
          if (sizeStr != null) {
            int imgSize = int(sizeStr.trim());
            if(imgSize <= 0 || imgSize > 400000) continue; 
            
            byte[] imgBuffer = new byte[imgSize];
            int bytesRead = 0;
            long startTime = millis();
            
            while (bytesRead < imgSize && !switchingChannels) {
              if (millis() - startTime > 3000) break;
              if (myPort.available() > 0) {
                int remaining = imgSize - bytesRead;
                byte[] chunk = new byte[min(remaining, myPort.available())];
                myPort.readBytes(chunk);
                arrayCopy(chunk, 0, imgBuffer, bytesRead, chunk.length);
                bytesRead += chunk.length;
              }
            }
            
            if (bytesRead == imgSize && !switchingChannels) {
              try {
                ByteArrayInputStream bais = new ByteArrayInputStream(imgBuffer);
                BufferedImage bimg = ImageIO.read(bais);
                if (bimg != null) {
                  PImage tempImg = createImage(bimg.getWidth(), bimg.getHeight(), RGB);
                  bimg.getRGB(0, 0, tempImg.width, tempImg.height, tempImg.pixels, 0, tempImg.width);
                  tempImg.updatePixels();
                  synchronized(this) {
                    img = tempImg;
                  }
                }
              } catch (Exception e) {
                // Drop bad frames safely
              }
            }
          }
        }
      }
    }
    delay(15); 
  }
}
