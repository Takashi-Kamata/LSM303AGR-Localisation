clc;clear;
clf;
clear serialportObj;
global result
global counter
% Create a serialport object called serialportObj that connects to the port
% "/dev/cu.usbmodem14103" with a default baud rate of 9600.
serialportObj = serialport("/dev/cu.usbmodem14203",9600);

configureCallback(serialportObj,"terminator",@readSerialData);

serialportObj.BaudRate = 115200;

% Set the value of the "DataBits" property of the serialport object serialportObj to 7.
serialportObj.DataBits = 7;

% Set the value of the "Parity" property of the serialport object serialportObj to "odd".
serialportObj.Parity = "odd";




serialportObj.UserData = struct("X",[], "Y",[], "Z",[] ,"Count",1);

figure(1);



function readSerialData(serialportObj,~)
    data = readline(serialportObj);
    
    data = split(data,",");
    X = str2double(data);

    % Convert the string data to numeric type and save it in the UserData
    % property of the serialport object.
    serialportObj.UserData.X(end+1) = X(1);
    serialportObj.UserData.Y(end+1) = X(2);
    serialportObj.UserData.Z(end+1) = X(3);
%     size(X)
    
    % Update the Count value of the serialport object.
    serialportObj.UserData.Count = serialportObj.UserData.Count + 1;
    
    % If 1001 data points have been collected from the Arduino, switch off the
    % callbacks and plot the data.
    if serialportObj.UserData.Count > 100
        configureCallback(serialportObj, "off");
        hold on
        plot(serialportObj.UserData.X(2:end));
        plot(serialportObj.UserData.Y(2:end));
        plot(serialportObj.UserData.Z(2:end));
        legend("X", "Y", "Z");
        counter = 0;
        for i = serialportObj.UserData.X(2:end)
            if (i  -1.27)
                counter = counter + 1;
            end
        end
        disp(counter);
        disp("Finished");
    end
%     disp(X);
end



