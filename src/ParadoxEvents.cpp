#include "ParadoxEvents.h"

String getEventDescription(int event, int sub_event) {
    String description = "";
    switch (event) {
        case 0: description = "Zone OK: " + String(sub_event); break;
        case 1: description = "Zone open: " + String(sub_event); break;
        case 2: {
            description = "Partition status: ";
            switch (sub_event) {
                case 2: description += "Silent alarm"; break;
                case 3: description += "Buzzer alarm"; break;
                case 4: description += "Steady alarm"; break;
                case 5: description += "Pulsed alarm"; break;
                case 6: description += "Strobe"; break;
                case 7: description += "Alarm stopped"; break;
                case 8: description += "Squawk ON"; break;
                case 9: description += "Squawk OFF"; break;
                case 10: description += "Ground start"; break;
                case 11: description += "Disarm partition"; break;
                case 12: description += "Arm partition"; break;
                case 13: description += "Entry delay started"; break;
                case 14: description += "Exit delay started"; break;
                case 15: description += "Pre-alarm delay"; break;
                case 16: description += "Report confirmation"; break;
                default: description = ""; break;
            }
            break;
        }
        case 3: {
            description = "Bell status: ";
            switch (sub_event) {
                case 0: description += "Bell OFF"; break;
                case 1: description += "Bell ON"; break;
                case 2: description += "Bell squawk arm"; break;
                case 3: description += "Bell squawk disarm"; break;
                default: description = ""; break;
            }
            break;
        }
        case 6: {
            description = "Non-reportable event: ";
            switch (sub_event) {
                case 3: description += "Arm in Stay mode"; break;
                case 4: description += "Arm in Sleep mode"; break;
                case 5: description += "Arm in Force mode"; break;
                default: description = ""; break;
            }
            break;
        }
        case 29: description = "Arming with user: " + String(sub_event); break;
        case 30: {
            description = "Special arming: ";
            switch (sub_event) {
                case 0: description += "Auto-arming"; break;
                case 4: description += "Quick arming"; break;
                default: description = ""; break;
            }
            break;
        }
        case 31: description = "Disarming with user: " + String(sub_event); break;
        case 34: {
            description = "Special disarming: ";
            switch (sub_event) {
                case 5: description += "Disarm with keyswitch"; break;
                default: description = ""; break;
            }
            break;
        }
        case 36: description = "Zone in alarm: " + String(sub_event); break;
        case 37: description = "Fire alarm: " + String(sub_event); break;
        case 38: description = "Zone alarm restore: " + String(sub_event); break;
        case 39: description = "Fire alarm restore: " + String(sub_event); break;
        case 44: description = "New trouble"; break;
        case 45: description = "Trouble restored"; break;
        case 48: {
            description = "Special: ";
            switch (sub_event) {
                case 2: description += "Software log on"; break;
                case 3: description += "Software log off"; break;
                default: description = ""; break;
            }
            break;
        }
        case 49: description = "Low battery on zone: " + String(sub_event); break;
        case 50: description = "Low battery on zone restore: " + String(sub_event); break;
        default: description = ""; break;
    }
    return description;
}
