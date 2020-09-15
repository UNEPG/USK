## Branches
* Maser: NOT YET MERGED - > For official release ONLY.
* LCD: Tested displaying the incoming packets on a LCD1602.
* SoftwareSerial: Tested the Serial Communication (Software Implemented) both on Arduino and RPi.
* Receive Commands from RS485 and Reply to the Commands if:
    1. Destination is 0x04 (The Sensor Board) ADDR_SB 0x04
    2. The Data Length >=1
    3. The Command is a valid Query Command for Sensor Board. 
    


```bash
/*

                        Created by Azat (@azataiot) on Sep.5 2020
 */

/*
 * Note:
 * 1. Language of the Source Code : C++
 * 2. C++ Supported Version ( >= C++ 11 )
 * 3. Build Tool: GCC >=7.2
 * 4. MAKE version: >= 2.8.4
 * 5. Target Platform: Arduino
 * 6. Build Tested on:
 *    macOS (clang-1103.0.32.62) PASS ✅ (05.09.2020)
 *    Linux (gcc 10.2.0) PASS ✅ (05.09.2020)
 *    Raspberry OS Linux (aa64 bit, gcc 9.1) Failed 🚫 (05.09.2020)
 * 7. Documentation and Other Reference:   https://azat.cc/2020/09/03/unisat-unisat-protocol.html
 */
```

For detailed info: 
https://azat.cc/2020/09/03/unisat-unisat-protocol.html