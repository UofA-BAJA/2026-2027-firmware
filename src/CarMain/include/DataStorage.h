/* 
 *
 *
 * Author: Matthew Larson
 */

#ifndef DATASTORAGE_H
#define DATASTORAGE_H

#include <sqlite3.h>
#include <iostream>
#include <string>
#include <queue>

#include <mutex>
#include <thread>
#include <atomic>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>

#include "SqlCommands.h"
#include "DataTypes.h"
#include "Constants.h"
#include "CarTime.h"

namespace BajaWildcatRacing
{

    class DataStorage {
        public:
            DataStorage();
            void start();
            void end();

            void setStoragePath(const char* path);

            void storeData(ShockDisplacement data);
            void storeData(RotationXYZ rotData, AccelerationXYZ accData);
            void storeData(BrakePressure data);
            void storeData(WheelSpeed data);
            void storeData(EngineRPM data);
            void storeData(CVTTemp data);
            void storeData(GPSPosition data);


        private:
            // Database handle for our sqlite database
            sqlite3* db;

            ulong numDataInserts = 1;

            std::mutex insertBufferMutex;
            std::queue<sqlite3_stmt *> insertBuffer;

            std::thread updateDBThread;
            std::atomic<bool> running = true;

            //path to storage location
            std::string storagePath;
            


            void queueSqlStatement(sqlite3_stmt *statement);

            void setupDatabase(const char* path);
            char* getFilepath();
            void updateDatabase();

    };

}



#endif
