
clear serialportObj
% Create a serialport object called serialportObj that connects to the port
% "/dev/cu.usbmodem14103" with a default baud rate of 9600.
serialportObj = serialport("/dev/cu.usbmodem14103",9600);

% Set the value of the "BaudRate" property of the serialport object serialportObj to
% 115200.
serialportObj.BaudRate = 115200;

% Set the value of the "DataBits" property of the serialport object serialportObj to 7.
serialportObj.DataBits = 7;

% Set the value of the "Parity" property of the serialport object serialportObj to "odd".
serialportObj.Parity = "odd";

% Read string data up to and including the first occurrence of the read terminator "LF"
% using the serialport object serialportObj. Data is returned without the read
% terminator.
data1 = readline(serialportObj);




clear serialportObj



