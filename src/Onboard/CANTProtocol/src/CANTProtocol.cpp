#include "CANTProtocol.h"
// #define DEBUG_CAN 1


#if !defined(CANT_ARDUINO) && !defined(CANT_ESP32)
    #error CANTProtocol must have a platform ("CANT_ARDUINO" or "CANT_ESP32") defined to work! Add it to your platformio.ini file with the build_flags parameter. See documentation for more info.
#endif

//Shenanigans (apparently the linker gets unhappy, thanks chat)
CANTProtocol* CANTProtocol::ref = nullptr;

//Declare this up here so it can be used
#if defined(CANT_ESP32)
// #include "esp_task_wdt.h"
// TaskHandle_t canTaskHandle = NULL;
// void ESP32ISRTrampoline(void* ref);
// volatile bool interruptRecieved = false;
#endif

/*
    begin() - Call this to start recieving and responding to CAN frames

    @returns true if initialization succeeded, false if the inititalization failed due to: 
        - Invalid device ID
        - Failure to communicate to CAN chip (check wiring)
*/
bool CANTProtocol::begin(){

    //Check for valid CAN ID
    if((CAN_DEVICE_ID & ~(0b11111)) > 0){
        return false;
    }

    //Begin CAN
    byte canInitResult = CAN.begin(MCP_STDEXT, CAN_500KBPS, MCP_8MHZ);
    
    if(canInitResult != CAN_OK){
        return false;
    }
    #if DEBUG_CAN
        Serial.println(CAN_DEVICE_ID << 24, BIN);
    #endif
    //Filter to just the commands we want
    CAN.init_Mask(0, 1, 0x1F000000);
    CAN.init_Filt(0, 1, CAN_DEVICE_ID << 24);
    CAN.init_Filt(1, 1, CAN_DEVICE_ID << 24);

    CAN.init_Mask(1, 1, 0x1F000000);
    CAN.init_Filt(2, 1, CAN_DEVICE_ID << 24);
    CAN.init_Filt(3, 1, CAN_DEVICE_ID << 24);
    CAN.init_Filt(4, 1, CAN_DEVICE_ID << 24);
    CAN.init_Filt(5, 1, CAN_DEVICE_ID << 24);


    canInitResult = CAN.setMode(MCP_NORMAL);
    if(canInitResult != MCP2515_OK){
        return false;
    }

    pinMode(CAN_INTERRUPT_PIN, INPUT);

    //ESP32 and Arduino have two slightly different implemenetations of attachInterrupt()
    //Also ESP32 needs its task defined 

    //TODO: remove the whole handler thing for esp32
    #if defined(CANT_ARDUINO)
        attachInterrupt(digitalPinToInterrupt(CAN_INTERRUPT_PIN), ISRHandler, FALLING);
    #elif defined(CANT_ESP32)
        attachInterrupt(CAN_INTERRUPT_PIN, ISRHandler, FALLING);
        // TaskHandle_t handle;
        // xTaskCreate(
        //     ESP32ISRTrampoline,
        //     "Interrupt Handler",
        //     1024, 
        //     (void*)ref,
        //     3,
        //     &canTaskHandle);
        // esp_task_wdt_delete(handle);
    #endif

    return true;
}

/**
 * Registers a request for the CAN system to respond to.
 * @param commandID The command ID from 0-15 to bind this request to
 * @param onRecieved The function name for the builder that responds to this request.
 * @returns true if successfully registered, false if an invalid ID is given. 
 */
bool CANTProtocol::registerRequest(byte commandID, void (*onRecieved) (unsigned char dataLength, byte* incomingData, unsigned long callbackID)){
    //Check for valid command ID
    if((commandID & ~(0b1111)) > 0){
        return false;
    }
    
    registeredMessages[commandID].onRecieved = onRecieved;
    registeredMessages[commandID].isRequest = true;
    registeredMessages[commandID].exists = true;

    return true;
}


/**
 * Registers a command (lossles/lossy, handled automatically) for the CAN system to respond to.
 * @param commandID The command ID from 0-15 to bind this command to
 * @param onRecieved The function name for the function that takes action on this command.
 * @returns true if successfully registered, false if an invalid ID is given. 
 */
bool CANTProtocol::registerCommand(byte commandID, void (*onRecieved) (unsigned char dataLength, byte* incomingData, unsigned long callbackID)){
    //Check for valid command ID
    if((commandID & ~(0b1111)) > 0){
        return false;
    }

    registeredMessages[commandID].onRecieved = onRecieved;
    registeredMessages[commandID].isRequest = false;
    registeredMessages[commandID].exists = true;

    return true;
}


/**
 * Call in the loop() function in the arduino code to run the CAN stuff.
 */
