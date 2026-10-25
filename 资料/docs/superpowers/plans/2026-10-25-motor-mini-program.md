# Motor mini-program implementation plan

Goal: add direct BLE motor control with the user's approved default keep-running policy.

Architecture: one Display-task-owned motor state and driver; BLE queues commands, notifications carry applied state. OLED uses the same state. Existing FF12/FF13 carries a separate B8 protocol without changing GATT handles.

Constraints: power-on stopped, default speed30%, keep=true; background/exit/disconnect keep running when enabled. keep=false sends stop on hide and firmware stops on disconnect or 3s without owner heartbeat. Running direction changes rejected. Reconnect/query never starts. No automatic hardware execution, flashing, commits or push.

- [x] Pure motor policy tests: boot, keep/drop, wrap-safe watchdog, direction rejection, owner query.
- [x] Motor service queues and BLE B8 commands with sequence/session validation; applied state notifications. Integrate OLED state and Display-task pump, preserve local exit-stop behavior only for local ownership.
- [x] Mini-program controller serializes commands, waits for applied replies, polls state while visible; hide sends stop iff policy disabled. Connection or command failure never implies started/stopped.
- [x] Add motor page: connection/status, start/stop, stopped-only direction, speed0..100 step5, exit-keep switch default true and explicit persistence warning; home entry.
- [x] Run full Node tests, host tests, PlatformIO build, diff checks. Manual hardware acceptance remains user-operated.

Independent review found three Important issues, all addressed with regression tests: partial updates now carry an explicit field mask to prevent stale START replay; OLED Back no longer takes motor ownership; waiter cancellation is generation-scoped so old writes cannot cancel new sessions. Added real MotorService host integration tests using test-only queue/PWM substitutes. Node40 tests, host33 suites and migration safety passed; ESP32 build RAM99544 bytes / Flash807797 bytes. No device flashing or motor activation performed.

Review focus: stale callbacks after hide; reconnect must not start; stop lost on exit; OLED/remote ownership; both-direction request while running. Tests accompany corresponding components. UI state describes commanded output, not measured shaft speed or current. No authentication or hardware emergency stop added.
