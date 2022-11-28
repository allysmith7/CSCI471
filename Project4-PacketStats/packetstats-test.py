#!/usr/bin/python3
import argparse
import sys
import subprocess
from os import path
import re


DEBUG = False

if (not path.isfile("./packetstats")):
    print ("The file packetstats does not exisit in this directory")
    sys.exit(-1)

argumentParser = argparse.ArgumentParser(usage='<pcap file to read>')
argumentParser.add_argument('pcapFilename' )
args = argumentParser.parse_args()


if (not path.isfile(args.pcapFilename)):
    print ("The fie",args.pcapFilename," does not seem to exist")
    sys.exit(-1)


#
# Run the programs and capture the results.
#
referenceResults = subprocess.run(['./packetstats-example','-m','-a','-u','-t','-f',args.pcapFilename],capture_output=True)
studentResults = subprocess.run(['./packetstats','-m','-a','-u','-t','-f',args.pcapFilename],capture_output=True)



#
# Make a basic, high-level comparison
#
if (referenceResults.stdout.decode() == studentResults.stdout.decode()):
    print ("The output of the two programs matches exactly. Checking the rest just for fun")
    if DEBUG:
       print (referenceResults.stdout)
       print (studentResults.stdout)
else:
    print ("NOTE: The two programs returned different results. Checking to see if we can figure out why")
    print ("Reference Results:")
    print (referenceResults.stdout)
    print ("Student's Results:")
    print (studentResults.stdout)


#
# Check all the counts
#
countList = [ 'Total Ethernet', 'Min Ethernet', 'Max Ethernet', 'Average Ethernet', 'Total OtherLink', 'Min OtherLink', 'Max OtherLink', 'Average OtherLink', 'Total ARP', 'Min ARP', 'Max ARP',
              'Average ARP', 'Total IPv4' , 'Min IPv4', 'Max IPv4', 'Average IPv4', 'Total IPv6', 'Min IPv6', 'Max IPv6', 'Average IPv6', 'Total OtherNetwork', 'Min OtherNetwork', 'Max OtherNetwork',
              'Average OtherNetwork', 'Total TCP', 'Min TCP', 'Max TCP', 'Average TCP', 'Total UDP', 'Min UDP', 'Max UDP', 'Average UDP', 'Total ICMP', 'Min ICMP', 'Max ICMP', 'Average ICMP',
              'Total OtherTransport', 'Min OtherTransport', 'Max OtherTransport', 'Average OtherTransport', 'Unique srcMac', 'Unique dstMac', 'Unique srcIPv4', 'Unique dstIPv4',
              'Unique srcUDP', 'Unique dstUDP', 'Unique srcTCP', 'Unique dstTCP','synCount', 'finCount', 'totalPacketCount' ]

countsMatch = True

for countName in countList:
    referenceMatch = re.search(countName + '\s+=\s+(\d+)',referenceResults.stdout.decode('utf-8'))
    studentMatch = re.search(countName + '\s+=\s+(\d+)',studentResults.stdout.decode('utf-8'))

    if (not studentMatch):
        print ("Could not find a match for the count " + countName + " in the output submitted by the student.")
        countsMatch = False
        continue

    if (referenceMatch.group(1) == studentMatch.group(1)):
        if (DEBUG): print ("The student's value for " + countName + " agress with the reference program. Both values were " + studentMatch.group(1))
    else:
        print ("The student's value for " + countName + " DOES NOT agree with the reference program. Reference program said " + referenceMatch.group(1) + " student's program said " + studentMatch.group(1))
        countsMatch = False

if (countsMatch):
    print ("The counts of all types appear to match")

    
################################################################################
# Check the list of unique values
################################################################################


uniqPrefixList = [ 'Unique Source Mac Addresses','Unique Source Mac Addresses', 'Unique Source IPv4 Addresses','Unique Destination IPv4 Addresses',
                   'Unique UDP Source Ports', 'Unique UDP Destination Ports','Unique TCP Source Ports', 'Unique TCP Destination Ports' ] 


    
################################################################################
# First, make sure everything in the reference output is in the student output
################################################################################
uniqListsMatch = True
for uniqPrefix in uniqPrefixList:
    
    referenceUniq = re.search(uniqPrefix + '\n\s+(.+\n)*\n',referenceResults.stdout.decode('utf-8'))
    referenceUniqList = re.split(r'[\n|\t]',referenceUniq.group(0))

    referenceUniqList.remove(uniqPrefix)
    listClean = False
    while (not listClean):
        try:
            referenceUnqList.remove("")
        except:
            listClean = True

    if DEBUG: print (referenceUniqList)

    for uniq in referenceUniqList:
        if (re.search(uniq,referenceUniq.group(0))):
           if DEBUG: print ('Found ' + uniq + ' in student\'s output')
        else:
            print ('NOTE Did not find ' + uniq + ' in student\'s output')
            uniqListsMatch = False

################################################################################
# Lastly, confirm there is nothing in the student's output that is not in the reference
################################################################################
for uniqPrefix in uniqPrefixList:
    
    studentUniq = re.search(uniqPrefix + '\n\s+(.+\n)*\n',studentResults.stdout.decode('utf-8'))
    studentUniqList = re.split(r'[\n|\t]',studentUniq.group(0))

    studentUniqList.remove(uniqPrefix)
    listClean = False
    while (not listClean):
        try:
            studentUnqList.remove("")
        except:
            listClean = True

    if DEBUG: print (studentUniqList)

    for uniq in studentUniqList:
        if (re.search(uniq,studentUniq.group(0))):
            if (DEBUG): print ('Found ' + uniq + ' in student\'s output')
        else:
            print ('NOTE Studen\'s output contains ' + uniq + ' which is not in the reference output')
            uniqListsMatch = False


if (uniqListsMatch):
    print ("All the list of unique addresses or port #s seem to match. Good Job!")
else:
    print ("Found a problem with the list of unique addresses or ports")
    
