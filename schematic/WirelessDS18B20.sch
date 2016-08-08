EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:ESP8266
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L AP1117D33 U1
U 1 1 57A0A520
P 1800 1000
F 0 "U1" H 1900 750 50  0000 C CNN
F 1 "LM1117 3.3" H 1800 1250 50  0000 C CNN
F 2 "" H 1800 1000 50  0000 C CNN
F 3 "" H 1800 1000 50  0000 C CNN
	1    1800 1000
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 57A0A73A
P 1800 1400
F 0 "#PWR?" H 1800 1150 50  0001 C CNN
F 1 "GND" H 1800 1250 50  0000 C CNN
F 2 "" H 1800 1400 50  0000 C CNN
F 3 "" H 1800 1400 50  0000 C CNN
	1    1800 1400
	1    0    0    -1  
$EndComp
$Comp
L VCC #PWR?
U 1 1 57A0A78C
P 2350 850
F 0 "#PWR?" H 2350 700 50  0001 C CNN
F 1 "VCC" H 2350 1000 50  0000 C CNN
F 2 "" H 2350 850 50  0000 C CNN
F 3 "" H 2350 850 50  0000 C CNN
	1    2350 850 
	1    0    0    -1  
$EndComp
$Comp
L CONN_01X02 P1
U 1 1 57A0A7B5
P 900 1050
F 0 "P1" H 900 1200 50  0000 C CNN
F 1 "Power Connector" V 1000 1050 50  0000 C CNN
F 2 "" H 900 1050 50  0000 C CNN
F 3 "" H 900 1050 50  0000 C CNN
	1    900  1050
	-1   0    0    1   
$EndComp
$Comp
L GND #PWR?
U 1 1 57A0A801
P 1250 1400
F 0 "#PWR?" H 1250 1150 50  0001 C CNN
F 1 "GND" H 1250 1250 50  0000 C CNN
F 2 "" H 1250 1400 50  0000 C CNN
F 3 "" H 1250 1400 50  0000 C CNN
	1    1250 1400
	1    0    0    -1  
$EndComp
$Comp
L ESP-01v090 U2
U 1 1 57A0AA8A
P 4200 2450
F 0 "U2" H 4200 2350 50  0000 C CNN
F 1 "ESP-01" H 4200 2550 50  0000 C CNN
F 2 "" H 4200 2450 50  0001 C CNN
F 3 "" H 4200 2450 50  0001 C CNN
	1    4200 2450
	-1   0    0    1   
$EndComp
$Comp
L CP C1
U 1 1 57A0AC9F
P 2350 1150
F 0 "C1" H 2375 1250 50  0000 L CNN
F 1 "10uF" H 2375 1050 50  0000 L CNN
F 2 "" H 2388 1000 50  0000 C CNN
F 3 "" H 2350 1150 50  0000 C CNN
	1    2350 1150
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 57A0AD22
P 2350 1400
F 0 "#PWR?" H 2350 1150 50  0001 C CNN
F 1 "GND" H 2350 1250 50  0000 C CNN
F 2 "" H 2350 1400 50  0000 C CNN
F 3 "" H 2350 1400 50  0000 C CNN
	1    2350 1400
	1    0    0    -1  
$EndComp
$Comp
L CONN_01X03 P2
U 1 1 57A0B00F
P 950 2300
F 0 "P2" H 950 2500 50  0000 C CNN
F 1 "1-Wire Bus" V 1050 2300 50  0000 C CNN
F 2 "" H 950 2300 50  0000 C CNN
F 3 "" H 950 2300 50  0000 C CNN
	1    950  2300
	-1   0    0    1   
$EndComp
$Comp
L GND #PWR?
U 1 1 57A0B13D
P 1200 2450
F 0 "#PWR?" H 1200 2200 50  0001 C CNN
F 1 "GND" H 1200 2300 50  0000 C CNN
F 2 "" H 1200 2450 50  0000 C CNN
F 3 "" H 1200 2450 50  0000 C CNN
	1    1200 2450
	1    0    0    -1  
$EndComp
$Comp
L VCC #PWR?
U 1 1 57A0B157
P 1200 2150
F 0 "#PWR?" H 1200 2000 50  0001 C CNN
F 1 "VCC" H 1200 2300 50  0000 C CNN
F 2 "" H 1200 2150 50  0000 C CNN
F 3 "" H 1200 2150 50  0000 C CNN
	1    1200 2150
	1    0    0    -1  
$EndComp
$Comp
L R R1
U 1 1 57A0B1BF
P 1500 2100
F 0 "R1" V 1580 2100 50  0000 C CNN
F 1 "1K" V 1500 2100 50  0000 C CNN
F 2 "" V 1430 2100 50  0000 C CNN
F 3 "" H 1500 2100 50  0000 C CNN
	1    1500 2100
	1    0    0    -1  
$EndComp
Wire Wire Line
	1100 1100 1250 1100
Wire Wire Line
	1250 1100 1250 1400
Wire Wire Line
	1800 1300 1800 1400
Wire Wire Line
	2100 1000 2350 1000
Wire Wire Line
	2350 1000 2350 850 
Wire Wire Line
	1100 1000 1500 1000
Wire Wire Line
	2350 1300 2350 1400
Connection ~ 2350 1000
Wire Wire Line
	1150 2200 1200 2200
Wire Wire Line
	1200 2200 1200 2150
Wire Wire Line
	1150 2400 1200 2400
Wire Wire Line
	1200 2400 1200 2450
Wire Wire Line
	1500 1950 1500 1900
Wire Wire Line
	1150 2300 3250 2300
Wire Wire Line
	1500 2300 1500 2250
