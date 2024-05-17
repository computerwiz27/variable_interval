#include <FED3.h>

#define FEED              0
#define FIXED_RATIO_LEFT_1  1
#define FIXED_RATIO_LEFT_3  2
#define FIXED_RATIO_LEFT_5  3
#define FIXED_RATIO_RIGHT_1 4
#define FIXED_RATIO_RIGHT_3 5
#define FIXED_RATIO_RIGHT_5 6
#define VARIABLE_INTERVAL 7

String sketch = "VI";
FED3 fed3(sketch);

int feeds;

int avg;
float predic_degree;

bool feed_on_left;
bool feed_on_right;

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
  feed_on_left = true;
  feed_on_right = false;

  waiting_to_feed = false;
  feeds = 0;
  elapsed_time = 0;
  remainder_micros = 0;

  fed3.FED3Menu = true;
  fed3.begin();

  if (fed3.FEDmode == VARIABLE_INTERVAL) {
    run_VI_menu();
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
  
  display.setCursor(10, 60);
  display.print("feed on: ");
  display.setCursor(100, 60);
  if (feed_on_left && feed_on_right) {
    display.print("both");
  } else if (feed_on_left) {
    display.print("left");
  } else {
    display.print("right");
  }

  display.refresh();
}

void draw_selection(String selection) {
  Adafruit_SharpMem display = fed3.display;

  display.setTextColor(WHITE);
  if (selection == "avg") {
    display.fillRect(98, 3, 70, 22, BLACK);
    display.setCursor(100, 20);
    display.print(avg); display.print("s");
  } 
  
  else if (selection == "p degree") {
    display.fillRect(98, 23, 70, 22, BLACK);
    display.setCursor(100, 40);
    display.print(predic_degree);
  } 
  
  else if (selection == "sensor") {
    display.fillRect(98, 43, 70, 22, BLACK);
    display.setCursor(100, 60);
    if (feed_on_left && feed_on_right) {
      display.print("both");
    } else if (feed_on_left) {
      display.print("left");
    } else {
      display.print("right");
    }
  }

  display.refresh();
}

String select(String selection) {
  String new_selection = selection;

  if (digitalRead(LEFT_POKE) == LOW && digitalRead(RIGHT_POKE) == LOW) {
    if (selection == "avg") {
      new_selection = "p degree";
    } else if (selection == "p degree") {
      new_selection = "sensor";
    } else if (selection == "sensor") {
      new_selection = "done";
    }
    fed3.display.clearDisplay();
    display_VI_menu();
    fed3.display.refresh();
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
    } else if (selection == "sensor") {
      if (feed_on_left && feed_on_right) {
        feed_on_left = false;
        feed_on_right = true;
      } else if (feed_on_left) {
        feed_on_left = true;
        feed_on_right = true;
      } else {
        feed_on_left = true;
        feed_on_right = false;
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
    } else if (selection == "sensor") {
      if (feed_on_left && feed_on_right) {
        feed_on_left = true;
        feed_on_right = false;
      } else if (feed_on_left) {
        feed_on_left = false;
        feed_on_right = true;
      } else {
        feed_on_left = true;
        feed_on_right = true;
      }
    }
  }

  

  return new_selection;
}

void run_VI_menu() {
  display_VI_menu();

  String selection = "avg";

  while(selection != "done") {
    draw_selection(selection);
    selection = select(selection);
    delay(300);
  }

  fed3.display.clearDisplay();
  fed3.display.refresh();
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

    if (fed3.FEDmode == FIXED_RATIO_LEFT_1)
    {
      fed3.sessiontype = "FR1_L";
      fed3.FR = 1;
    }
    if (fed3.FEDmode == FIXED_RATIO_LEFT_3)
    {
      fed3.sessiontype = "FR3_L";
      fed3.FR = 3;
    }
    if (fed3.FEDmode == FIXED_RATIO_LEFT_5)
    {
      fed3.sessiontype = "FR5_L";
      fed3.FR = 5;
    }

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

    if (fed3.FEDmode == FIXED_RATIO_RIGHT_1)
    {
      fed3.sessiontype = "FR1_R";
      fed3.FR = 1;
    }
    if (fed3.FEDmode == FIXED_RATIO_RIGHT_3)
    {
      fed3.sessiontype = "FR3_R";
      fed3.FR = 3;
    }
    if (fed3.FEDmode == FIXED_RATIO_RIGHT_5)
    {
      fed3.sessiontype = "FR5_R";
      fed3.FR = 5;
    }

    if (right_poke)
    {
      if (fed3.RightCount % fed3.FR == 0)
      {
        fed3.Feed();
      }
    }
  }

  //////// VARIABLE INTERVAL MODES ////////

  if (fed3.FEDmode == VARIABLE_INTERVAL)
  {
    fed3.sessiontype = "VAR INT";

    if (feed_on_left && feeds == 0 && left_poke) {
      feeds++;
      fed3.Feed();
      return;
    }

    if (feed_on_right && feeds == 0 && right_poke) {
      feeds++;
      fed3.Feed();
      return;
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

        // Device must not enter sleep mode while keeping track of time
        // This is important, do not change
        fed3.EnableSleep = true;
      }
    }


    // Sleep mode is activated once the device is done feeding
    else {
      if (feed_on_left && left_poke) {
        interval = get_interval();
        
        waiting_to_feed = true;
        fed3.EnableSleep = false;
      }

      if (feed_on_right && right_poke) {
        interval = get_interval();
        
        waiting_to_feed = true;
        fed3.EnableSleep = false;
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