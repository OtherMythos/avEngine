"""Typemask parsing and signature rendering."""

import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from typemask import formatSignature, buildParams, maskProblems


class TypemaskTests(unittest.TestCase):

    def test_optional_trailing_from_negative_count(self):
        #Negative count means "at least N", the rest are optional.
        self.assertEqual(
            formatSignature("setPosition", -2, ".n|unn"),
            "setPosition(number|userdata, [number], [number])")

    def test_fixed_count(self):
        self.assertEqual(
            formatSignature("getRay", 3, ".nn"),
            "getRay(number, number)")

    def test_single_userdata(self):
        self.assertEqual(
            formatSignature("setOrientation", 2, ".u"),
            "setOrientation(userdata)")

    def test_alternatives_group(self):
        self.assertEqual(
            formatSignature("set", 4, ".uii|f|b"),
            "set(userdata, int, int|float|bool)")

    def test_no_check_is_unknown_arity(self):
        self.assertEqual(formatSignature("getPosition", None, None),
                         "getPosition(...)")

    def test_space_separates_slots(self):
        #Squirrel tolerates spaces grouping a parameter's alternatives.
        self.assertEqual(
            formatSignature("add", -3, ".u u|o u|o"),
            "add(userdata, userdata|null, [userdata|null])")

    def test_padding_when_mask_shorter_than_count(self):
        params, _ = buildParams(4, ".u")
        #Count says three real params; the mask only describes one.
        self.assertEqual(len(params), 3)
        self.assertEqual(params[1]["types"], ["any"])

    def test_invalid_char_is_tolerated_and_reported(self):
        self.assertEqual(maskProblems(".di"), ["d"])
        #The bad character renders rather than crashing.
        self.assertIn("unknown", formatSignature("f", 3, ".di"))

    def test_clean_mask_has_no_problems(self):
        self.assertEqual(maskProblems(".n|unn"), [])


if __name__ == "__main__":
    unittest.main()
