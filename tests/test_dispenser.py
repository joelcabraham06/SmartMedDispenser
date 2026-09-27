"""
Automated Pytest / Unittest Test Suite for SmartMedDispenser
"""

import unittest
import os
import sys

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

from simulation.dispenser_sim import SmartMedDispenserSimulator

class TestSmartMedDispenser(unittest.TestCase):
    def setUp(self):
        self.sim = SmartMedDispenserSimulator()

    def test_schedule_check(self):
        due_8am = self.sim.check_schedule(current_hour=8)
        self.assertEqual(len(due_8am), 2)
        self.assertEqual(due_8am[0]["name"], "Aspirin 100mg")

    def test_servo_dispensing_positioning(self):
        res = self.sim.trigger_dispense(slot_id=2)
        self.assertEqual(self.sim.current_servo_angle, 120)
        self.assertIn("status", res["event"])

    def test_inventory_deduction(self):
        initial_count = self.sim.compartments[0]["count"]
        res = self.sim.trigger_dispense(slot_id=0)
        if res["success"]:
            self.assertEqual(self.sim.compartments[0]["count"], initial_count - 1)

    def test_telemetry_summary(self):
        self.sim.trigger_dispense(slot_id=0)
        telemetry = self.sim.get_telemetry_summary()
        self.assertGreater(telemetry["total_events"], 0)
        self.assertEqual(telemetry["wifi_status"], "ONLINE")

if __name__ == "__main__":
    unittest.main()
