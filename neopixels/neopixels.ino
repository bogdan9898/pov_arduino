#include <Timer.h>
Timer t;

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
 
#define DATA_PIN 10
#define NUMPIXELS 24
#define COLOR_SPACE 3

//#define DEBUG

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, DATA_PIN, NEO_GRB + NEO_KHZ800);

#define NUM_COLORS 6
const byte colors[NUM_COLORS+1][3] = {255, 0, 255,
                                                                      0, 255, 255,
                                                                      255, 255, 0,
                                                                      0, 255, 0,
                                                                      255, 0, 0,
                                                                      0, 0, 255,
                                                                      0, 0, 0
                                                                      };

#define OFF NUM_COLORS

const unsigned int segments = 32;
byte image[segments][NUMPIXELS] = {0};

unsigned int currentSegment = 0;
unsigned long segmentInterval = 0;

unsigned long fullRotationStartTime = 0;

int timerId = -1;
int animationTimer = -1;

//const unsigned long interval = 6;
//unsigned long prevTime = 0;
//unsigned int offset = 0;

void setup() {
#ifdef DEBUG
  Serial.begin(9600);
#endif
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(A0, INPUT);
  
  digitalWrite(11, LOW);
  digitalWrite(12, HIGH);
  randomSeed(analogRead(0));
  
  pixels.begin();
  pixels.setBrightness(25);

  // random colors
  int prev_color_index = -1;
  for(int i = 0; i < segments; i ++) {
    int color_index = -1;
    do {
      color_index = random(NUM_COLORS);
    } while(prev_color_index == color_index);
    prev_color_index = color_index;
    for(int j = 0; j < NUMPIXELS; j++) {
      image[i][j] =  color_index;
    }
  }

  timerId = t.every(500, drawSegment, -1);
}

void loop() {
//  for(int i=0; i < NUMPIXELS; i++){
//    pixels.setPixelColor(i, pixels.Color(image[(i + offset) % NUMPIXELS][0], image[(i + offset) % NUMPIXELS][1], image[(i + offset) % NUMPIXELS][2])); 
//  }
//  pixels.show();
//  if(micros() - prevTime >= interval) {
//    offset++;
//    prevTime = micros();
////    Serial.println(prevTime);
//  }

//    static const unsigned int imageInterval = interval * segments;
//    if(millis() - prevTime > imageInterval) {
//      prevTime = millis();
//      drawImage();
//  anim1();

  int val = analogRead(A0);
//#ifdef DEBUG
//  Serial.println(val);
//#endif
//  static unsigned int t = random(NUM_COLORS);
//  static unsigned long prevFullRotationTime = millis();
  static boolean changeColor = true;
  if(val < 200) {
    if(changeColor) {
      changeColor = false;
//      t = random(NUM_COLORS);
      segmentInterval = (millis() - fullRotationStartTime) / segments;
      fullRotationStartTime = millis();
      currentSegment = 0;
      if(timerId >= 0) {
        t.stop(timerId);        
      }
      timerId = t.every(segmentInterval, drawSegment, -1);
    }
  } else {
    changeColor = true;
  }

  static bool animRunning = false;
  if(millis() > 5000 and not animRunning) {
    animRunning = true;
//    anim1();
    anim2();
  }

  t.update();
}

void anim1() { // spinning circles
  // generate initial state
  int prev_color_index = -1;
  for(int j = 0; j < NUMPIXELS; j+=2) {
    int color_index = -1;
    if((j / 2) % 2 == 0) {
      do {
        color_index = random(NUM_COLORS);
      } while(prev_color_index == color_index);
      prev_color_index = color_index;
    } else {
        color_index = OFF; // last color will always be black
    }

    const byte gapSize = 4;
    
    for(int i = 0; i < segments; i++) {
      if(i < gapSize) {
        image[i][j] = OFF;
        image[i][j+1] = OFF;
      } else {
        image[i][j] = color_index;
        image[i][j+1] = color_index;
      }
    }
  }

  animationTimer = t.every(100, anim1Update, -1);
}

