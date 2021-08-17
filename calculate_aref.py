#!/usr/bin/env python3
import argparse
import logging


def calculate_aref(
    divider_high, divider_low, observed_total, actual_total, used_aref):
  logging.info('Determining the correct VOLTAGE_REF value.')
  observed_divided = observed_total * (
      divider_low / (divider_low + divider_high))
  logging.info(
      f'The ATTiny85 thought it saw {observed_divided:.3f}v at its analog input'
      ' pin.')
  analog_value = int(1024 * observed_divided / used_aref)
  logging.info(
      f'The ATTiny85 saw analogRead return {analog_value}. This will not'
      ' change, but we will change how the ATTin85 interprets it.')
  actual_divided = actual_total * (
      divider_low / (divider_low + divider_high))
  logging.info(
      f'The actual voltage at the analog input was {actual_divided:.3f}v.'
      ' (This could be verified with a voltmeter.)')
  actual_aref = actual_divided * (1024.0 / analog_value)
  logging.info(
      f'The ATTiny85 voltage reference is actually {actual_aref:.3f}v.')

  check_divided = actual_aref * (analog_value / 1024.0)
  check_total = check_divided * (
      (divider_low + divider_high) / divider_low)
  logging.info(
      'Using the new voltage reference value and the original analogRead value,'
      f' we calculate a total of {check_total:.3f}v.')

  return actual_aref


if __name__ == '__main__':
  logging.basicConfig(
    format='%(levelname)s %(asctime)s %(filename)s:%(lineno)s: %(message)s',
    level=logging.INFO)

  parser = argparse.ArgumentParser()
  parser.add_argument(
      'divider_high', type=float,
      help='Resistor value on the high side of the voltage divider (kOhms).')
  parser.add_argument(
      'divider_low', type=float,
      help='Resistor value on the low side of the voltage divider (kOhms).')
  parser.add_argument(
      'actual_total', type=float,
      help='Actual battery voltage, as measured by a voltmeter.')
  parser.add_argument(
      'observed_total', type=float,
      help='Reported battery voltage from the ATTin85.')
  parser.add_argument(
      'used_aref', type=float,
      help='Analog reference value in use by the ATTin85.')
  args = parser.parse_args()

  corrected_aref = calculate_aref(
      args.divider_high,
      args.divider_low,
      args.observed_total,
      args.actual_total,
      args.used_aref)
  logging.info(f'Corrected Aref: {corrected_aref:.3f}.')
