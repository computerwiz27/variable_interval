#include <Arduino.h>
#line 1 "/home/paul/cate/variable_interval/variable_interval.ino"
#include <FED3.h>

#define FEED              0
#define FIXED_RATIO_LEFT_1  1
#define FIXED_RATIO_LEFT_3  2
#define FIXED_RATIO_LEFT_5  3
#define FIXED_RATIO_RIGHT_1 4
#define FIXED_RATIO_RIGHT_3 5
#define FIXED_RATIO_RIGHT_5 6
#define VARIABLE_INTERVAL_BOTH_30 7
#define VARIABLE_INTERVAL_LEFT_30 8
#define VARIABLE_INTERVAL_RIGHT_30 9

String sketch = "VI";
FED3 fed3(sketch);

int feeds;

bool waiting_to_feed;

long interval;
long elapsed_time;
int remainder_micros;

#line 25 "/home/paul/cate/variable_interval/variable_interval.ino"
void setup();
#line 42 "/home/paul/cate/variable_interval/variable_interval.ino"
void loop();
#line 219 "/home/paul/cate/variable_interval/variable_interval.ino"
int get_interval(uint avg, float predic_degree);
#line 25 "/home/paul/cate/variable_interval/variable_interval.ino"
void setup()
{
  Serial.begin(9600);
  delay(500);
  Serial.println("***RESET***");

  waiting_to_feed = false;
  feeds = 0;
  elapsed_time = 0;
  remainder_micros = 0;

  fed3.FED3Menu = true;
  fed3.begin();
  fed3.run();
  srand((unsigned int)fed3.unixtime);
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

  if (fed3.FEDmode == VARIABLE_INTERVAL_BOTH_30
    || fed3.FEDmode == VARIABLE_INTERVAL_LEFT_30
    || fed3.FEDmode == VARIABLE_INTERVAL_RIGHT_30)
  {

    if (fed3.FEDmode == VARIABLE_INTERVAL_BOTH_30)
    {
      fed3.sessiontype = "VI_30";
    }
    if (fed3.FEDmode == VARIABLE_INTERVAL_LEFT_30)
    {
      fed3.sessiontype = "VI_L_30";
    }
    if (fed3.FEDmode == VARIABLE_INTERVAL_RIGHT_30)
    {
      fed3.sessiontype = "VI_R_30";
    }

    bool feed_on_left = fed3.FEDmode == VARIABLE_INTERVAL_BOTH_30 
      || fed3.FEDmode == VARIABLE_INTERVAL_LEFT_30;
    bool feed_on_right = fed3.FEDmode == VARIABLE_INTERVAL_BOTH_30 
      || fed3.FEDmode == VARIABLE_INTERVAL_RIGHT_30;

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
      fed3.display.print((interval - elapsed_time) / 1000 + 1);
      fed3.display.print("s");
      fed3.display.refresh();

      if(elapsed_time >= interval) {
        fed3.Feed();
        feeds++;
        elapsed_time = 0;

        waiting_to_feed = false;
        fed3.EnableSleep = true;
      }
    }

    else {
      if (feed_on_left && left_poke) {
        interval = get_interval(30, 0.75);
        
        waiting_to_feed = true;
        fed3.EnableSleep = false;
      }

      if (feed_on_right && right_poke) {
        interval = get_interval(30, 0.75);
        
        waiting_to_feed = true;
        fed3.EnableSleep = false;
      }
    }
  }
}

// get the interval in milliseconds
int get_interval(uint avg, float predic_degree) {
  float upper = avg * (1 + predic_degree);
  float lower = avg * (1 - predic_degree);

  int mod = upper - lower + 1;

  int r = rand();

  return (r % mod + lower) * 1000;
}