void anim1Update() {
    bool dir = false;
    for(int j = 0; j < NUMPIXELS; j++) {
#ifdef DEBUG
      Serial.print("j: "); Serial.println(j);
      for(int i = 0; i < segments; i++) {
        Serial.print(image[i][j]); Serial.print(", ");
      }
      Serial.println("");
#endif
      // search first turned on led
      if(j % 4 == 0) {
        dir = not dir;
      }
#ifdef DEBUG
      Serial.print("dir: "); Serial.println(dir);
#endif
      int index = -1;
      int i = 0;
      for(int x = 0;x < segments; x++) {
#ifdef DEBUG
        Serial.print("next i: "); Serial.println(dir ? (i + 1) : (i - 1) >= 0 ? i - 1 : (segments - i - 1));
#endif
        if(isTurnedOn(i, j)) {
          index = i;
          break;
        }
        i = dir ? (i + 1) : (i - 1) >= 0 ? i - 1 : (segments - i - 1);
      }
      if(index == -1) {
#ifdef DEBUG
        Serial.println("Skipping this pixel\n");
#endif  
        continue;
      }
#ifdef DEBUG
      Serial.print("ON index: "); Serial.println(index);
#endif

      // search next first turned off led and light it up
      i = dir ? (index + 1) % segments : (index - 1) >= 0 ?((index - 1) % segments) : ((segments - index - 1) % segments);
      for(int x = 0; x < segments; x++) {
//        Serial.print("iiiiiii: "); Serial.println(i);
        if(not isTurnedOn(i, j)) {
          image[i][j] = image[index][j];
          index = i;
          break;
        }
        i = dir ? (i + 1) % segments : (i - 1) >= 0 ? (i - 1) % segments : ((segments - i - 1) % segments);
      }
#ifdef DEBUG
      Serial.print("OFF index: "); Serial.println(index);
#endif

      i = dir ? (index + 1) % segments : (index - 1) >= 0 ?((index - 1) % segments) : ((segments - index - 1) % segments);
      // search next first turned on led and turn it off
      for(int x = 0; x < segments; x++) {
        if(isTurnedOn(i, j)) {
#ifdef DEBUG
          Serial.print("i: "); Serial.println(i);
#endif
          image[i][j] = OFF;
          break;
        }
        i = dir ? (i + 1) % segments : (i - 1) >= 0 ? (i - 1) % segments :((segments - i - 1) % segments);
      }
#ifdef DEBUG
      for(int i = 0; i < segments; i++) {
        Serial.print(image[i][j]); Serial.print(", ");
      }
      Serial.println("\n");
#endif
    }
#ifdef DEBUG
    Serial.println("~~~~~~~~~~~~~~");
    for(int i = 0; i < segments; i++) {
      for(int j = 0; j < NUMPIXELS; j++) {
        Serial.print(image[i][j]); Serial.print(", ");  
      }
      Serial.println("");
    }
    Serial.println("@@@@@@@@@@@@\n");
#endif
}

void anim2() { // exploding circle
  int color_index = random(NUM_COLORS);
  for(int i = 0; i < segments; i++) {
    for(int j = 0; j < NUMPIXELS; j++) {
      if(j == NUMPIXELS-1) {
        image[i][j] = color_index;          
      } else {
        image[i][j] = OFF;
      }
    }
  }
  
  animationTimer = t.every(250, anim2Update, -1);
}

