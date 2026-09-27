"""
Smart Medicine Reminder & Dispenser - Hardware & IoT Telemetry Simulator
Author: Joel C. Abraham
"""

import time
import random

class SmartMedDispenserSimulator:
    def __init__(self):
        self.compartments = [
            {"slot": 0, "name": "Aspirin 100mg", "scheduled_hour": 8, "count": 14, "status": "PENDING"},
            {"slot": 1, "name": "Vitamin C 500mg", "scheduled_hour": 13, "count": 30, "status": "PENDING"},
            {"slot": 2, "name": "Metformin 500mg", "scheduled_hour": 20, "count": 28, "status": "PENDING"},
            {"slot": 3, "name": "Calcium 250mg", "scheduled_hour": 8, "count": 15, "status": "PENDING"},
            {"slot": 4, "name": "BP Medication", "scheduled_hour": 20, "count": 20, "status": "PENDING"},
            {"slot": 5, "name": "Multi-Vitamin", "scheduled_hour": 13, "count": 25, "status": "PENDING"}
        ]
        self.current_servo_angle = 0
        self.wifi_connected = True
        self.blynk_synced = True
        self.dispensed_log = []

    def check_schedule(self, current_hour=8):
        """
        Checks whether any medicine dose is scheduled for the given hour.
        """
        due_doses = [c for c in self.compartments if c["scheduled_hour"] == current_hour and c["status"] == "PENDING"]
        return due_doses

    def trigger_dispense(self, slot_id):
        """
        Simulates ESP32 Servo positioning, IR sensor detection, and dose release.
        """
        comp = self.compartments[slot_id]
        target_angle = slot_id * 60

        print(f"[+] Moving Carousel Servo from {self.current_servo_angle} deg -> {target_angle} deg...")
        self.current_servo_angle = target_angle
        time.sleep(0.1)

        # Simulate IR Beam Obstruction (98% success rate, 2% pill jam simulation)
        ir_sensor_triggered = random.random() < 0.98

        if ir_sensor_triggered:
            comp["count"] -= 1
            comp["status"] = "DISPENSED"
            event = {
                "timestamp": time.strftime("%Y-%m-%d %H:%M:%S"),
                "slot": slot_id,
                "medicine": comp["name"],
                "status": "DISPENSED_SUCCESS",
                "remaining": comp["count"],
                "ir_verified": True
            }
            self.dispensed_log.append(event)
            return {"success": True, "event": event}
        else:
            comp["status"] = "JAM_WARNING"
            event = {
                "timestamp": time.strftime("%Y-%m-%d %H:%M:%S"),
                "slot": slot_id,
                "medicine": comp["name"],
                "status": "MECHANICAL_JAM_ALERT",
                "remaining": comp["count"],
                "ir_verified": False
            }
            self.dispensed_log.append(event)
            return {"success": False, "event": event}

    def get_telemetry_summary(self):
        total_dispensed = sum(1 for e in self.dispensed_log if e["status"] == "DISPENSED_SUCCESS")
        total_failed = sum(1 for e in self.dispensed_log if e["status"] != "DISPENSED_SUCCESS")
        
        return {
            "total_events": len(self.dispensed_log),
            "successful_dispenses": total_dispensed,
            "failed_dispenses": total_failed,
            "dispense_accuracy_pct": round((total_dispensed / max(1, len(self.dispensed_log))) * 100, 2),
            "wifi_status": "ONLINE" if self.wifi_connected else "OFFLINE",
            "blynk_status": "SYNCED" if self.blynk_synced else "DISCONNECTED"
        }

if __name__ == "__main__":
    sim = SmartMedDispenserSimulator()
    print("==========================================================")
    print("💊 ESP32 Smart Medicine Reminder & Dispenser Simulator")
    print("==========================================================\n")

    due = sim.check_schedule(current_hour=8)
    print(f"[*] Schedule Check (8:00 AM): Found {len(due)} due dose(s):")
    for d in due:
        print(f"  - Slot {d['slot']}: {d['name']}")
        res = sim.trigger_dispense(d['slot'])
        status_str = "[OK] DISPENSED & IR VERIFIED" if res["success"] else "[!] JAM ALERT"
        print(f"    Result: {status_str}\n")

    telemetry = sim.get_telemetry_summary()
    print("==========================================================")
    print("📊 IOT TELEMETRY & DISPENSER ACCURACY REPORT")
    print("==========================================================")
    print(f"  • Total Dose Events:    {telemetry['total_events']}")
    print(f"  • Dispensing Accuracy:  {telemetry['dispense_accuracy_pct']}%")
    print(f"  • WiFi Cloud Connection:{telemetry['wifi_status']}")
    print(f"  • Blynk IoT Sync:       {telemetry['blynk_status']}")
    print("==========================================================\n")
