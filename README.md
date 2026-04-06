# Hospital Management System (C++ + DAA Approach)

This project is a hospital management system built in C++ with a strong data-structures-and-algorithms (DAA) focus. It supports both a CLI flow and a web API + frontend flow on top of the same core logic and data files.

![Hospital Management System Screenshot](image.png)


## Features

- Add and search patients
- Add doctors and manage specializations
- Book appointments with ID validation
- View appointments sorted by date and time
- Manage emergency triage with severity priority
- Persist data with automatic load/save
- Access the same system from CLI or web frontend

## DAA Approaches Used (What and Why)

This project uses practical DAA choices to keep everyday hospital operations fast and predictable as records grow.

**Where and how DAA approaches are used:**

- **Hash map (`unordered_map`)**
	- Used in: `patientsById_` and `doctorsById_` in [src/hospital_system.h](src/hospital_system.h)
	- Purpose: Fast patient/doctor lookup by ID (average O(1) insert/find)
	- Example: See all patient/doctor add/search functions in [src/hospital_system.cpp](src/hospital_system.cpp)

- **Hash set (`unordered_set`)**
	- Used in: `appointmentIds_` in [src/hospital_system.h](src/hospital_system.h)
	- Purpose: Ensures appointment IDs are unique before booking (average O(1) insert/find)
	- Example: See appointment booking logic in [src/hospital_system.cpp](src/hospital_system.cpp)

- **Binary search (`lower_bound`)**
	- Used in: `findPatientByNameBinarySearch` and patient name index insertion in [src/hospital_system.cpp](src/hospital_system.cpp)
	- Purpose: Fast patient search by name using a sorted index (O(log n) search)

- **Sorted index maintenance**
	- Used in: `patientNameIndex_` (vector of pairs) in [src/hospital_system.h](src/hospital_system.h)
	- Purpose: Keeps the name index always sorted for binary search (O(n) insertion)

- **Sorting (`std::sort`)**
	- Used in: Appointment and doctor listing functions in [src/hospital_system.cpp](src/hospital_system.cpp)
	- Purpose: Ensures output is always sorted by date/time or ID (O(n log n))

- **Priority queue (heap)**
	- Used in: `emergencyQueue_` in [src/hospital_system.h](src/hospital_system.h) and related logic in [src/hospital_system.cpp](src/hospital_system.cpp)
	- Purpose: Emergency triage—always treat the highest-severity case first (O(log n) push/pop)

- **Greedy strategy**
	- Used in: Emergency treatment policy (see `treatNextEmergency` in [src/hospital_system.cpp](src/hospital_system.cpp))
	- Purpose: Always process the most critical case available, matching real-world triage

## Build and Run

### Windows (PowerShell + MSYS2/MinGW)

```powershell
cmake -S . -B build-mingw -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=g++
cmake --build build-mingw
.\build-mingw\hms.exe
```

### Run Web Frontend (Connected to Current C++ Codebase)

Build and run the web server target:

```powershell
cmake -S . -B build-mingw -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=g++
cmake --build build-mingw --target hms_web
.\build-mingw\hms_web.exe
```

Then open:

```text
http://localhost:8080
```

If you open `frontend/index.html` using VS Code Live Preview (port 3000), the frontend will automatically call the API at `http://127.0.0.1:8080`.

Notes:

- Web API endpoints are served at `/api/*`.
- Frontend files are served from `frontend/`.
- The web app uses the same `data/` files as the CLI app.

### Linux/macOS

```bash
cmake -S . -B build
cmake --build build
./build/hms
```

## Direct g++ Build (Optional)

```bash
g++ -std=c++17 -O2 src/main.cpp src/hospital_system.cpp -o hms
./hms
```

## File Handling (Data Persistence)

The application automatically loads data when it starts.
The application also saves immediately after every successful data-changing action (add/book/treat), and saves again on exit.

- Data folder: `data/`
- Patients file: `data/patients.txt`
- Doctors file: `data/doctors.txt`
- Appointments file: `data/appointments.txt`
- Emergency queue file: `data/emergency_queue.txt`

If the `data/` folder does not exist, it is created automatically on first save.
