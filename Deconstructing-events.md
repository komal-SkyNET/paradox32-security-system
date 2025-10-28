# Deconstructing Paradox Events from MQTT

This document explains how to interpret the MQTT messages sent by the ESP32 Paradox bridge. The bridge acts as a passthrough, sending raw event codes from the alarm panel. You can use this guide to create sensors, automations, and dashboards in Home Assistant.

## MQTT Message Structure

All events are published to a topic with the following structure:

**Topic:** `paradox/events/{EVENT_NUMBER}`
**Payload:** `{"value":"{PAYLOAD}"}`

-   `paradox/`: This is the default MQTT topic prefix.
-   `{EVENT_NUMBER}`: The raw event code from the panel. See the lookup tables below.
-   `{PAYLOAD}`: Contains the specific details of the event, such as the Zone number or a more detailed status message.

## Event & Payload Lookup Tables

--- 

### Main Events (`{EVENT_NUMBER}`)

This table maps the primary event number to its meaning.

| Event # | Meaning                               |
| :------ | :------------------------------------ |
| 0       | Zone OK                               |
| 1       | Zone open                             |
| 2       | Partition status                      |
| 3       | Bell status                           |
| 6       | Non-reportable event                  |
| 12      | Cold start wireless zone              |
| 13      | Cold start wireless module            |
| 14      | Bypass programming                    |
| 15      | User code activated output            |
| 16      | Wireless smoke maintenance signal     |
| 17      | Delay zone alarm transmission         |
| 18      | Zone signal strength weak 1           |
| 19      | Zone signal strength weak 2           |
| 20      | Zone signal strength weak 3           |
| 21      | Zone signal strength weak 4           |
| 24      | Fire delay started                    |
| 26      | Software access                       |
| 27      | Bus module event                      |
| 28      | StayD pass acknowledged               |
| 29      | Arming with user                      |
| 30      | Special arming                        |
| 31      | Disarming with user                   |
| 32      | Disarming after an alarm with user    |
| 33      | Alarm cancelled with user             |
| 34      | Special disarming                     |
| 35      | Zone bypassed                         |
| 36      | Zone in alarm                         |
| 37      | Fire alarm                            |
| 38      | Zone alarm restore                    |
| 39      | Fire alarm restore                    |
| 40      | Special alarm                         |
| 41      | Zone shutdown                         |
| 42      | Zone tampered                         |
| 43      | Zone tamper restore                   |
| 44      | New trouble                           |
| 45      | Trouble restored                      |
| 46      | Bus/EBus/wireless module new trouble  |
| 47      | Bus/EBus/wireless module trouble restored |
| 48      | Special                               |
| 49      | Low battery on zone                   |
| 50      | Low battery on zone restore           |
| 51      | Zone supervision trouble              |
| 52      | Zone supervision restore              |
| 53      | Wireless module supervision trouble   |
| 54      | Wireless module supervision restore   |
| 55      | Wireless module tamper trouble        |
| 56      | Wireless module tamper restore        |
| 57      | Non-medical alarm                     |
| 58      | Zone forced                           |
| 59      | Zone included                         |
| 60      | Remote low battery                    |
| 61      | Remote low battery restore            |
| 64      | System status                         |

---

### Sub-Events / Payloads

When an event occurs, the payload provides more detail.

#### `Partition_status` (Event 2)

| Payload # | Meaning                 |
| :-------- | :---------------------- |
| 2         | Silent alarm            |
| 3         | Buzzer alarm            |
| 4         | Steady alarm            |
| 5         | Pulsed alarm            |
| 6         | Strobe                  |
| 7         | Alarm stopped           |
| 8         | Squawk ON               |
| 9         | Squawk OFF              |
| 10        | Ground start            |
| 11        | Disarm partition        |
| 12        | Arm partition           |
| 13        | Entry delay started     |
| 14        | Exit delay started      |
| 15        | Pre-alarm delay         |
| 16        | Report confirmation     |
| 99        | Any partition status event |

