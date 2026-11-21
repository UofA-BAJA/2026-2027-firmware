/*
 * Here are the possible commands that will be sent from the pit computer
 * to the car computer.
*/

#ifndef COMMANDS_H
#define COMMANDS_H

enum Command{
    RADIO_FREQUENCY_CHANGE = 0,     // Reserved for the "frequency change" flag (radio command)
    RADIO_DATA_CHANGE = 1,          // Reserved for changing which data is being sent via radio

    DEFAULT_CAR_START = 2,          // Default command recieved when car starts (currently starts everything)
    
    START_LOG = 100,                // Starts recording data and realtime transmission
    END_LOG = 101,                  // Stops recording data and realtime transmission
    SET_BOUNDING_BOX = 102,
    LAP_INCREMENT = 103,


    EMERGENCY_STOP = 104,           // Unused at the moment.
                                    // If we do use it, it would be a "crash the program" response 
    
    ENABLE_CHECK_ENGINE = 69,       // currently unused
    DISABLE_CHECK_ENGINE = 70       // currently unused
};


#endif