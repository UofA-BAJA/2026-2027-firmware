#ifndef SQLCOMMANDS_H
#define SQLCOMMANDS_H

// It's hard to read from files during runtime because you don't necessarily know where the file is.
// Because of that, it's a lot easier to hard code the sql commands in some file like this one.

namespace BajaWildcatRacing
{

    // inline allows for multiple declarations
    inline const char* DATABASE_SCHEMA = R"SQL(
    PRAGMA journal_mode = WAL;
    PRAGMA synchronous = NORMAL;

    PRAGMA compile_options;

    PRAGMA foreign_keys = OFF;
    PRAGMA journal_mode = WAL;
    PRAGMA synchronous = NORMAL;

    CREATE TABLE IF NOT EXISTS vehicle_info(
        name TEXT PRIMARY KEY,
        competition_year INTEGER NOT NULL,
        competition_location TEXT NOT NULL
    );

    CREATE TABLE IF NOT EXISTS log(
        id INTEGER PRIMARY KEY,
        epoch_ms INTEGER NOT NULL DEFAULT (unixepoch('now') * 1000),
        vehicle TEXT NOT NULL,
        type TEXT NOT NULL,
        message TEXT
    );

    CREATE TABLE IF NOT EXISTS imu(
        epoch_ms INTEGER PRIMARY KEY NOT NULL DEFAULT (unixepoch('now') * 1000),
        ax REAL,
        ay REAL,
        az REAL,
        rx REAL,
        ry REAL,
        rz REAL,
        a_unit TEXT GENERATED ALWAYS AS ('m/s^2') VIRTUAL
    );

    CREATE TABLE IF NOT EXISTS gps(
        epoch_ms INTEGER PRIMARY KEY NOT NULL DEFAULT (unixepoch('now') * 1000),
        lat REAL,
        lon REAL,
        alt REAL,
        alt_unit TEXT GENERATED ALWAYS AS ('ft') VIRTUAL
    );

    CREATE TABLE IF NOT EXISTS brake_pressure(
        epoch_ms INTEGER PRIMARY KEY NOT NULL DEFAULT (unixepoch('now') * 1000),
        front REAL,
        rear REAL,
        pressure_unit TEXT GENERATED ALWAYS AS ('psi') VIRTUAL
    );

    CREATE TABLE IF NOT EXISTS shock_displacement(
        epoch_ms INTEGER PRIMARY KEY NOT NULL DEFAULT (unixepoch('now') * 1000),
        front_left REAL,
        front_right REAL,
        rear_left REAL,
        rear_right REAL,
        displacement_unit TEXT GENERATED ALWAYS AS ('cm') VIRTUAL
    );

    CREATE TABLE IF NOT EXISTS temperature(
        epoch_ms INTEGER PRIMARY KEY NOT NULL DEFAULT (unixepoch('now') * 1000),
        cvt_temp REAL,
        temp_unit TEXT GENERATED ALWAYS AS ('°C') VIRTUAL
    );

    CREATE TABLE IF NOT EXISTS engine_tachometer(
        epoch_ms INTEGER PRIMARY KEY NOT NULL DEFAULT (unixepoch('now') * 1000),
        rpm REAL
    );

    CREATE TABLE IF NOT EXISTS wheel_speed(
        epoch_ms INTEGER PRIMARY KEY NOT NULL DEFAULT (unixepoch('now') * 1000),
        front_left REAL,
        front_right REAL,
        rear REAL,
        speed_unit TEXT GENERATED ALWAYS AS ('rpm') VIRTUAL
    );


    CREATE TABLE IF NOT EXISTS vehicle_state(
        epoch_ms INTEGER PRIMARY KEY NOT NULL DEFAULT (unixepoch('now') * 1000),
        speed REAL,
        dist REAL,
        speed_unit TEXT GENERATED ALWAYS AS ('m/s') VIRTUAL,
        dist_unit TEXT GENERATED ALWAYS AS ('m') VIRTUAL
    );
    
    )SQL";

    inline const char* INSERT_VEHICLE = R"SQL(
    INSERT INTO vehicle_info (name, competition_year, competition_location)
    VALUES (?, ?, ?);
    )SQL";

    inline const char* INSERT_LOG = R"SQL(
        INSERT INTO log (epoch_ms, type, message)
        VALUES (?, ?, ?);
    )SQL";

    inline const char* INSERT_IMU = R"SQL(
        INSERT INTO imu (epoch_ms, ax, ay, az, rx, ry, rz)
        VALUES (?, ?, ?, ?, ?, ?, ?);
    )SQL";

    inline const char* INSERT_GPS = R"SQL(
        INSERT INTO gps (epoch_ms, lat, lon, alt)
        VALUES (?, ?, ?, ?);
    )SQL";

    inline const char* INSERT_BRAKE_PRESSURE = R"SQL(
        INSERT INTO brake_pressure (epoch_ms, front, rear)
        VALUES (?, ?, ?);
    )SQL";

    inline const char* INSERT_DISPLACEMENT_SENSORS = R"SQL(
        INSERT INTO shock_displacement (epoch_ms, front_left, front_right, rear_left, rear_right)
        VALUES (?, ?, ?, ?, ?);
    )SQL";

    inline const char* INSERT_TEMPERATURE = R"SQL(
        INSERT INTO temperature (epoch_ms, cvt_temp)
        VALUES (?, ?);
    )SQL";

    inline const char* INSERT_ENGINE_TACHOMETER = R"SQL(
        INSERT INTO engine_tachometer (epoch_ms, rpm)
        VALUES (?, ?);
    )SQL";

    inline const char* INSERT_WHEEL_SPEED = R"SQL(
        INSERT INTO wheel_speed (epoch_ms, front_left, front_right, rear)
        VALUES (?, ?, ?, ?);
    )SQL";

    inline const char* INSERT_VEHICLE_STATE = R"SQL(
        INSERT INTO vehicle_state (epoch_ms, speed, dist)
        VALUES (?, ?, ?);
    )SQL";

}

#endif