$Comp
L VCC #PWR?
U 1 1 57A0B1A8
P 1500 1900
F 0 "#PWR?" H 1500 1750 50  0001 C CNN
F 1 "VCC" H 1500 2050 50  0000 C CNN
F 2 "" H 1500 1900 50  0000 C CNN
F 3 "" H 1500 1900 50  0000 C CNN
	1    1500 1900
	1    0    0    -1  
$EndComp
$Comp
L VCC #PWR?
U 1 1 57A0B4F9
P 5200 2250
F 0 "#PWR?" H 5200 2100 50  0001 C CNN
F 1 "VCC" H 5200 2400 50  0000 C CNN
F 2 "" H 5200 2250 50  0000 C CNN
F 3 "" H 5200 2250 50  0000 C CNN
	1    5200 2250
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 57A0B516
P 3200 2650
F 0 "#PWR?" H 3200 2400 50  0001 C CNN
F 1 "GND" H 3200 2500 50  0000 C CNN
F 2 "" H 3200 2650 50  0000 C CNN
F 3 "" H 3200 2650 50  0000 C CNN
	1    3200 2650
	1    0    0    -1  
$EndComp
Wire Wire Line
	3200 2650 3200 2600
Wire Wire Line
	3200 2600 3250 2600
Wire Wire Line
	5200 2250 5200 2300
Wire Wire Line
	5200 2300 5150 2300
Wire Wire Line
	5150 2500 5250 2500
$Comp
L SW_PUSH SW1
U 1 1 57A0B62B
P 2750 3050
F 0 "SW1" H 2900 3160 50  0000 C CNN
F 1 "RESCUE_MODE" H 2750 2970 50  0000 C CNN
F 2 "" H 2750 3050 50  0000 C CNN
F 3 "" H 2750 3050 50  0000 C CNN
	1    2750 3050
	1    0    0    -1  
$EndComp
Wire Wire Line
	3250 2500 3050 2500
$Comp
L GND #PWR?
U 1 1 57A0B889
P 2400 3100
F 0 "#PWR?" H 2400 2850 50  0001 C CNN
F 1 "GND" H 2400 2950 50  0000 C CNN
F 2 "" H 2400 3100 50  0000 C CNN
F 3 "" H 2400 3100 50  0000 C CNN
	1    2400 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	2400 3100 2400 3050
Wire Wire Line
	2400 3050 2450 3050
$Comp
L BS170 Q1
U 1 1 57A0B8C6
P 2300 2550
F 0 "Q1" H 2500 2625 50  0000 L CNN
F 1 "BS170" H 2500 2550 50  0000 L CNN
F 2 "TO-92" H 2500 2475 50  0001 L CIN
F 3 "" H 2300 2550 50  0000 L CNN
	1    2300 2550
	-1   0    0    -1  
$EndComp
Connection ~ 1500 2300
$Comp
L GND #PWR?
U 1 1 57A0BC3F
P 2200 2800
F 0 "#PWR?" H 2200 2550 50  0001 C CNN
F 1 "GND" H 2200 2650 50  0000 C CNN
F 2 "" H 2200 2800 50  0000 C CNN
F 3 "" H 2200 2800 50  0000 C CNN
	1    2200 2800
	1    0    0    -1  
$EndComp
Wire Wire Line
	2200 2750 2200 2800
Wire Wire Line
	2200 2300 2200 2350
Connection ~ 2200 2300
$Comp
L R R2
U 1 1 57A0BCAA
P 2700 2600
F 0 "R2" V 2780 2600 50  0000 C CNN
F 1 "2.2K" V 2700 2600 50  0000 C CNN
F 2 "" V 2630 2600 50  0000 C CNN
F 3 "" H 2700 2600 50  0000 C CNN
	1    2700 2600
	0    1    1    0   
$EndComp
Wire Wire Line
	2500 2600 2550 2600
Wire Wire Line
	3250 2400 2900 2400
Wire Wire Line
	2900 2400 2900 2600
Wire Wire Line
	2900 2600 2850 2600
NoConn ~ 5150 2600
$Comp
L SW_PUSH SW2
U 1 1 57A8DA97
P 5800 2400
F 0 "SW2" H 5950 2510 50  0000 C CNN
F 1 "RESET" H 5800 2320 50  0000 C CNN
F 2 "" H 5800 2400 50  0000 C CNN
F 3 "" H 5800 2400 50  0000 C CNN
	1    5800 2400
	1    0    0    -1  
$EndComp
$Comp
L VCC #PWR?
U 1 1 57A8DB2D
P 5450 2650
F 0 "#PWR?" H 5450 2500 50  0001 C CNN
F 1 "VCC" H 5450 2800 50  0000 C CNN
F 2 "" H 5450 2650 50  0000 C CNN
F 3 "" H 5450 2650 50  0000 C CNN
	1    5450 2650
	1    0    0    -1  
$EndComp
Wire Wire Line
	5250 2500 5250 2700
Wire Wire Line
	5250 2700 5450 2700
Wire Wire Line
	5450 2700 5450 2650
Wire Wire Line
	5150 2400 5500 2400
$Comp
L GND #PWR?
U 1 1 57A8DBD6
P 6150 2450
F 0 "#PWR?" H 6150 2200 50  0001 C CNN
F 1 "GND" H 6150 2300 50  0000 C CNN
F 2 "" H 6150 2450 50  0000 C CNN
F 3 "" H 6150 2450 50  0000 C CNN
	1    6150 2450
	1    0    0    -1  
$EndComp
Wire Wire Line
	6100 2400 6150 2400
Wire Wire Line
	6150 2400 6150 2450
Wire Wire Line
	3050 2500 3050 3050
$EndSCHEMATC