#### `Bell_status` (Event 3)

| Payload # | Meaning                 |
| :-------- | :---------------------- |
| 0         | Bell OFF                |
| 1         | Bell ON                 |
| 2         | Bell squawk arm         |
| 3         | Bell squawk disarm      |

#### `Non-reportable_event` (Event 6)

| Payload # | Meaning                             |
| :-------- | :---------------------------------- |
| 0         | Telephone line trouble              |
| 1         | CLEAR + ENTER                       |
| 3         | Arm in Stay mode                    |
| 4         | Arm in Sleep mode                   |
| 5         | Arm in Force mode                   |
| 6         | Full arm when armed in Stay mode    |
| 7         | PC fail to communicate              |
| 8         | Utility key 1-2 pressed             |
| 9         | Utility key 4-5 pressed             |
| 10        | Utility key 7-8 pressed             |
| 11        | Utility key 2-3 pressed             |
| 12        | Utility key 5-6 pressed             |
| 13        | Utility key 8-9 pressed             |
| 14        | Tamper generated alarm              |
| 15        | Supervision loss generated alarm    |
| 20        | Full arm when armed in Sleep mode   |
| 23        | StayD mode activated                |
| 24        | StayD mode deactivated              |
| 25        | IP registration status change       |
| 26        | GPRS registration status change     |
| 27        | Armed with trouble(s)               |
| 28        | Supervision alert                   |
| 29        | Supervision alert restore           |
| 30        | Armed with remote with low battery  |

#### `Special_arming` (Event 30)

| Payload # | Meaning                         |
| :-------- | :------------------------------ |
| 0         | Auto-arming                     |
| 1         | Late to close                   |
| 2         | No movement arming              |
| 3         | Partial arming                  |
| 4         | Quick arming                    |
| 5         | Arming through WinLoad/BabyWare |
| 6         | Arming with keyswitch           |

#### `Special_disarming` (Event 34)

| Payload # | Meaning                                   |
| :-------- | :---------------------------------------- |
| 0         | Auto-arm cancelled                        |
| 1         | Disarming through WinLoad/BabyWare        |
| 2         | Disarming through WinLoad/BabyWare after alarm |
| 3         | Alarm cancelled through WinLoad/BabyWare  |
| 4         | Paramedical alarm cancelled               |
| 5         | Disarm with keyswitch                     |
| 6         | Disarm with keyswitch after an alarm      |
| 7         | Alarm cancelled with keyswitch            |

#### `Special_alarm` (Event 40)

| Payload # | Meaning                       |
| :-------- | :---------------------------- |
| 0         | Panic non-medical emergency   |
| 1         | Panic medical                 |
| 2         | Panic fire                    |
| 3         | Recent closing                |
| 4         | Global shutdown               |
| 5         | Duress alarm                  |
| 6         | Keypad lockout                |

#### `New_trouble` / `Trouble_restored` (Events 44 & 45)

| Payload # | Meaning                               |
| :-------- | :------------------------------------ |
| 1         | AC failure                            |
| 2         | Battery failure                       |
| 3         | Auxiliary current overload            |
| 4         | Bell current overload                 |
| 5         | Bell disconnected                     |
| 6         | Clock loss                            |
| 7         | Fire loop trouble                     |
| 8         | Fail call station telephone # 1       |
| 9         | Fail call station telephone # 2       |
| 11        | Fail to communicate with voice report |
| 12        | RF jamming                            |
| 13        | GSM RF jamming                        |
| 14        | GSM no service                        |
| 15        | GSM supervision lost                  |
| 16        | Fail to communicate IP receiver 1     |
| 17        | Fail to communicate IP receiver 2     |
| 18        | IP module no service                  |
| 19        | IP module supervision loss            |
| 22        | GSM/GPRS module tamper trouble        |

#### `Special` (Event 48)

| Payload # | Meaning             |
| :-------- | :------------------ |
| 2         | Software log on     |
| 3         | Software log off    |