void anim2Update() {
  static bool dir = true;
  if(dir) {
    // search first turned on led and light up the prev
    for(int j = 0; j < NUMPIXELS; j++) {
#ifdef DEBUG
      Serial.print("j cresc: "); Serial.println(j);
#endif
      if(isTurnedOn(0, j)) {
        if(j > 0) {
          for(int i = 0; i < segments; i++) {
            image[i][j-1] = image[i][j];
          }
        } else {
            dir = not dir;
        }
#ifdef DEBUG
        Serial.println("");
#endif
        break;
      }
    }
  } else {
    // search first turned on led and turn it off except last one
    int j = 0;
    for(; j < NUMPIXELS; j++) {
#ifdef DEBUG
      Serial.print("j desc: "); Serial.println(j);
#endif
      if(isTurnedOn(0, j)) {
        if(j < NUMPIXELS-1) {
          for(int i = 0; i < segments; i++) {
            image[i][j] = OFF;
          }          
        } else {
            dir = not dir;
            // change color
            int color_index = random(NUM_COLORS);
            for(int i = 0; i < segments; i++) {
              for(int j = 0; j < NUMPIXELS; j++) {
                if(j == NUMPIXELS-1) {
                  image[i][j] = color_index;          
                } else {
                  image[i][j] = OFF;
                }
              }
            }
        }
#ifdef DEBUG
        Serial.println("");
#endif
        break;
      }
    }
  }
}

void anim3() { // loading circle
  byte color_index = random(NUM_COLORS);
  for(int i = 0; i < segments; i++) {
    for(int j = 0; j < NUMPIXELS; j++) {
      if(i == 0) {
        image[i][j] = color_index;
      } else {
        image[i][j] = OFF;
      }
    }
  }
  
  animationTimer = t.every(250, anim3Update, -1);
}
 void anim3Update() {
  if(not isTurnedOn(0, 0)) { // if no led turned on
    byte color_index = random(NUM_COLORS);
    for(int i = 0; i < segments; i++) {
      for(int j = 0; j < NUMPIXELS; j++) {
        if(i == 0) {
          image[i][j] = color_index;
        } else {
          image[i][j] = OFF;
        }
      }
    }
    return;
  }

  static bool dir = true;
  if(dir) {
    // search first on segment and turn all leds off
    for(int i = 0; i < segments; i++) {
      if(isTurnedOn(i, 0)) {
        for(int j = 0; j < NUMPIXELS; j++) {
          image[i][j] = OFF;
        }
        if(i >= segments - 1) {
          dir = not dir;
        }
        break;
      }
    }
  } else {
    // search first off segment and turn all leds on
    for(int i = 0; i < segments; i++) {
      if(not isTurnedOn(i, 0)) {
        for(int j = 0; j < NUMPIXELS; j++) {
          // edge case for led (0, 0) OFF, is checked and fixed at the beginning
          image[i][j] = image[i-1][j];
        }
        if(i >= segments - 1) {
          dir = not dir;
        }
        break;
      }
    }
  }
  
}

void anim4() { // spiral
  
}

void anim4Update() {
  
}

bool isTurnedOn(int i, int j) {
#ifdef DEBUG
  Serial.print("isTurnedOn("); Serial.print(i); Serial.print(", "); Serial.print(j); Serial.print(") = "); Serial.println(image[i][j] != OFF);
#endif
  return image[i][j] != OFF;
}

void drawSegment() {
    if(currentSegment < segments) {
      for(int j = 0; j < NUMPIXELS; j++) {
        const byte r = colors[image[currentSegment][j]][0];
        const byte g = colors[image[currentSegment][j]][1];
        const byte b = colors[image[currentSegment][j]][2];
        pixels.setPixelColor(j, pixels.Color(r, g, b));
      }
      pixels.show();
      currentSegment++;
  }
//#ifdef DEBUG
//  Serial.println("~~~~~~~~~~~~~~");
//  for(int i = 0; i < segments; i++) {
//    for(int j = 0; j < NUMPIXELS; j++) {
//      Serial.print(image[i][j]); Serial.print(", ");  
//    }
//    Serial.println("");
//  }
//  Serial.println("@@@@@@@@@@@@\n");
//#endif
}
