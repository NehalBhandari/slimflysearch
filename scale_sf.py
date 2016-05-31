#!/usr/bin/env python3

###########################################################
# Program:		  scale_sf.py
# Purpose:      Determine largest Slim Fly network possible
#               given minimum bisection bandwidth and
#               a radix range.
# Author:		    Franky Romero, Ashish Chaudhari,
#               Wesson Altoyan, Nehal Bhandari       
# Date:         Spring 2016
# Notes:        Based on script written by Nic McDonald for
#               the HyperX topology
# Known Limits:	Currenty only goes up to radix 51 (not
#               the script's limitation, but the Slim Fly
#               implementation on SuperSim).
###########################################################

import argparse
import subprocess

def getInfo(exe, maxradix, minterminals, minbandwidth):
  cmd = ('{0} --maxradix {1} --minterminals {2} --minbandwidth {3} '
         '--maxresults 1').format(
           exe, maxradix, minterminals, minbandwidth)
  stdout = subprocess.check_output(cmd, shell=True).decode('utf-8')
  lines = stdout.split('\n')
  if len(lines) < 3:
    return None
  else:
    best = lines[1].split()
    routers = float(best[5])
    channels = float(best[7])
    return routers, channels

def main(args):
  if args.verbose:
    print(args)
  
  if args.maxradix > 51:
    maximumRadix = 51
  else
    maximumRadix = args.maxradix
  
  print('radix,terms,routers,channels,terms/router,channels/term')
  for radix in range(args.minradix, maximumRadix+1, 1):
    if args.verbose:
      print('working on radix {0}'.format(radix))

    terms = None
    routers = None
    channels = None

    bot = 2
    maxdimensions = 2
    top = radix**(maxdimensions+1)

    # verify top is unachievable
    info = getInfo(args.slimflysearch, radix, top, args.minbandwidth)
    assert info is None, 'The programmer is an idiot!'

    # use a binary search to find the largest network possible
    while True:
      assert bot <= top
      mid = ((top - bot) // 2) + bot
      info = getInfo(args.slimflysearch, radix, mid, args.minbandwidth)

      if args.verbose:
        print('bot={0} top={1} mid={2} solution={3}'.format(
          bot, top, mid, False if info is None else True))

      if bot == mid:
        assert info is not None
        terms = mid
        routers, channels = info
        break

      if info is None:
        top = mid - 1
      else:
        bot = mid

    print('{0},{1},{2},{3},{4},{5}'.format(radix, terms, routers, channels,
                                           terms / routers, channels / terms))

if __name__ == '__main__':
  ap = argparse.ArgumentParser()
  ap.add_argument('slimflysearch',
                  help='slimflysearch executable')
  ap.add_argument('minradix', type=int,
                  help='minimum radix to search')
  ap.add_argument('maxradix', type=int,
                  help='maximum radix to search (limited to 51)')
  ap.add_argument('minbandwidth', type=float,
                  help='minimum bisection bandwidth')
  ap.add_argument('-v', '--verbose', default=False, action='store_true',
                  help='turn on verbose output')
  args = ap.parse_args()

  assert args.minradix <= args.maxradix

  main(args)
