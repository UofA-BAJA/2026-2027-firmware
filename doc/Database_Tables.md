thank you chat I was NOT making these tables from the schema myself.<br/>
idea is that each set of inputs that comes in roughly at the same time gets its own tables 
### 1. `vehicle_info`

Stores information about the vehicle this data is associated with.<br/>
This is manually put into `DataStorage.h`, and then the Data Storage system puts one row in and fills out the columns. Each new "session" is a new SQLite file, thus if the car or comp is changed, each new SQLite file will have the new info.<br/>Some of this is a layover from when all data was planned to be stored in one file.

| Column | Data type | Description | Constraints |
|---|---|---|---|
| `name` | TEXT | Vehicle name | Primary Key |
| `competition_year` | INTEGER | Year the vehicle competed | |
| `competition_location` | TEXT | Location of the competition | |

### 2. `log`

Stores the log messages with the different severities <br/> (NOT USED RIGHT NOW)

| Column | Data type | Description | Constraints |
|---|---|---|---|
| `id` | INTEGER | Unique log entry ID | Primary Key |
| `epoch_ms` | INTEGER | Timestamp (Unix epoch milliseconds) | NOT NULL |
| `type` | TEXT | Category or severity of the log | NOT NULL |
| `message` | TEXT | Log message contents | Optional |

### 3. `imu`

Stores measurements from the IMU<br/>Note: Which axis is which direction in terms of the physical car is determined later.

| Column | Data type | Description | Constraints |
|---|---|---|---|
| `epoch_ms` | INTEGER | Timestamp (Unix epoch milliseconds) | Primary Key |
| `ax` | REAL | Acceleration along the X-axis |  |
| `ay` | REAL | Acceleration along the Y-axis |  |
| `az` | REAL | Acceleration along the Z-axis |  |
| `rx` | REAL | Rotation around the X-axis | range tbd |
| `ry` | REAL | Rotation around the Y-axis | range tbd |
| `rz` | REAL | Rotation around the Z-axis | range tbd |
| `a_unit` | TEXT | Acceleration unit | Constant value `m/s²` (generated, virtual) |
| `r_unit` | TEXT | Acceleration unit | Constant value `degrees` (generated, virtual) |

### 4. `gps`

Stores GPS measurements.

| Column | Data type | Description | Constraints |
|---|---|---|---|
| `epoch_ms` | INTEGER | Timestamp (Unix epoch milliseconds) | Primary Key |
| `lat` | REAL | Latitude |  |
| `lon` | REAL | Longitude |  |
| `alt` | REAL | Altitude |  |
| `alt_unit` | TEXT | Altitude Unit | Constant value `ft` (generated, virtual) |

### 5. `brake_pressure`

Stores brake pressure measurements from the two brake pressure sensors.

| Column | Data type | Description | Constraints |
|---|---|---|---|
| `epoch_ms` | INTEGER | Timestamp (Unix epoch milliseconds) | Primary Key |
| `front` | REAL | Front brake pressure | |
| `rear` | REAL | Rear brake pressure | |
| `pressure_unit` | TEXT | Brake pressure unit | Constant value `psi` (generated, virtual) |

### 6. `shock_displacement`

Stores shock displacement measurements.

| Column | Data type | Description | Constraints |
|---|---|---|---|
| `epoch_ms` | INTEGER | Timestamp (Unix epoch milliseconds) | Primary Key |
| `front_left` | REAL | Front left shock displacement |  |
| `front_right` | REAL | Front right shock displacement |  |
| `rear_left` | REAL | Rear left shock displacement |  |
| `rear_right` | REAL | Rear right shock displacement |  |
| `displacement_unit` | TEXT | Displacement unit | Constant value `cm` (generated, virtual) |

### 7. `temperature`

Stores temperature measurements. For now, only CVT temp.

| Column | Data type | Description | Constraints |
|---|---|---|---|
| `epoch_ms` | INTEGER | Timestamp (Unix epoch milliseconds) | Primary Key |
| `cvt_temp` | REAL | Measured temperature |  |
| `temp_unit` | TEXT | Temperature unit | Constant value `°C` (generated, virtual) |

### 8. `engine_tachometer`

Stores engine tachometer readings.

| Column | Data type | Description | Constraints |
|---|---|---|---|
| `epoch_ms` | INTEGER | Timestamp (Unix epoch milliseconds) | Primary Key |
| `rpm` | REAL | Engine speed in RPM | Optional |


### 9. `wheel_speed`

Stores wheel speed measurments.<br/>
Originally, this was one big `tachometer` table that specified which sensor each measurement came from, but this proved more annoying to deal with for 26-27.

| Column | Data type | Description | Constraints |
|---|---|---|---|
| `epoch_ms` | INTEGER | Measurement timestamp (Unix epoch milliseconds) | Primary Key |
| `front_left` | REAL | Wheel RPM of front left wheel | Optional |
| `front_right` | REAL | Wheel RPM of front right wheel | Optional |
| `rear` | REAL | Wheel RPM of rear wheels (via driveshaft) | Optional |
| `speed_unit` | TEXT | Unit of speed measurment | Constant `rpm` (generated, virtual) |

### 10. `vehicle_state`

Stores calculated or overall vehicle-state measurements.

| Column | Data type | Description | Constraints |
|---|---|---|---|
| `epoch_ms` | INTEGER | State timestamp (Unix epoch milliseconds) | Primary Key |
| `speed` | REAL | Vehicle speed | |
| `dist` | REAL | Distance traveled | |
| `speed_unit` | TEXT | Speed unit | Constant `m/s` (Generated, Virtual) |
| `dist_unit` | TEXT | Distance unit | Constant `m` (Generated, Virtual) |

---

**Notes:**
- `ON DELETE CASCADE` means deleting a referenced vehicle or sensor automatically deletes its associated records.
- `GENERATED ALWAYS ... VIRTUAL` columns are calculated automatically by SQLite and are not physically stored in the database.