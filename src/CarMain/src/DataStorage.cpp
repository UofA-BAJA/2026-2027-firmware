#include "DataStorage.h"

namespace BajaWildcatRacing
{

    DataStorage::DataStorage()
    {
        std::cout << "Data Storage constructed" << std::endl;
    }

    void DataStorage::setStoragePath(const char* path){
        storagePath = path;
    }

  
    void DataStorage::start(){
        setupDatabase(storagePath.c_str());

        updateDBThread = std::thread(&DataStorage::updateDatabase, this);

        std::cout << "Data Storage started" << std::endl;
    }

    void DataStorage::end(){
        running = false;

        if (updateDBThread.joinable()) {
            updateDBThread.join();  // Wait for the thread to finish
        }

        sqlite3_close(db);

    }


    /*
    *  Method:  setupDatabase (private)
    *
    *  Purpose: Sets up the database file, tables, and any initial values. Does not retry if it fails.
    *
    *  Pre-Condition: 
    *
    *  Post-Condition: Database is setup unless some weird I/O error occurs (check the console)
    *
    *  @param path    ABSOLUTE path to a folder which the database will be stored in
    *
    *  @returns None
    *
    */
    void DataStorage::setupDatabase(const char* path){


        // Create new .db file with the current time in the name

        auto now = std::chrono::system_clock::now();
        // Format: Year-Month-Day_Hour-Minute-Second
        std::time_t t = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss;
        ss << std::put_time(std::localtime(&t), "%Y-%m-%d_%H-%M-%S");
        std::string timestamp = ss.str();
        
        std::string fullPath = std::string(path) + "/baja_data_" + timestamp +".db";

        int exit = 0;
        // If database does not already exist, it will be created.
        exit = sqlite3_open(fullPath.c_str(), &db);

        if (exit) { 
            std::cerr << "Error open DB: " << sqlite3_errmsg(db) << std::endl; 
            return;
        } 
        else
        {
            // std::cout << "Opened Database Successfully!" << std::endl; 
        }
        

        // Execute database schema
        char* errMsg = nullptr;
        int rc = sqlite3_exec(db, DATABASE_SCHEMA, nullptr, nullptr, &errMsg);
        if(rc != SQLITE_OK){
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
    }


    /*
    *  Method:  updateDatabase (private)
    *
    *  Purpose: Writes out buffered SQL commands to the database
    *
    *  Pre-Condition: Database is already setup via `setupDatabase()`
    *
    *  Post-Condition: locked in an infinite loop until `running` is set to false
    *
    *  Parameters:
    *           None
    *
    *  Returns: None
    *
    */
    void DataStorage::updateDatabase(){

        char* messageError;
        int exit;

        //While datastorage is still running (this allows for graceful termination)
        while(running.load()){

            std::unique_lock<std::mutex> lock(insertBufferMutex);

            //switch the queues so that we don't hog the mutex forever
            std::queue<sqlite3_stmt *> localInsertBuffer;
            std::swap(localInsertBuffer, insertBuffer);
            lock.unlock();


            sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &messageError);

            //Iterate through all pending statements and execute them
            while(!localInsertBuffer.empty()){

                sqlite3_stmt *statement = localInsertBuffer.front();
                localInsertBuffer.pop();

                int exit = 0;

                exit = sqlite3_step(statement);

                if(exit != SQLITE_DONE){
                    std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
                }
                sqlite3_finalize(statement);
            }

            sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &messageError);

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }


    /*
    *  Method:  queueSqlStatement (private)
    *
    *  Purpose: Enqueues the given sqlite3 statement to be stored in the database.
    *
    *  Pre-Condition: 
    *
    *  Post-Condition: Statement is queued and will be stored eventually.
    *
    *  @param statement    an sqlite3_stmt pointer (NOT NULL)
    *
    *  @returns None
    *
    */
    void DataStorage::queueSqlStatement(sqlite3_stmt *statement){

        if(statement == nullptr){
            // literally the end of the world
            return;
        }

        numDataInserts++;

        std::lock_guard<std::mutex> lock (insertBufferMutex);
        insertBuffer.push(statement);
    }

