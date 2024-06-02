#include <FED3.h>

#define FEED              0
#define FIXED_RATIO_LEFT_1  1
#define FIXED_RATIO_LEFT_3  2
#define FIXED_RATIO_LEFT_5  3
#define FIXED_RATIO_RIGHT_1 4
#define FIXED_RATIO_RIGHT_3 5
#define FIXED_RATIO_RIGHT_5 6
#define VARIABLE_INTERVAL 7

String sketch = "Menu";
FED3 fed3(sketch);

int feeds;

int avg;
float predic_degree;

bool waiting_to_feed;

long interval;
long elapsed_time;
int remainder_micros;

void setup()
{
  Serial.begin(9600);
  delay(500);
  Serial.println("***RESET***");

  avg = 30;
  predic_degree = 0.75;

  waiting_to_feed = false;
  feeds = 0;
  elapsed_time = 0;
  remainder_micros = 0;

  fed3.FED3Menu = true;
  fed3.begin();

  if (fed3.FEDmode == FIXED_RATIO_LEFT_1
    || fed3.FEDmode == FIXED_RATIO_RIGHT_1) {
    fed3.FR = 1;
  }

  if (fed3.FEDmode == FIXED_RATIO_LEFT_3
    || fed3.FEDmode == FIXED_RATIO_RIGHT_3) {
    fed3.FR = 3;
  }

  if (fed3.FEDmode == FIXED_RATIO_LEFT_5
    || fed3.FEDmode == FIXED_RATIO_RIGHT_5) {
    fed3.FR = 5;
  }

  if (fed3.FEDmode == VARIABLE_INTERVAL) {
    run_VI_menu();
  }

  if (fed3.FEDmode == VARIABLE_INTERVAL) {
    fed3.disableSleep();
  }

  fed3.run();
  srand((unsigned int)fed3.unixtime);
}

void display_VI_menu() {
  Adafruit_SharpMem display = fed3.display;
  display.setTextColor(BLACK);
  display.clearDisplay();

  display.setCursor(10, 20); 
  display.print("avg: ");
  display.setCursor(100, 20);
  display.print(avg);
  display.print("s");
  

  display.setCursor(10, 40);
  display.print("p degree: ");
  display.setCursor(100, 40);
  display.print(predic_degree);

  display.setCursor(10, 130);
  display.print("Done");

  display.refresh();
}

void draw_selection(String selection) {
  Adafruit_SharpMem display = fed3.display;

  display.setTextColor(WHITE);
  if (selection == "avg") {
    display.fillRect(98, 3, 65, 22, BLACK);
    display.setCursor(100, 20); display.print("<");
    display.setCursor(115, 20); display.print(avg); display.print("s");
    display.setCursor(150, 20);display.print(">");
  } 
  
  else if (selection == "p degree") {
    display.fillRect(98, 23, 65, 22, BLACK);
    display.setCursor(100, 40); display.print("<");
    display.setCursor(115, 40); display.print(predic_degree);
    display.setCursor(150, 40); display.print(">");
  } 

  else if (selection == "confirm") {
    display.fillRect(98, 113, 65, 22, BLACK);
    display.setCursor(150, 130); display.print(">");
  }

  display.refresh();
}

String select(String selection) {
  String new_selection = selection;

  if (digitalRead(LEFT_POKE) == LOW && digitalRead(RIGHT_POKE) == LOW) {
    if (selection == "avg") {
      new_selection = "p degree";
    } else if (selection == "p degree") {
      new_selection = "confirm";
    } else if (selection == "confirm") {
      new_selection = "avg";
    }
  }

  else if(digitalRead(LEFT_POKE) == LOW) {
    if (selection == "avg") {
      avg -= 10;
      if (avg < 10) {
        avg = 10;
      }
    } else if (selection == "p degree") {
      predic_degree -= 0.25;
      if (predic_degree < 0) {
        predic_degree = 0;
      }
    } 
  }

  else if(digitalRead(RIGHT_POKE) == LOW) {
    if (selection == "avg") {
      avg += 10;
      if (avg > 300) {
        avg = 300;
      }
    } else if (selection == "p degree") {
      predic_degree += 0.25;
      if (predic_degree > 1) {
        predic_degree = 1;
      }
    } else if (selection == "confirm") {
      new_selection = "done";
    }
  }  

  return new_selection;
}

void run_VI_menu() {
  display_VI_menu();

  String selection = "avg";

  String last_selection = selection;
  while(selection != "done") {
    draw_selection(selection);
    selection = select(selection);
    if (selection != last_selection) {
      fed3.display.clearDisplay();
      display_VI_menu();
      fed3.display.refresh();
      last_selection = selection;
    }
    delay(300);
  }

  fed3.display.clearDisplay();
  fed3.display.refresh();

  fed3.Left = false;
  fed3.Right = false;
}

