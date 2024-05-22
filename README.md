## Build

Verify and Upload the file to the FED3 device using the Arduino IDE as usual.

## Run

The program boots into the standard FED3 menu. From here you must select one of the modes.

* **Mode 1: Feed**

    In this mode the device dispenses a pallet immediately on any poke event.

* **Mode 2: Fixed Ratio 1 Left**

    The device dispenses a pallet immediately every 1 left poke events.

* **Mode 3: Fixed Ratio 3 Left**

    The device dispenses a pallet immediately every 3 left poke events.

* **Mode 4: Fixed Ratio 5 Left**

    The device dispenses a pallet immediately every 5 left poke events.

* **Mode 5: Fixed Ratio 1 Right**

    The device dispenses a pallet immediately every 1 right poke events.

* **Mode 6: Fixed Ratio 3 Right**

    The device dispenses a pallet immediately every 3 right poke events.

* **Mode 7: Fixed Ratio 5 Right**

    The device dispenses a pallet immediately every 5 right poke events.

* **Mode 8: Variable Interval**

    This mode brings up a different menu where the parameters of the variable interval and feeding are selected. 
    
    To navigate between the menu elements, both poke sensors must be triggered at the same time. 
    
    Changing the values for the interval average, prediction degree, and the active poke sensor is done by triggering the left or right poke sensors.

    To confirm the selection and begin the routine, trigger the right poke sensor when the 'Done' field is selected.