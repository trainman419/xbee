#!/usr/bin/env python

import sys
import yaml
import subprocess
import re
from time import sleep

import phue
from phue import Bridge

def main():
    if len(sys.argv) != 2:
        print "Usage: pants.py <config.yaml>"
        sys.exit(1)

    config = yaml.load(open(sys.argv[1]))

    if not 'lights' in config:
        print "Config must contain a 'lights' array"
        sys.exit(1)

    if not 'device' in config:
        print "Config must contain a 'device'"
        sys.exit(1)

    b = None
    while b == None:
        try:
            b = Bridge("10.0.0.31")
        except phue.PhueRegistrationException as e:
            print str(e)
            sleep(5)

    lights = b.get_light_objects('name')

    xbee = subprocess.Popen(["./xbsh2", config['device']], stdout=subprocess.PIPE, 
            stderr=subprocess.STDOUT)

    line = xbee.stdout.readline()
    p = re.compile('AD3:\s+(\d+)')
    while line != None:
        m = p.search(line)
        if m:
            value = int(m.group(1))
            print value
            saturation = 254
            hue = 65000
            if value > 1000:
                # pants all the way down
                brightness = 90
            elif value > 900:
                # pants 3/4 down
                brightness = 130
            elif value > 700:
                # pants halfway down
                brightness = 170
            elif value > 400:
                # pants 1/4 down
                brightness = 210
            else:
                # pants up
                brightness = 254
                saturation = 0
            for light in config['lights']:
                l = lights[light]
                l.on = True
                l.saturation = saturation
                l.hue = hue
                l.brightness = brightness
        line = xbee.stdout.readline()

if __name__ == '__main__':
    main()
