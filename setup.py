# -*- coding: utf-8 -*-
#  _    _       _  _____       _
# | |  | |     (_)/ ____|     | |
# | |  | |_ __  _| (___   __ _| |_
# | |  | | '_ \| |\___ \ / _` | __|
# | |__| | | | | |____) | (_| | |_
#  \____/|_| |_|_|_____/ \__,_|\__|
#
# Author: Yaakov Azat
# 2020 June, Done with ❤ | a@azat.ai
import os
import sys
from setuptools import setup, find_packages

with open('README.rst') as f:
    readme = f.read()

with open('LICENSE') as f:
    a_license = f.read()

here = os.path.abspath(os.path.dirname(__file__))

with open(path.join(here, 'README.md'), encoding='utf-8') as f:
    long_description = f.read()

# 'setup.py publish' shortcut.
if sys.argv[-1] == 'publish':
    os.system('python setup.py sdist')
    os.system('twine upload dist/*')
    os.system('rm -rf dist')
    os.system('rm -rf *.egg-info')
    sys.exit()

setup(
    name='usk',
    version='0.1.9',
    description="UniSat Software Kit",
    long_description=long_description,
    long_description_content_type='text/markdown',
    author='Yaakov Azat',
    author_email='a@azat.ai',
    url='https://github.com/unisatkz/USK',
    license=a_license,
    python_requires='>=3.6',
    install_requires=[
        'setuptools',
        'pigpio',
        'bme680',
        'smbus',
        'adafruit-circuitpython-bno055',
        'SI1145'
    ],
    packages=find_packages(exclude=('tests', 'docs')),
    classifiers=[
            # Trove classifiers
            # Full list: https://pypi.python.org/pypi?%3Aaction=list_classifiers
            'License :: OSI Approved :: MIT License',
            'Programming Language :: Python',
            'Programming Language :: Python :: 3.6',
            'Programming Language :: Python :: Implementation :: CPython'
        ],
)