------------------------------------------------------------------------------------
-- The big idea behind this schema is that the database is stored in a single file.
-- Every time the "new session" command is send from the pit, a new .db file is
-- created. Associated with this file is a timestamp in the form YYYY-MM-DDTHH:SSZ
-- (conforming to the ISO 8601 standard)
-- The z means "Zulu time", otherwise known as an offset of 0 from UTC. (The name of 
-- the file is really only used as a unique identifier, so it's not actually that
-- important)
--
-- [The paragraph below is still theoretical and a WIP. Not happening in 2026-2027]
-- All of these sessions are stored locally on a flash drive on the car. Once 
-- somebody with the flash drive has access to the Postgres server, they can sync
-- up all of the locally stored data with the rest of the data. The syncing process
-- will also clear the data on the flash drive, eliminating the need to manually 
-- clean it. Each .db file will be treated as its own unique session in the Postgres
-- database
--
-- [Fever dream goal below]
-- Eventually, it would be really cool to set up a web service that uses the postgres
-- database in combination with a data visualizer (Grafana?). It would also be really
-- cool to have this as a service that other teams could set up with their own 
-- telemetry systems.
--
-- Originally written by Matthew Larson
-- Started August 27, 2025

-- 2026 - 2027 Season Update
-- With the webapp being deprioritized in favor of simply making the system work well,
-- changes were made to the format of the tables. 
--
-- A lot of the tables were formatted in a way that implied that all sessions would be
-- stored in one giant table, but they aren't on here. Additionally, the sensor field
-- made it way more complicated when they aren't stored in one spot. Thus, the sensor
-- table and field have been removed, and each data point no longer references a single
-- vehicle, because there is no concept of multiple vehicles in this scope.
--
-- If the data were to be added to a central database, this info could be added again. 
-- The simple idea is one table that describes where all the measurements came
-- from, since each of these database files should only be from one vehicle. That data
-- can then be applied in post-processing to properly tag the data in a central database.
--
-- Additionally, some keys were stored in annoying ways, for example, anything that 
-- measured RPM was stored in a generic table with a tag describing which RPM it was.
-- While it may be good for a database, it is not good for analyzing the data because
-- these different values really need different columns, not one column. We also control
-- all the data ourselves, so it isn't hard to add a new column. Thus, data collected 
-- at the same time gets its own table with its own columns for the data points.
--
-- If post processing software is developed in the future to parse these databases 
-- into different columns, it may be a good idea to revisit that. 
--
-- I'll leave the old SqlCommands (SqlCommands.old) in the include folder.
--
-- Karsten Yin  (November 6th 2026)
------------------------------------------------------------------------------------



-- I think doing foreign key checks while running the car are a waste of resources
-- PRAGMA foreign_keys = ON;

-- Performance optimization (add more information here)
PRAGMA journal_mode = WAL;
-- Another performance optimization. Should probably give some more information
PRAGMA synchronous = NORMAL;

PRAGMA compile_options;

PRAGMA foreign_keys = OFF;
PRAGMA journal_mode = WAL;
PRAGMA synchronous = NORMAL;

CREATE TABLE IF NOT EXISTS vehicle_info(
    name TEXT PRIMARY KEY,
    competition_year INTEGER NOT NULL
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