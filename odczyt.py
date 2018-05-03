#import tinybrd
from tinybrd import Radio

import struct
from time import localtime, strftime, sleep
import sys
import socket
import signal

#default settings for errors: write errors on pipes and sockets can be reported as ordinary Python exceptions
signal.signal(signal.SIGPIPE, signal.SIG_DFL)

def print_with_time(val):
    print("{0}{1}".format(strftime("%d %b %Y %H:%M:%S: ", localtime()),val))            

#NRF24L01 initialization: 
def setup_radio():
    radio = Radio(bytes([0,0,3]),100)
    return radio
    

def read_data():
    radio = setup_radio()
    last_seq = {}
            
    while True:
        if radio.available():
            data = radio.read()
            #if (len(data)==8):
            if (len(data)==18):
                #value = struct.unpack('=BHHBH', data) #original
                value = struct.unpack('=BLffBL', data)
                                    # 0 - id
                                    # 1 - battery
                                    # 2 - seqential no
                                    # 3 - retry no
                                    # 4 - total lost cnt
                sensor_id = value[0]
                battery = value[1]
                seq_no = value[2]
                retry = value[3]
                lost_cnt = value[4]
                node_name = "tinyBrd#{}".format(sensor_id)
                print_with_time ("ID:{}: Msg {}#{} from node: {} => {}".format(sensor_id,seq_no,retry,node_name,value) )
                if sensor_id in last_seq.keys():
                    delta_seq = seq_no-last_seq[sensor_id]
                    if delta_seq > 1 and delta_seq != 255:
                        print_with_time("ID:{}: Lost packets? Seq diff: {}".format(sensor_id,delta_seq))
                    if delta_seq == 0 and retry == 0:
                        print_with_time("ID:{}: Duplicate packet? Seq diff = 0".format(sensor_id))
                last_seq[sensor_id] = seq_no
            else:
                print_with_time ("Unknown format: {}, size: {}".format(data, len(data)))
            

while (True):
    print_with_time("Start")
    read_data()
        