void CANTProtocol::execute(){
    #if DEBUG_CAN == 2
        Serial.println("Execute loop");
    #endif

    //If the interrupt flagged commands being available
    if(dataAvailable == true){
        dataAvailable = false; //Reset flag

        //While there is still stuff in the recieve buffer
        while(CAN.readMsgBuf(&canRXId, &canRXlength, canRXBuf) != CAN_NOMSG){            
            //If the length of the queue is 16 frames or greater (max size), drop it. 
            if(frameQueueLength >= QUEUE_LENGTH) {
                #if DEBUG_CAN
                    Serial.println("WARNING: Frame queue overload. Reduce frequency");
                #endif

                break;
            }
            
            //Insert at the right place in the queue
            int insertLocation = frameQueueFront + frameQueueLength;
            //If we're trying to insert at an index off the end of the array, wraparound to the front
            if(insertLocation > QUEUE_LENGTH - 1) insertLocation = insertLocation - QUEUE_LENGTH;
            
            //Populate the frame (the object is always there, just overwrite it)
            memcpy(pendingFrames[insertLocation].data, canRXBuf, canRXlength);
            pendingFrames[insertLocation].dataLength = canRXlength;
            pendingFrames[insertLocation].callbackID = (canRXId & 0x000FFFFF);
            pendingFrames[insertLocation].dataID = ((canRXId & 0x00F00000) >> 20); 
            
            frameQueueLength++;
        }
    }

    //If there is something to be responded to, do it.
    //Only do one at a time so that there's time to go back and check for new commands
    if(frameQueueLength > 0){
        PendingCANFrame* incoming = &pendingFrames[frameQueueFront];

        //TODO: should this be in the recieve block
        if(registeredMessages[incoming->dataID].exists){
            RegisteredBase canMessage = registeredMessages[incoming->dataID];
            if(canMessage.isRequest){
                #if DEBUG_CAN
                    // Serial.print("request.onRecieved() id: ");
                    // Serial.println(incoming->dataID);
                #endif

                //Run onRecieved / builder method 
                canMessage.onRecieved(incoming->dataLength, incoming->data, incoming->callbackID);
            }else{
                #if DEBUG_CAN
                    // Serial.println("running command.onRecieved() for id: " + incoming -> dataID);
                #endif
                
                //Run onRecieved / builder method
                //It should *not* call sendRequestResponse()
                canMessage.onRecieved(incoming->dataLength, incoming->data, 0);

                //If there is a callback, send ack bit. Otherwise do nothing
                if(incoming->callbackID > 0){
                    byte outputBuffer[8];
                    outputBuffer[0] = 0x43; //TODO: PROPERLY DEFINE THE ACK BIT
                    byte sendMSG = CAN.sendMsgBuf(incoming->callbackID, 1, 1, outputBuffer);
                    #if DEBUG_CAN
                        if(sendMSG == CAN_GETTXBFTIMEOUT) Serial.println("Get TX Buffer Timeout");
                        else if(sendMSG == CAN_SENDMSGTIMEOUT) Serial.println("Send Message Timeout");
                        else if(sendMSG != CAN_OK) Serial.println("Send message failed unknown");
                    #endif
                }
            }
        }
        #if DEBUG_CAN
        else{
            Serial.println("Recieved a message that isn't registered. Guh?");
        }
        #endif

        //Wrap around if at end
        if(frameQueueFront == QUEUE_LENGTH - 1) frameQueueFront = 0;
        else frameQueueFront++;
        frameQueueLength--;

    }
}

/**
 * Turns off the interrupt and disables the CAN chip
 * @return true. lol.
 */
bool CANTProtocol::end(){
    #if defined(CANT_ARDUINO)
        detachInterrupt(digitalPinToInterrupt(CAN_INTERRUPT_PIN));
    #elif defined(CANT_ESP32)
        detachInterrupt(CAN_INTERRUPT_PIN);
    #endif
    CAN.setMode(MCP_SLEEP);
    return true;
}

/*
ISRs have to be static functions (i.e. not tied to an instance of CANTProtocol).

Wejust call the non-static version directly in the ISRHandler() function.

IRAM_ATTR is a flag for ESP32 functions that tells the RTOS to keep it in RAM so access is faster. 
Since this is critical, we have to 
*/

#if defined(CANT_ESP32)
void IRAM_ATTR CANTProtocol::ISRHandler(){ 
#else
void CANTProtocol::ISRHandler(){ 
#endif
    ref->InterruptSubroutine();  
}

//DON'T CALL THIS YOURSELF. THIS WILL MESS THINGS UP
#if defined(CANT_ESP32)
void IRAM_ATTR CANTProtocol::InterruptSubroutine(){
#else
void CANTProtocol::InterruptSubroutine(){
#endif

    //Tell the main loop that data is available
    dataAvailable = true;
}