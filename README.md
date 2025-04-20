# LinuxTaskManager

A complete, modular Linux process monitor & manager with C core, ANSI‑CLI, JSON daemon, Spring Boot REST API, and Thymeleaf web UI.

## Features
- **C core library**: parses `/proc` into `proc_stat` structs  
- **Interactive CLI**: live, ANSI‑based terminal UI (kill, renice, filter)  
- **Background daemon**: polls core and serves JSON over a UNIX socket  
- **Spring Boot REST API**: exposes process data to the web front end  
- **Thymeleaf Web UI**: dashboard, filtering, and controls in the browser

## Planned Project Structure
/ (repo root)

# Core parsing library: reads and parses /proc data into C structs
├── c-core/                
│   ├── include/           # Public headers
│   │   └── lmtcore.h      # proc_stat, API prototypes
│   ├── src/               # Implementation
│   │   ├── parse_proc.c   # reading /proc/[pid]/stat, /proc/stat
│   │   ├── sleeper.c      # detect sleepers by state+thresholds
│   │   └── utils.c        # is_number(), split_stat_line(), read_stat()
│   ├── tests/             # Unit tests (mocked stat lines)
│   │   └── test_parse.c
│   └── CMakeLists.txt     # Build `liblmtcore.a` + tests

# Interactive CLI: live, ANSI-based terminal UI for process monitoring
├── c-cli/                 
│   ├── src/               
│   │   └── main.c         # redraw loop, interactive commands (kill, renice)
│   └── CMakeLists.txt     # Links against `lmtcore`

# Background service: polls liblmtcore and exposes a JSON API over UNIX socket
├── c-daemon/              
│   ├── src/
│   │   └── daemon.c       # poll lmtcore, serve JSON over UNIX socket
│   └── CMakeLists.txt

# REST API backend: Spring Boot service exposing process data to front-end
├── java-service/          
│   ├── src/
│   │   ├── main/java/com/example/lmt/
│   │   │   ├── LmtApplication.java
│   │   │   ├── controller/ ProcessController.java   # /api/processes endpoints
│   │   │   ├── service/    LmtClient.java           # calls UNIX socket or CLI
│   │   │   └── model/      ProcessDto.java          # maps to proc_stat fields
│   │   └── resources/
│   │       └── application.yml  # socket path, poll intervals
│   └── pom.xml or build.gradle

# Front-end templates & assets: Thymeleaf HTML, CSS, and JS for dashboard
├── web-ui/                
│   ├── src/main/resources/
│   │   ├── templates/
│   │   │   ├── index.html       # main dashboard
│   │   │   ├── fragments/       # process row, header, modals
│   │   └── static/
│   │       ├── css/             # custom styles
│   │       └── js/              # AJAX refresh, kill/renice calls
│   └── Dockerfile             # bundles into WAR with Spring Boot

├── docker-compose.yml     # Orchestrates c-daemon, java-service, and web-ui
└── README.md              # High-level overview + build & run instructions
