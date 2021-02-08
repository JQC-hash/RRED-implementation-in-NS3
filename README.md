# Robust Random Early Detection(RRED) implementation in Network Simulator 3(NS3)

<p align="center">
  <img src="https://github.com/JQC-hash/RRED-implementation-in-NS3/blob/main/RRED.png">
</p>

With the network growing and evolving faster than ever in the ear of information, active queue management mechanisms are becoming an integral part of network congestion management. Active queue management mechanism Random Early Detection(RED) and its variants have been widely used handle network congestion and to improve TCP performance. However, these were not designed to resist emerging network attacks. With low-rate denial of service (LDoS) attacks becoming a concerning threat, as the attacks act rather similarly to legitimate traffic, the RED and its variant algorithms have been shown vulnerable to LDoS attacks[1].

To address the original RED¡¦s vulnerability to LDoS attacks, in 2010 Robust RED (RRED) was proposed to counter the attacks by simply detecting whether an incoming packet arrives within a short range after the latest packet dropped by the RED [2].

The original NS3 (version 3.30.1) program does not include RRED as a model in the traffic-control-module. This project implements the RRED on the basis of the existing NS3 red-queue-disc model, the following modifications are made:

1. Modification on queue-disc.cc and queue-disc.h files

The queue-disc files modified for two reasons. First, the original statistics class only keeps tracking on packets and bytes that are being processed. The new lines of codes are added for the statistics class to counting packets according to the transportation layer protocols, so performance of the RRED can be further analyzed. Second, a new function "RREDFilterBeforeEnqueue" is added to process the packets filtered by the RRED filter.

2.	Modification on red-queue-disc.cc and red-queue-disc.h files

The implementation of RRED is mainly in the function "DoEnqueue". Before the original DoEnqueue syntax, an if condition block added to perform the RRED packet detection and filtering task. New member variables are added in the red-queue-disc class, including a 2-dimensional array for the bloom table, a time variable for the user-determined short range interval, t2 for the arrival time of the last packet dropped by the RED block and more.

3.	Modification on the wscript file of traffic-control module

module = bld.create_ns3_module('traffic-control', ['point-to-point', 'core', 'network'])

4.	A new red-vs-rred.cc file in /scratch is created

red-vs-rred.cc is a script that allow users to run a default network setting with either RED or RRED mode. Network and RED/RRED parameters can be changed using command line arguments.
The default network topology adopts the experimental design of [1], which contains 30 legitimate users and 20 LDoS attackers.

**To sum up, five NS3 files are modified:**

src/traffic-control/model/red-queue-disc.cc

src/traffic-control/model/red-queue-disc.h

src/traffic-control/model/queue-disc.cc

src/traffic-control/model/queue-disc.h

examples/traffic-control/wscript

**One file is newly created:**

examples/traffic-conttrol/red-vs-rred.cc

**Development Environment:**   
Network Simulator 3.30.1 (NS3)  
Ubuntu 18.04LTS operating system   
VMware workstation 15 Player  
Windows 10 desktop  

References:

[1] M. Guirguis, A. Bestavros, and I. Matta, ¡§Exploiting the transients of adaptation for RoQ attacks on Internet resources,¡¨ IEEE, 8 10 2004. 

[2] C.Zhang, J.Yin, Z. Cai, and W. Chen, "RRED: Robust RED Algorithm to Counter Low-Rate Denial-of-Service Attacks," IEEE Communications Letters, p. Vol.14, May 2010.  