void loop() {
  unsigned long loop_start_t_mil = millis();
  unsigned long loop_start_t_mic = micros();

  fed3.run();

  bool left_poke = fed3.Left;
  bool right_poke = fed3.Right;

  if (left_poke)
  {
    fed3.logLeftPoke();
  }
  if (right_poke)
  {
    fed3.logRightPoke();
  }

//////// FEED MODE ////////
  if (fed3.FEDmode == FEED) {
    fed3.sessiontype = "FEED";

    if (left_poke || right_poke)
    {
      fed3.Feed();
    }
  }

//////// FIXED RATIO LEFT MODES ////////

if (fed3.FEDmode == FIXED_RATIO_LEFT_1
  || fed3.FEDmode == FIXED_RATIO_LEFT_3
  || fed3.FEDmode == FIXED_RATIO_LEFT_5)
  {

    if (left_poke)
    {
      if (fed3.LeftCount % fed3.FR == 0)
      {
        fed3.Feed();
      }
    }
  }

//////// FIXED RATIO RIGHT MODES ////////

if (fed3.FEDmode == FIXED_RATIO_RIGHT_1
  || fed3.FEDmode == FIXED_RATIO_RIGHT_3
  || fed3.FEDmode == FIXED_RATIO_RIGHT_5)
  {

    if (right_poke)
    {
      if (fed3.RightCount % fed3.FR == 0)
      {
        fed3.Feed();
      }
    }
  }

  //////// VARIABLE INTERVAL MODE ////////

  if (fed3.FEDmode == VARIABLE_INTERVAL)
  {
    fed3.sessiontype = "VAR INT";

    if (feeds == 0) {
      if (left_poke || right_poke) {
        fed3.Feed();
        feeds++;
        logFeed();
      }
      else return;
    }

    if (waiting_to_feed) {
      // get the time spent in the loop
      int loop_t_mil = millis() - loop_start_t_mil;
      int loop_t_mic = micros() - loop_start_t_mic;

      // add the remainder from the previous loop
      remainder_micros += loop_t_mic;
      loop_t_mil += remainder_micros / 1000;
      remainder_micros %= 1000;

      // update the elapsed time
      elapsed_time += loop_t_mil;

      fed3.display.fillRect (85, 22, 70, 15, WHITE);  
      fed3.display.setCursor(90, 36);
      fed3.display.print((interval - elapsed_time) / 1000);
      fed3.display.print("s");
      fed3.display.refresh();

      if(elapsed_time >= interval) {
        fed3.Feed();
        feeds++;
        elapsed_time = 0;

        waiting_to_feed = false;
        logFeed();
      }
    }


    // Sleep mode is activated once the device is done feeding
    else {
      if (fed3.PelletAvailable == false) {
        interval = get_interval();
        waiting_to_feed = true;
        logVI();
      }
    }
  }
}

// get the interval in milliseconds
int get_interval() {
  float upper = avg * (1 + predic_degree);
  float lower = avg * (1 - predic_degree);

  int mod = upper - lower + 1;

  int r = rand();

  return (r % mod + lower) * 1000;
}

void VIlog(String event) {
  // preamble from FED3.cpp
  if (fed3.EnableSleep==true) {
    digitalWrite(MOTOR_ENABLE, LOW);
  }
  fed3.SD.begin(cardSelect, SD_SCK_MHZ(4));

  // fix filename
  char *filename = fed3.filename;
  filename[16] = '.';
  filename[17] = 'C';
  filename[18] = 'S';
  filename[19] = 'V';
  File logfile = fed3.SD.open(filename, FILE_WRITE);

  //if FED3 cannot open file put SD card icon on screen 
  if ( ! logfile ) {
    Adafruit_SharpMem display = fed3.display;
    display.fillRect (68, 1, 15, 22, WHITE);
    //draw SD card icon
    display.drawRect (70, 2, 11, 14, BLACK);
    display.drawRect (69, 6, 2, 10, BLACK);
    display.fillRect (70, 7, 4, 8, WHITE);
    display.drawRect (72, 4, 1, 3, BLACK);
    display.drawRect (74, 4, 1, 3, BLACK);
    display.drawRect (76, 4, 1, 3, BLACK);
    display.drawRect (78, 4, 1, 3, BLACK);
    //exclamation point
    display.fillRect (72, 6, 6, 16, WHITE);
    display.setCursor(74, 16);
    display.setTextSize(2);
    display.setFont(&Org_01);
    display.print("!");
    display.setFont(&FreeSans9pt7b);
    display.setTextSize(1);
  }

  // log time
  logfile.print(fed3.currentHour);
  logfile.print(":");
  if (fed3.currentMinute < 10)
    logfile.print('0');      // Trick to add leading zero for formatting
  logfile.print(fed3.currentMinute);
  logfile.print(":");
  if (fed3.currentSecond < 10)
    logfile.print('0');      // Trick to add leading zero for formatting
  logfile.print(fed3.currentSecond);
  logfile.print(", ");

  if (event == "VI") {
    logfile.print("variable interval set to ");
    logfile.print(interval/1000);
    logfile.print(" s");
  }

  if (event == "feed") {
    logfile.print("dispensed pellet");
  }

  logfile.println();
  logfile.flush();
  logfile.close();
}

void logVI() {
  VIlog("VI");
}

void logFeed() {
  VIlog("feed");
}