    /**
     * Note: storeData() is overridden for each data type listed in DataTypes.h
     */

    
    void DataStorage::storeData(ShockDisplacement data){
        sqlite3_stmt *statement;
        
        int exit = sqlite3_prepare_v2(db, INSERT_DISPLACEMENT_SENSORS, -1, &statement, nullptr);
        
        if(exit){
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            return;
        }
        // Bind values to parameters
        sqlite3_bind_int64(statement, 1, CarTime::getUnixEpoch()); 
        sqlite3_bind_double(statement, 2, data.frontLeft);
        sqlite3_bind_double(statement, 3, data.frontRight);
        sqlite3_bind_double(statement, 4, data.rearLeft);
        sqlite3_bind_double(statement, 5, data.rearRight);

        queueSqlStatement(statement);
    }
    
    void DataStorage::storeData(RotationXYZ rotData, AccelerationXYZ accData){
        sqlite3_stmt *statement;
        
        int exit = sqlite3_prepare_v2(db, INSERT_IMU, -1, &statement, nullptr);
        
        if(exit){
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            return;
        }
        // Bind values to parameters
        sqlite3_bind_int64(statement, 1, CarTime::getUnixEpoch());      // TODO: Need unix epoch somehow
        sqlite3_bind_double(statement, 2, accData.accelerationx);
        sqlite3_bind_double(statement, 3, accData.accelerationy);
        sqlite3_bind_double(statement, 4, accData.accelerationz);
        sqlite3_bind_double(statement, 5, rotData.rotationx);
        sqlite3_bind_double(statement, 6, rotData.rotationy);
        sqlite3_bind_double(statement, 7, rotData.rotationz);


        queueSqlStatement(statement);
    }

    void DataStorage::storeData(BrakePressure data){
        sqlite3_stmt *statement;
        
        int exit = sqlite3_prepare_v2(db, INSERT_BRAKE_PRESSURE, -1, &statement, nullptr);
        
        if(exit){
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            return;
        }

        // Bind values to parameters
        sqlite3_bind_int64(statement, 1, CarTime::getUnixEpoch());
        sqlite3_bind_double(statement, 2, data.front);
        sqlite3_bind_double(statement, 3, data.rear);

        queueSqlStatement(statement);
    }

    void DataStorage::storeData(WheelSpeed data){
        sqlite3_stmt *statement;
        
        int exit = sqlite3_prepare_v2(db, INSERT_WHEEL_SPEED, -1, &statement, nullptr);
        
        if(exit){
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            return;
        }

        // Bind values to parameters
        sqlite3_bind_int64(statement, 1, CarTime::getUnixEpoch());      // TODO: Need unix epoch somehow
        sqlite3_bind_double(statement, 2, data.frontLeft);
        sqlite3_bind_double(statement, 3, data.frontRight);
        sqlite3_bind_double(statement, 4, data.rear);

        queueSqlStatement(statement);
    }

    void DataStorage::storeData(EngineRPM data){
        sqlite3_stmt *statement;
        
        int exit = sqlite3_prepare_v2(db, INSERT_ENGINE_TACHOMETER, -1, &statement, nullptr);
        
        if(exit){
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            return;
        }

        // Bind values to parameters
        sqlite3_bind_int64(statement, 1, CarTime::getUnixEpoch());      // TODO: Need unix epoch somehow
        sqlite3_bind_double(statement, 2, data.rpm);

        queueSqlStatement(statement);
    }

    void DataStorage::storeData(CVTTemp data){
        sqlite3_stmt *statement;
        
        int exit = sqlite3_prepare_v2(db, INSERT_TEMPERATURE, -1, &statement, nullptr);
        
        if(exit){
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            return;
        }
        // Bind values to parameters
        sqlite3_bind_int64(statement, 1, CarTime::getUnixEpoch());      // TODO: Need unix epoch somehow
        sqlite3_bind_double(statement, 2, data.temp);

        queueSqlStatement(statement);
    }

    void DataStorage::storeData(GPSPosition data){
        sqlite3_stmt *statement;
        
        int exit = sqlite3_prepare_v2(db, INSERT_GPS, -1, &statement, nullptr);
        
        if(exit){
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            return;
        }
        // Bind values to parameters
        sqlite3_bind_int64(statement, 1, CarTime::getUnixEpoch());      // TODO: Need unix epoch somehow
        sqlite3_bind_double(statement, 2, data.latitude);
        sqlite3_bind_double(statement, 3, data.longitude);
        sqlite3_bind_double(statement, 4, data.atltitude);

        queueSqlStatement(statement);
    }

}