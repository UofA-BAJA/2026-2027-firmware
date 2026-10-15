#ifndef CANTPROTOCOL_H
#define CANTPROTOCOL_H
// 1 = normal logging, 2 = logs most verbose info
// #define DEBUG_CAN 1

#define QUEUE_LENGTH 16

#include <mcp_can.h>
#include <Arduino.h>


class CANTProtocol{
    public:
        /*
            Constructor
            @param cs the pin number of the chip select pin
            @param interrupt the interrupt pin. For arduinos, this is later wrapped in digitalPinToInterrupt()
            @param deviceID the 5 bit ID of the device. 
        */
        CANTProtocol(int cs, int interrupt, unsigned long deviceID) : CAN(cs), CAN_INTERRUPT_PIN(interrupt), CAN_DEVICE_ID(deviceID){
            //Set reference object to itself
            CANTProtocol::ref = this;
        }
        
        bool begin();
        bool end();
        

        bool registerRequest(byte commandID, void (*onRecieved) (unsigned char dataLength, byte* incomingData, unsigned long callbackID));
        bool registerCommand(byte commandID, void (*onRecieved) (unsigned char dataLength, byte* incomingData, unsigned long callbackID));
        
        void execute();

        //TODO: move this into a new file
        //Why is it in the header file? https://stackoverflow.com/questions/495021/why-can-templates-only-be-implemented-in-the-header-file

        /**
         * Call this method with your built data at the end of a CAN *Request* onRecieved method.
         * Do NOT call this method for CAN commands (what would the data be anyway?)
         * @param data Your data, can be anything that isn't null
         * @param callbackID The callback ID passed to the builder, UNMODIFIED
        */
        template <typename T>
        void sendRequestResponse(T data, unsigned long callbackID){
            #if DEBUG_CAN == 2
                Serial.println("inside sendRequestResponse()");
            #endif

            //If there's actually a callback to send to...
            if(callbackID > 0){
                #if DEBUG_CAN == 2
                    Serial.print("actually sending response. size: ");
                    Serial.println(sizeof(T));
                #endif
                byte outputBuffer[8];

                //Calculate the number of frames:
                //Divide by 8, and then add 1 if there's remainder
                int numFrames = (sizeof(T) / 8);
                if(sizeof(T) % 8 > 0){
                    numFrames++;
                }

                byte* ptr = (byte*) &data; //Get a pointer that increments by bytes
                for(int j = 0; j < numFrames; j++){
                    //(this could be one line but I was having trouble thinking of the right way to do it)
                    //If this is the last frame, memcpy the right amount
                    if(j == numFrames - 1){
                        #if DEBUG_CAN
                            Serial.print("now sending end frame callback id: ");
                            Serial.println(callbackID + j);
                        #endif
                        memcpy(&outputBuffer, ptr + (j * 8), sizeof(T) - (j * 8));
                        byte sendMSG = CAN.sendMsgBuf(callbackID + j, 1, sizeof(T) - (j * 8), outputBuffer);
                        #if DEBUG_CAN
                            if(sendMSG == CAN_GETTXBFTIMEOUT) Serial.println("Get TX Buffer Timeout");
                            else if(sendMSG == CAN_SENDMSGTIMEOUT) Serial.println("Send Message Timeout");
                            else if(sendMSG != CAN_OK) Serial.println("Send message failed unknown");
                        #endif
                    }
                    //Otherwise, copy 8 bytes
                    else {
                        #if DEBUG_CAN == 2
                            Serial.print("now sending callback id: ");
                            Serial.println(callbackID + j);
                        #endif
                        memcpy(&outputBuffer, ptr + (j * 8), 8);
                        byte sendMSG = CAN.sendMsgBuf(callbackID + j, 1, 8, outputBuffer);
                        #if DEBUG_CAN
                            if(sendMSG == CAN_GETTXBFTIMEOUT) Serial.println("Get TX Buffer Timeout");
                            else if(sendMSG == CAN_SENDMSGTIMEOUT) Serial.println("Send Message Timeout");
                            else if(sendMSG != CAN_OK) Serial.println("Send message failed unknown");
                        #endif
                    }
                    //TODO: do something if it fails to send 
                }
            }
        }

        static void ISRHandler();
        void InterruptSubroutine();

        MCP_CAN CAN;

    private:
        const int CAN_INTERRUPT_PIN; 
        const unsigned long CAN_DEVICE_ID;

        //Variables used for can reading
        long unsigned int canRXId = 0;
        unsigned char canRXlength = 0;
        unsigned char canRXBuf[8];

        //Flag for "there is data" so you don't need to do an expensive read
        //to check every loop
        volatile bool dataAvailable = false;
        
        //ISRs are static, but we need a specific reference to a CANTProtocol object
        //This does sadly prevent more than 1 can chip right now
        static CANTProtocol* ref;

        //Chat-assisted code begins here
        //TODO: Claude is this still true? The description doesn't line up quite well
        /*
            Basically, in order to have one function to call, but multiple different implementations (one including a template) in the same array,
            you have to have a base struct, and then structs that derive from that base struct.
            call() carries out all the functions, including calling the dataBuilder()/onRecieved() and sending out CAN frames, if nessecary.
            This kinda sucks being in the header file, in the future I might figure out if there's a way to split this off into sub files
            without it breaking the templates (see above: registerRequest())
        */
        struct RegisteredBase {
            bool exists = false;
            void (*onRecieved) (unsigned char dataLength, byte* incomingData, unsigned long callbackID) = 0;
            bool isRequest = false;
        };

        //A list of all the messages we can recieve. Each array index is a datatype as defined in the CANT spec 
        RegisteredBase registeredMessages[16];
        //Chat-assisted code ends here

 
        //This struct holds the data and callback ID for pending requests and commands
        struct PendingCANFrame{
            byte data[8]; //Recieved data
            unsigned char dataLength;
            unsigned long dataID;
            unsigned long callbackID;
        };
        
        /*
            TODO: recomment
        */

        //TODO: global variable / preprocessor for queue length
        PendingCANFrame pendingFrames[QUEUE_LENGTH];
        int frameQueueFront = 0;
        int frameQueueLength = 0;
};






#endif