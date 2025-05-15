#!/usr/bin/python3

#  Copyright (C) 2025 The Android Open Source Project
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

import argparse
import sys
import xml.etree.ElementTree as et

CONFIG_PATH = 'carrier_config'
CONFIG_EXT = '.xml'
IGNORE_CHECK_NAMES = [
    'ignore_data_enabled_changed_for_video_calls',
]

INT_MIN = -2147483648
INT_MAX = 2147483647
LONG_MIN = -9223372036854775808
LONG_MAX = 9223372036854775807
VALIDATOR_BOOLEAN = lambda v: v in ('true', 'false', '')
VALIDATOR_INT = lambda v: INT_MIN <= int(v) <= INT_MAX if v != '' else True
VALIDATOR_LONG = lambda v: LONG_MIN <= int(v) <= LONG_MAX if v != '' else True
VALIDATOR_FLOAT = lambda v: float(v if len(v) > 0 else 0)

error_count = 0

def error(message, name=''):
  global error_count
  error_count += 1
  prefix = f'\033[95m{name}\033[0m: ' if name else ''
  print(f' - {prefix}{message}')

def check_name(name, postfix):
  if name in IGNORE_CHECK_NAMES:
    return
  if not name.endswith(postfix):
    error(f'name must end with "{postfix}"', name)

def check_value(name, element, validator, value_type):
  try:
    if not validator(element.attrib['value']):
      raise ValueError
  except ValueError:
    error(f'"value" attribute is not a valid {value_type}', name)
  except KeyError:
    error(f'"value" attribute is missing', name)

def check_array(name, element, item_check_fn):
  try:
    num = element.attrib['num']
    if int(num if num != '' else 0) != len(element):
      error(f'"num" attribute is different from the actual count', name)
  except ValueError:
    error(f'invalid "num" attribute', name)
  except KeyError:
    error(f'"num" attribute is missing', name)

  for child in element:
    if child.tag != 'item':
      error(f'array items must have "item" tags but "{child.tag}" found', name)
    if item_check_fn:
      item_check_fn(child)

def check_boolean(name, element):
  check_name(name, '_bool')
  check_value(name, element, VALIDATOR_BOOLEAN, 'boolean')

def check_int(name, element):
  check_name(name, '_int')
  check_value(name, element, VALIDATOR_INT, 'int')

def check_long(name, element):
  check_name(name, '_long')
  check_value(name, element, VALIDATOR_LONG, 'long')

def check_double(name, element):
  check_name(name, '_double')
  check_value(name, element, VALIDATOR_FLOAT, 'double')

def check_boolean_array(name, element):
  check_name(name, '_bool_array')
  check_array(name, element,
      lambda e: check_value(f'{name}/item', e, VALIDATOR_BOOLEAN, 'boolean'))

def check_int_array(name, element):
  check_name(name, '_int_array')
  check_array(name, element,
      lambda e: check_value(f'{name}/item', e, VALIDATOR_INT, 'int'))

def check_long_array(name, element):
  check_name(name, '_long_array')
  check_array(name, element,
      lambda e: check_value(f'{name}/item', e, VALIDATOR_LONG, 'long'))

def check_string_array(name, element):
  check_name(name, '_string_array')
  check_array(name, element,
      lambda e: check_value(f'{name}/item', e, lambda v: True, 'string'))

def check_double_array(name, element):
  check_name(name, '_double_array')
  check_array(name, element,
      lambda e: check_value(f'{name}/item', e, VALIDATOR_FLOAT, 'double'))

def check_bundle(element):
  names = set()

  for child in element:
    name = child.get('name')
    if not name:
      error('"name" attribute is missing or empty')
      continue

    if name in names:
      error(f'duplicated name', name)
    names.add(name)

    check_fn = {
        'boolean': check_boolean,
        'int': check_int,
        'long': check_long,
        'string': lambda n, e: check_name(n, '_string'),
        'double': check_double,
        'boolean-array': check_boolean_array,
        'int-array': check_int_array,
        'long-array': check_long_array,
        'string-array': check_string_array,
        'double-array': check_double_array,
        'pbundle_as_map': lambda n, e: check_bundle(e),
    }.get(child.tag)
    if check_fn:
      check_fn(name, child)
    else:
      error(f'"{child.tag}" is unknown tag', name)

def check(path):
  try:
    root = et.parse(path).getroot()

    if root.tag != 'carrier_config':
      error(f'The root must have "carrier_config" tag but "{root.tag}" found')

    check_bundle(root)

  except et.ParseError as e:
    error(f'Parsing error at line {e.position[0]}, col {e.position[1]}')

def get_args():
  parser = argparse.ArgumentParser()
  parser.add_argument('files', type=str, nargs='*',
      help='configuration files to check')
  return parser.parse_args()

def main():
  args = get_args()
  config_files = [
      file for file in args.files
      if CONFIG_PATH in file and file.lower().endswith(CONFIG_EXT)
  ]

  for config_file in config_files:
    print(f'Checking {config_file}')
    check(config_file)

  print(f'Checked {len(config_files)} files, ', end='')
  if error_count > 0:
    print(f'{error_count} errors found')
    return 1
  print(f'no error found')
  return 0

if __name__ == '__main__':
  sys.exit(main())
